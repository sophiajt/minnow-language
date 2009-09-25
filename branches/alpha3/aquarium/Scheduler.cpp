// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <string.h>
#include "Aquarium.hpp"
#include "Threading.hpp"

const unsigned int TIMESLICE_QUANTUM__= 2000;
const unsigned int MSG_CACHE_MAX__ = 1000;

const unsigned int MAX_MEMBLOCK_SIZE__ = 500;
const unsigned int MAX_MEMBLOCK_DEPTH__ = 1000;

/**
 * Gets a message from the cache of recycled messages, or creates one if there aren't enough.
 * @param sched The scheduler to use
 */
Message__ *get_msg_from_cache__(void *sched) {
    Scheduler__ *s = (Scheduler__ *)sched;
    Message__ *return_val;

    if (s->msg_cache == NULL) {
        return_val = (Message__ *)malloc(sizeof(Message__));
        if (return_val == NULL) {
            printf("Out of memory, exiting...\n");
            exit(-1);
        }
    }
    else {
        return_val = s->msg_cache;
        s->msg_cache = s->msg_cache->next;
        --s->msg_cache_size;
    }
    return_val->sched = s;

    return return_val;
}

/**
 * Recycles a message into the message cache, up to a maximum cache message number
 * @param sched The scheduler to use
 * @param msg The message to recycle
 */
void recycle_msg__(void *sched, Message__ *msg) {
    Scheduler__ *s = (Scheduler__ *)sched;

    if (s->msg_cache_size < MSG_CACHE_MAX__) {
        msg->recipient = NULL;
        msg->sched = NULL;
        msg->message_type = MESSAGE_TYPE_RECYCLED;
        msg->prev = NULL;
        msg->next = s->msg_cache;
        s->msg_cache = msg;
        ++s->msg_cache_size;
    }
    else {
        free(msg);
    }
}

void *get_memblock_from_cache__(void *sched, unsigned int size) {
    Scheduler__ *s = (Scheduler__ *)sched;

    if (size > MAX_MEMBLOCK_SIZE__) {
        return malloc(size);
    }
    else {
        Typeless_Vector__ *cache_block = INDEX_AT__(s->mem_cache_blocks, size, Typeless_Vector__*);

        if (cache_block == NULL) {
             cache_block = create_typeless_vector__(sizeof(void*), 0);
             INDEX_AT__(s->mem_cache_blocks, size, Typeless_Vector__*) = cache_block;
        }

        if (cache_block->current_size == 0) {
            return malloc(size);
        }
        else {
            void *v = pop_off_typeless_vector__(cache_block).VoidPtr;
            return v;
        }
    }
}

void recycle_memblock__(void *sched, void *memblock, unsigned int size) {
    Scheduler__ *s = (Scheduler__ *)sched;

    if (size > MAX_MEMBLOCK_SIZE__) {
        free(memblock);
    }
    else {
        Typeless_Vector__ *cache_block = INDEX_AT__(s->mem_cache_blocks, size, Typeless_Vector__*);

        if (cache_block == NULL) {
             cache_block = create_typeless_vector__(sizeof(void*), 0);
             INDEX_AT__(s->mem_cache_blocks, size, Typeless_Vector__*) = cache_block;
        }

        if (cache_block->current_size >= MAX_MEMBLOCK_DEPTH__) {
            free(memblock);
        }
        else {
            push_onto_typeless_vector__(cache_block, &memblock);
        }
    }
}

/**
 * Increments the local actor list rev id, which helps actors know if their cache is out of date.
 * @param s The scheduler to use
 */
void increment_sched_rev__(Scheduler__ *s) {
    ++s->actor_queue_rev_id;
    if (s->actor_queue_rev_id == 0) {
        //looped around, reset actors
        for (int i = 0, end = s->local_actors->current_size; i != end; ++i) {
            Actor__ *actor = INDEX_AT__(s->local_actors, i, Actor__*);
            for (int j = 0; j < ACTOR_MSG_CACHE_SIZE__; ++j) {
                actor->receiver_cache[j] = NULL;
                actor->cache_id = 0;
                actor->next_cache_pos = 0;
            }
        }
        s->actor_queue_rev_id = 1;
    }
}

/**
 * Removes an actor from the scheduler based on the index of that actor in the local actor list
 * @param s The scheduler to use
 * @param index The index of the actor to remove
 */
Message__ *remove_indexed_actor_from_sched__(Scheduler__ *s, unsigned int index) {
    Actor__ *actor = INDEX_AT__(s->local_actors, index, Actor__*);
    Message__ *iter, *out_queue = NULL;
    iter = s->maint->prev;
    int out_queue_size = 0;

    while (iter != s->maint) {
        //we go backward so that the messages end up in correct order after each append step
        if (iter->recipient == actor) {
            iter->next->prev = iter->prev;
            iter->prev->next = iter->next;
            ++out_queue_size;
            if (out_queue != NULL) {
                out_queue->prev = iter;
            }
            iter->next = out_queue;
            out_queue = iter;
        }
        iter = iter->prev;
    }
    delete_from_typeless_vector__(s->local_actors, index);
    increment_sched_rev__(s);

    return out_queue;
}

/**
 * Removes an actor from the scheduler
 * @param s The scheduler to use
 * @param target The actor to remove
 */
Message__ *remove_actor_from_sched__(Scheduler__ *s, Actor__ *target) {
    unsigned int index = 0;
    bool found = false;
    int l=0, r=s->local_actors->current_size + 1, p; //pivots

    p = (r - l) / 2;
    while ((p > 0) && (!found)) {
        p = l + p;
        if (INDEX_AT__(s->local_actors, (p-1), Actor__*) == target) {
            index = p-1;
            found = true;
        }
        else if (INDEX_AT__(s->local_actors, (p-1), Actor__*) < target) {
            l = p;
            p = (r - l) / 2;
        }
        else {
            r = p;
            p = (r - l) / 2;
        }
    }

    if (found) {
        return remove_indexed_actor_from_sched__(s, index);
    }
    else {
        return NULL;
    }
}

/**
 * The maintenance message that each scheduler runs on itself.  This handles updating the kernel with the number of active
 * actors, incoming messages for the scheduler and local actors, and receive actors which will become local.
 * @param msg The message, which is given to all actions
 */
BOOL maint_loop__(Message__ *msg) {
    Actor__ *a = (Actor__*)msg->recipient;
    Scheduler__ *s = (Scheduler__*)(msg->sched);
    unsigned int i;
    unsigned int num_active_actors=0;
    Message__ *my_mail, *recycle_mail;
    bool recycle_this_msg = true;

    for (i = 0; i < s->local_actors->current_size; ++i) {
        Actor__ *tmp = INDEX_AT__(s->local_actors, i, Actor__*);

        if (tmp->actor_state == ACTOR_STATE_ACTIVE__) {
            ++num_active_actors;
        }
        tmp->timeslice_remaining = TIMESLICE_QUANTUM__;
    }

    if (num_active_actors != s->prev_active_actor_count) {
        Message__ *notify = get_msg_from_cache__(s);
        notify->next = NULL;
        notify->message_type = MESSAGE_TYPE_LOAD_STATUS;
        notify->args[0].UInt32 = num_active_actors;
        send_messages__(s->outgoing_channel, notify);

        s->prev_active_actor_count = num_active_actors;
    }

    if (s->local_actors->current_size == 0) {
        sleep_in_ms__(50);
    }


    my_mail = recv_messages__(s->incoming_channel);
    while (my_mail != NULL) {
        switch (my_mail->message_type) {
            case (MESSAGE_TYPE_REBALANCE_ACTORS) : {
                Message__ *send_msgs = NULL;
                unsigned int num_found_actors = 0;

                //Step #1: find an active actor
                i = 0;

                //unsigned int current_queue_rev_id = s->actor_queue_rev_id;

                while (i < s->local_actors->current_size) {
                    Actor__ *actor = INDEX_AT__(s->local_actors, i, Actor__*);

                    //Move if it's an active actor and hasn't been sending messages
                    //if ((actor->actor_state == ACTOR_STATE_ACTIVE__) && (actor->cache_id != current_queue_rev_id)) {
                    if (actor->actor_state == ACTOR_STATE_ACTIVE__) {
                        Message__ *out_queue = remove_indexed_actor_from_sched__(s, i);
                        Message__ *next_msg = get_msg_from_cache__(s);
                        next_msg->message_type = MESSAGE_TYPE_RECV_ACTOR;
                        next_msg->args[0].VoidPtr = actor;
                        next_msg->args[1].VoidPtr = out_queue;
                        next_msg->next = send_msgs;
                        send_msgs = next_msg;
                        ++num_found_actors;

                        if (num_found_actors >= my_mail->args[0].UInt32) {
                            break;
                        }
                    }
                    else {
                        ++i;
                    }
                }
                if (send_msgs != NULL) {
                    send_messages__((Message_Channel__*)my_mail->args[1].VoidPtr, send_msgs);
                }
            }
            break;


            case (MESSAGE_TYPE_RECV_ACTOR) : {
                Message__ *incoming_bundle;

                for (int j = 0; j < ACTOR_MSG_CACHE_SIZE__; ++j) {
                    Actor__ *actor = (Actor__*)my_mail->args[0].VoidPtr;
                    actor->receiver_cache[j] = NULL;
                    actor->cache_id = 0;
                    actor->next_cache_pos = 0;
                }

                add_actor_to_sched__(s, (Actor__*)my_mail->args[0].VoidPtr);
                incoming_bundle = (Message__*)my_mail->args[1].VoidPtr;

                if (incoming_bundle != NULL) {
                    s->maint->prev->next = incoming_bundle;
                    incoming_bundle->prev = s->maint->prev;
                    incoming_bundle->sched = s;
                    int j = 1;
                    while (incoming_bundle->next != NULL) {
                        incoming_bundle->next->prev = incoming_bundle;
                        incoming_bundle = incoming_bundle->next;
                        ++j;
                        incoming_bundle->sched = s;
                    }
                    incoming_bundle->next = s->maint;
                    s->maint->prev = incoming_bundle;
                }

            }
            break;

            case (MESSAGE_TYPE_ACTION) :
            case (MESSAGE_TYPE_DATA) : {
                Message__ *outbound = my_mail;
                outbound->sched = s;
                recycle_this_msg = false;

                my_mail = my_mail->next;
                outbound->next = NULL;
                mail_to_actor__(outbound, (Actor__*)msg->recipient);
            }
            break;

            default: {
                printf("Unknown message type: %i on %p %p (%p)\n", my_mail->message_type, s->incoming_channel, my_mail, s);
                exit(-1);
            }
        }

        if (recycle_this_msg) {
            recycle_mail = my_mail;
            my_mail = my_mail->next;
            recycle_msg__(s, recycle_mail);
        }
        else {
            recycle_this_msg = true;
        }
    }
    flush_message_queue__(s->outgoing_channel);

    a->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return TRUE;
}

/**
 * Creates the maintenance message the scheduler will run on itself each time through the message list.
 * @param s The scheduler to create the message for
 */
Message__ *create_maint_msg__(Scheduler__ *s) {
    Message__ *return_val;

    return_val = (Message__ *)malloc(sizeof(Message__));
    if (return_val == NULL) {
        printf("Out of memory, exiting...\n");
        exit(-1);
    }

    return_val->act_data.task = maint_loop__;
    return_val->message_type = MESSAGE_TYPE_ACTION;
    return_val->next = return_val;
    return_val->prev = return_val;

    Actor__ *actor = create_actor__(sizeof(Actor__));
    actor->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    actor->task = maint_loop__;
    actor->resume_message = return_val;
    actor->timeslice_remaining = TIMESLICE_QUANTUM__;
    return_val->recipient = actor;

    return_val->sched = s;

    return return_val;
}

/**
 * Creates a scheduler by allocating resources and setting defaults.  Requires the type of scheduler.
 * @todo Remove the need to define the scheduler type
 */
Scheduler__ *create_scheduler__(unsigned int scheduler_type) {
    Scheduler__ *return_val = (Scheduler__ *)malloc(sizeof(Scheduler__));
    if (return_val == NULL) {
        printf("Out of memory, exiting...\n");
        exit(-1);
    }

    return_val->actor_queue_rev_id = 1;
    return_val->maint = create_maint_msg__(return_val);
    return_val->msg_cache = NULL;
    return_val->is_running = TRUE;
    return_val->msg_cache_size = 0;
    return_val->prev_active_actor_count = 0;
    return_val->local_actors = create_typeless_vector__(sizeof(Actor__*), 0);
    return_val->incoming_channel = create_message_channel__();
    return_val->outgoing_channel = create_message_channel__();

    return_val->mem_cache_blocks = create_typeless_vector__(sizeof(Typeless_Vector__*), MAX_MEMBLOCK_SIZE__+1);
    /*
    for (unsigned int i = 0; i < MAX_MEMBLOCK_SIZE__+1; ++i) {
        Typeless_Vector__ *tv = create_typeless_vector__(sizeof(void*), 0);
        //tv->current_size = 4;
        INDEX_AT__(return_val->mem_cache_blocks, i, Typeless_Vector__*) = tv;
    }
    */
    return_val->mem_cache_blocks->current_size = MAX_MEMBLOCK_SIZE__+1;
    memset(return_val->mem_cache_blocks->contents, 0, (MAX_MEMBLOCK_SIZE__+1) * sizeof(Typeless_Vector__*));

    return return_val;
}

/**
 * Adds an actor to the given scheduler
 * @param s The scheduler to add to
 * @param new_actor The actor to use
 */
void add_actor_to_sched__(Scheduler__ *s, Actor__ *new_actor) {
    unsigned int l=0, r=s->local_actors->current_size + 1, p; //pivots

    increment_sched_rev__(s);

    new_actor->timeslice_remaining = TIMESLICE_QUANTUM__;

    p = (r - l) / 2;
    while (p > 0) {
        p = l + p;
        if (INDEX_AT__(s->local_actors, (p-1), Actor__*) == new_actor) {
            printf("*****ADDING ACTOR TWICE: %p*****\n", new_actor); fflush(stdout);
            return;
        }
        else if (INDEX_AT__(s->local_actors, (p-1), Actor__*) < new_actor) {
            l = p;
            p = (r - l) / 2;
        }
        else {
            r = p;
            p = (r - l) / 2;
        }
    }
    insert_into_typeless_vector__(s->local_actors, &new_actor, l);

    Message__ *notify = get_msg_from_cache__(s);
    notify->next = NULL;
    notify->message_type = MESSAGE_TYPE_ADD_ACTOR;
    notify->args[0].VoidPtr = new_actor;
    notify->args[1].UInt32 = s->scheduler_id;
    send_messages__(s->actor_updates, notify);
}

/**
 * Mails a message to an actor
 * @param msg The message to send
 * @param sender The actor who is doing the sending.  This is for internal caching of the receiver.
 */
void mail_to_actor__(Message__ *msg, Actor__ *sender) {
    Scheduler__ *s = (Scheduler__*)msg->sched;
    Actor__ *target = (Actor__*)msg->recipient;
    unsigned int i;

    if (s->actor_queue_rev_id == sender->cache_id) {
        for (i = 0; i < ACTOR_MSG_CACHE_SIZE__; ++i) {
            if (sender->receiver_cache[i] == target) {
                msg->next = s->maint;
                msg->prev = s->maint->prev;
                s->maint->prev->next = msg;
                s->maint->prev = msg;
                return;
            }
        }
    }
    else {
        for (i = 0; i < ACTOR_MSG_CACHE_SIZE__; ++i) {
            sender->receiver_cache[i] = NULL;
        }
        sender->cache_id = s->actor_queue_rev_id;
        sender->next_cache_pos = 0;
    }

    unsigned int l=0, r=s->local_actors->current_size + 1, p; //pivots

    p = (r - l) / 2;
    while (p > 0) {
        p = l + p;
        if (INDEX_AT__(s->local_actors, (p-1), Actor__*) == target) {
            sender->receiver_cache[sender->next_cache_pos] = target;
            ++sender->next_cache_pos;
            if (sender->next_cache_pos >= ACTOR_MSG_CACHE_SIZE__) {
                sender->next_cache_pos = 0;
            }

            msg->next = s->maint;
            msg->prev = s->maint->prev;
            s->maint->prev->next = msg;
            s->maint->prev = msg;
            return;
        }
        else if (INDEX_AT__(s->local_actors, (p-1), Actor__*) < target) {
            l = p;
            p = (r - l) / 2;
        }
        else {
            r = p;
            p = (r - l) / 2;
        }
    }

    msg->next = NULL;
    //send_messages__(s->outgoing_channel, msg);
    queue_messages__(s->outgoing_channel, msg);
}

/**
 * The main work loop of the scheduler.
 * @param sched The scheduler itself
 */
void *scheduler_loop__(void *sched) {
    Scheduler__ *s = (Scheduler__ *)sched;

    Message__ *current = s->maint;
    Message__ *new_current;
    Actor__ *current_actor;

    while (s->is_running) {
        switch (current->message_type) {
            case (MESSAGE_TYPE_ACTION) : {
                current_actor = (Actor__*)current->recipient;

                if (current_actor->timeslice_remaining > 0) {
                    if (current_actor->actor_state == ACTOR_STATE_ACTIVE__) {
                        if ((current_actor->resume_message == current) && (current->act_data.task(current) == FALSE)) {
                            //action is complete, so recycle it
                            new_current = current->next;

                            current->prev->next = current->next;
                            current->next->prev = current->prev;
                            recycle_msg__(s, current);

                            current = new_current;
                        }
                        else {
                            current = current->next;
                        }
                    }
                    else if (current_actor->actor_state == ACTOR_STATE_WAITING_FOR_ACTION__) {
                        if (current->act_data.task(current) == FALSE) {
                            current_actor->task = current->act_data.task;

                            //action is complete, so recycle it
                            new_current = current->next;

                            current->prev->next = current->next;
                            current->next->prev = current->prev;
                            recycle_msg__(s, current);

                            current = new_current;
                        }
                        else {
                            current_actor->task = current->act_data.task;

                            current_actor->resume_message = current;
                            current = current->next;
                        }
                    }
                    else {
                        current = current->next;
                    }
                }

                else {
                    current = current->next;
                }
            }
            break;

            case (MESSAGE_TYPE_DATA) : {
                current_actor = (Actor__*)current->recipient;
                switch (current_actor->actor_state) {
                    case (ACTOR_STATE_ACTIVE__) : {
                        if ((current_actor->resume_message == current) && (current_actor->task(current) == FALSE)) {

                            //action is complete, so recycle it
                            new_current = current->next;

                            current->prev->next = current->next;
                            current->next->prev = current->prev;
                            recycle_msg__(s, current);

                            current = new_current;
                        }
                        else {
                            current = current->next;
                        }
                    }
                    break;

                    case (ACTOR_STATE_WAITING_FOR_DATA__) : {
                        if (current_actor->task(current) == FALSE) {

                            //action is complete, so recycle it
                            new_current = current->next;

                            current->prev->next = current->next;
                            current->next->prev = current->prev;
                            recycle_msg__(s, current);

                            current = new_current;
                        }
                        else {
                            //current_actor->task = current->act_data.task;
                            current = current->next;
                        }
                    }
                    break;

                    default : {
                        //if the actor can't be getting ready for data, then get rid of the message so it doesn't
                        //hang around and come at a bad time
                        new_current = current->next;

                        current->prev->next = current->next;
                        current->next->prev = current->prev;
                        recycle_msg__(s, current);

                        current = new_current;
                    }
                }
            }
            break;

            default: {
                printf("Unhandled message type: %i\n", current->message_type);
                exit(-1);
            }
        }
    }

    return NULL;
}
