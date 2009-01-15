// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <string.h>

#include <map>

#include "Aquarium.hpp"
#include "Threading.hpp"

/**
 * The kernel actor, which handles all high-level synchronization tasks, rebalancing, and long distance message delivery
 */
typedef struct {
    Actor__ base__; /**< The common base between all actors */
    std::map<Actor__*, unsigned int> actor_addresses; /**< The lookup between an actor and where it currently is */
    std::vector<Message_Channel__*> incoming_channels; /**< The incoming channels from all schedulers */
    std::vector<Message_Channel__*> outgoing_channels; /**< The outgoing channels to all schedulers */
    Message_Channel__ *actor_updates; /**< The channel of actor updates shared by all schedulers */
    std::vector<unsigned int> schedule_weights; /**< The active actor count for all schedulers */
    unsigned int num_hw_threads;
    std::vector<unsigned int> thread_pool_ids; /**< The ids of threads used for isolated actors in the thread pool */
    std::vector<bool> thread_available; /**< Whether or not the spare scheduler thread is available for an isolated actor */

    std::vector<Thread__*> threads; /**< Scheduler threads themselves */
    std::vector<Scheduler__*> schedulers;
} Kernel_Actor__;

/**
 * The main kernel loop which runs every time the kernel action fires.  Handles status updates, rebalance notifications,
 * messages across schedulers, and creating isolated actors.
 */
BOOL kernel_loop__(Message__ *m) {
    Message__ *recycle_mail;
    Kernel_Actor__ *actor__ = (Kernel_Actor__*)m->recipient;
    bool check_rebalance = false;

    Message__ *actor_updates = recv_messages__(actor__->actor_updates);
    while (actor_updates != NULL) {
        switch (actor_updates->message_type) {
            case (MESSAGE_TYPE_ADD_ACTOR) : {
                actor__->actor_addresses[(Actor__*)actor_updates->args[0].VoidPtr] = actor_updates->args[1].UInt32;
            }
            break;
        }
        recycle_mail = actor_updates;
        actor_updates = actor_updates->next;
        recycle_msg__(m->sched, recycle_mail);
    }

    for (int current_pos = 0, end_pos = actor__->incoming_channels.size(); current_pos < end_pos; ++current_pos) {
        Message__ *new_mail = recv_messages__(actor__->incoming_channels[current_pos]);
        bool recycle_this_msg = true;

        while (new_mail != NULL) {
            switch (new_mail->message_type) {
                case (MESSAGE_TYPE_LOAD_STATUS) : {
                    //printf("S[%i:%i]", current_pos, new_mail->args[0].UInt32); fflush(stdout);
                    actor__->schedule_weights[current_pos] = new_mail->args[0].UInt32;
                    check_rebalance = true;
                }
                break;

                case (MESSAGE_TYPE_CREATE_ISOLATED_ACTOR) : {
                    //printf("Creating isolated actor: ");
                    bool found = false;
                    for (int i = 0, end = actor__->thread_available.size(); i != end; ++i) {
                        if (actor__->thread_available[i]) {
                            Message__ *next_msg = get_msg_from_cache__(m->sched);
                            next_msg->message_type = MESSAGE_TYPE_RECV_ACTOR;
                            next_msg->args[0].VoidPtr = new_mail->args[0].VoidPtr; //the newly isolated actor
                            next_msg->args[1].VoidPtr = NULL; //no messages yet
                            next_msg->next = NULL;

                            actor__->thread_available[i] = false;

                            found = true;

                            //send_messages__(actor__->outgoing_channels[actor__->thread_pool_ids[i]], next_msg);
                            queue_messages__(actor__->outgoing_channels[actor__->thread_pool_ids[i]], next_msg);
                        }
                    }
                    if (!found) {
                        Scheduler__ *new_s = create_scheduler__(SCHEDULER_NORMAL__);
                        new_s->actor_updates = actor__->actor_updates;

                        actor__->incoming_channels.push_back(new_s->outgoing_channel);
                        actor__->outgoing_channels.push_back(new_s->incoming_channel);
                        actor__->schedule_weights.push_back(0xFFFFFFFF); //do not weigh isolated thread pool threads

                        new_s->scheduler_id = actor__->incoming_channels.size() - 1;

                        actor__->thread_available.push_back(false);
                        actor__->thread_pool_ids.push_back(actor__->incoming_channels.size() - 1);

                        Thread__ *t = new Thread__();
                        t->create(scheduler_loop__, new_s);

                        Message__ *next_msg = get_msg_from_cache__(m->sched);
                        next_msg->message_type = MESSAGE_TYPE_RECV_ACTOR;
                        next_msg->args[0].VoidPtr = new_mail->args[0].VoidPtr; //the newly isolated actor
                        next_msg->args[1].VoidPtr = NULL; //no messages yet
                        next_msg->next = NULL;

                        found = true;

                        //send_messages__(new_s->incoming_channel, next_msg);
                        queue_messages__(new_s->incoming_channel, next_msg);
                    }
                }
                break;

                case (MESSAGE_TYPE_DELETE_ACTOR) : {
                    std::map<Actor__*, unsigned int>::iterator finder = actor__->actor_addresses.find((Actor__*)m->recipient);
                    if (finder != actor__->actor_addresses.end()) {
                        for (int i = 0, end = actor__->thread_pool_ids.size(); i != end; ++i) {
                            if (actor__->thread_pool_ids[i] == finder->second) {
                                //we found an actor that was isolate in our thread pool, so make that thread available again
                                actor__->thread_available[i] = true;
                            }
                        }
                        actor__->actor_addresses.erase(finder);
                    }
                    else {
                        printf("Error: Deleting unknown actor\n");
                    }
                }
                break;

                case (MESSAGE_TYPE_ACTION) :
                case (MESSAGE_TYPE_DATA) : {
                    //Mail that the sender doesn't know who it should go to, so look up the receiver and send the mail
                    std::map<Actor__*, unsigned int>::iterator finder =
                        actor__->actor_addresses.find((Actor__*)new_mail->recipient);

                    if (finder != actor__->actor_addresses.end()) {
                        //printf("<R %p:%p>", new_mail, actor__->outgoing_channels[finder->second]); fflush(stdout);
                        Message__ *outbound = new_mail;
                        recycle_this_msg = false;

                        new_mail = new_mail->next;
                        outbound->next = NULL;
                        //send_messages__(actor__->outgoing_channels[finder->second], outbound);
                        queue_messages__(actor__->outgoing_channels[finder->second], outbound);
                    }
                    else {
                        Message__ *outbound = new_mail;
                        recycle_this_msg = false;

                        new_mail = new_mail->next;
                        outbound->next = NULL;
                        send_messages__(actor__->incoming_channels[0], outbound); //try again later
                        //queue_messages__(actor__->incoming_channels[0], outbound); //try again later
                    }
                }
                break;

                default : {
                    printf("Kernel: unhandled message type %i\n", new_mail->message_type);
                    exit(-1);
                }
            }
            if (recycle_this_msg) {
                recycle_mail = new_mail;
                new_mail = new_mail->next;
                recycle_msg__(m->sched, recycle_mail);
            }
            else {
                recycle_this_msg = true;
            }
        }
    }

    /*
    if (check_rebalance) {
        unsigned int heaviest_pos=0;
        unsigned int heaviest_weight=0, total=0;

        for (unsigned int i = 0, end = actor__->schedule_weights.size(); i != end; ++i) {
            // 0xffffffff is our "ignore me" flag
            if ((actor__->schedule_weights[i] != 0xffffffff) && (actor__->schedule_weights[i] > heaviest_weight)) {
                heaviest_weight = actor__->schedule_weights[i];
                heaviest_pos = i;
                total += actor__->schedule_weights[i];
            }
        }

        if (total > 1) {
            //unsigned int minimum = total / actor__->num_hw_threads;
            for (unsigned int i = 0, end = actor__->schedule_weights.size(); i != end; ++i) {
                // 0xffffffff is our "ignore me" flag
                if ((actor__->schedule_weights[i] != 0xffffffff) && ((actor__->schedule_weights[i] * 2) < heaviest_weight)) {
                    unsigned int amount = (heaviest_weight + actor__->schedule_weights[i])/2 - actor__->schedule_weights[i] - 1;
                    if (amount > 0) {
                        Message__ *msg = get_msg_from_cache__(m->sched);
                        msg->message_type = MESSAGE_TYPE_REBALANCE_ACTORS;
                        msg->args[0].Int32 = amount;
                        msg->args[1].VoidPtr = actor__->outgoing_channels[i];
                        msg->next = NULL;
                        send_messages__(actor__->outgoing_channels[heaviest_pos], msg);

                        heaviest_weight -= amount;
                        actor__->schedule_weights[heaviest_pos] = heaviest_weight;
                    }
                }
            }
        }

        //rebalance, with some smoothing thrown in
        if ((lightest_pos != heaviest_pos) && ((heaviest_weight - lightest_weight) > (1 + (heaviest_weight / 10)))) {
            unsigned int amount = (50 + heaviest_weight - lightest_weight) / 50;

            Message__ *msg = get_msg_from_cache__(m->sched);
            msg->message_type = MESSAGE_TYPE_REBALANCE_ACTORS;
            msg->args[0].Int32 = amount;
            msg->args[1].VoidPtr = actor__->outgoing_channels[lightest_pos];
            msg->next = NULL;
            send_messages__(actor__->outgoing_channels[heaviest_pos], msg);

        }
    }
    */

    if (check_rebalance) {
    //if (false) {
        unsigned int lightest_pos = 0, heaviest_pos = 0;
        unsigned int lightest_weight = 0xffffffff, heaviest_weight=0;

        for (unsigned int i = 0, end = actor__->schedule_weights.size(); i != end; ++i) {
            // 0xffffffff is our "ignore me" flag
            if (actor__->schedule_weights[i] != 0xffffffff) {
                if (actor__->schedule_weights[i] < lightest_weight) {
                    lightest_pos = i;
                    lightest_weight = actor__->schedule_weights[i];
                }
                if (actor__->schedule_weights[i] > heaviest_weight) {
                    heaviest_pos = i;
                    heaviest_weight = actor__->schedule_weights[i];
                }
            }
        }

        //rebalance, with some smoothing thrown in
        if ((lightest_pos != heaviest_pos) && ((heaviest_weight - lightest_weight) > (1 + (heaviest_weight / 10)))) {
            unsigned int amount = (50 + heaviest_weight - lightest_weight) / 50;

            if (actor__->threads[lightest_pos] == NULL) {
                actor__->threads[lightest_pos] = new Thread__();
                actor__->threads[lightest_pos]->create(scheduler_loop__, actor__->schedulers[lightest_pos]);
            }

            Message__ *msg = get_msg_from_cache__(m->sched);
            msg->message_type = MESSAGE_TYPE_REBALANCE_ACTORS;
            msg->args[0].Int32 = amount;
            msg->args[1].VoidPtr = actor__->outgoing_channels[lightest_pos];
            msg->next = NULL;
            //send_messages__(actor__->outgoing_channels[heaviest_pos], msg);
            queue_messages__(actor__->outgoing_channels[heaviest_pos], msg);

            //actor__->schedule_weights[heaviest_pos] -= amount;
        }
    }
    for (int current_pos = 0, end_pos = actor__->incoming_channels.size(); current_pos < end_pos; ++current_pos) {
        flush_message_queue__(actor__->outgoing_channels[current_pos]);
    }
    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return TRUE;
}

/**
 * The main entry point into aquarium.  Starts all schedulers and begins the main action
 */
/*
int aquarium_main__(int argc, char *argv[], BOOL(*task)(Message__ *), BOOL pass_cmd_line) {
    Thread__ t;

    //FIXME: traditionally something like num_hw_threads would be a static
    int num_sched_threads = t.num_hw_threads();
    Message_Channel__ *actor_updates = create_message_channel__();

    Scheduler__ *schedulers[num_sched_threads+1];

    for (int i = 0; i < num_sched_threads+1; ++i) {
        schedulers[i] = create_scheduler__(SCHEDULER_NORMAL__);
        schedulers[i]->actor_updates = actor_updates;
        schedulers[i]->scheduler_id = i;
    }

    //special case: the kernel is new'd here because it's one of the only things that has to be C++ managed
    Kernel_Actor__ *kernel = new Kernel_Actor__();
    initialize_actor__(&kernel->base__);
    kernel->actor_updates = actor_updates;
    kernel->num_hw_threads = num_sched_threads;

    //connect up channels
    for (int i = 0; i < num_sched_threads+1; ++i) {
        //printf("<%i>", i);
        kernel->incoming_channels.push_back(schedulers[i]->outgoing_channel);
        kernel->outgoing_channels.push_back(schedulers[i]->incoming_channel);
        kernel->schedule_weights.push_back(0);
    }

    //Schedule the main actor (like the main function in the program)
    Actor__ *main_actor = create_actor__(sizeof(Actor__));
    add_actor_to_sched__(schedulers[0+1], main_actor);
    add_actor_to_sched__(schedulers[0+1], &kernel->base__);

    //Set up the action that will fire off our kernel event
    Message__ *kernel_action = get_msg_from_cache__(schedulers[0]);
    kernel_action->act_data.task = kernel_loop__;
    kernel_action->recipient = kernel;
    kernel_action->sched = schedulers[0];
    kernel_action->message_type = MESSAGE_TYPE_ACTION;
    mail_to_actor__(kernel_action, &kernel->base__);

    if (pass_cmd_line) {
        //package up the commandline for the first actor
        Typeless_Vector__ *commandline_args = create_typeless_vector__(sizeof(Typeless_Vector__*),0);

        //Grab commandline args
        for (int i = 1; i < argc; ++i) {
            Typeless_Vector__ *cmd_arg = create_char_string_from_char_ptr__(argv[i]);

            push_onto_typeless_vector__(commandline_args, &cmd_arg);
        }

        //And send it the first message
        Message__ *main_action = get_msg_from_cache__(schedulers[1]);
        main_action->act_data.task = task;
        main_action->recipient = main_actor;
        main_action->sched = schedulers[1];
        main_action->args[0].VoidPtr = commandline_args;
        main_action->message_type = MESSAGE_TYPE_ACTION;

        mail_to_actor__(main_action, main_actor);
    }
    else {
        Message__ *main_action = get_msg_from_cache__(schedulers[1]);
        main_action->act_data.task = task;
        main_action->recipient = main_actor;
        main_action->sched = schedulers[1];
        main_action->message_type = MESSAGE_TYPE_ACTION;

        mail_to_actor__(main_action, main_actor);
    }

    Thread__ *threads[num_sched_threads+1];
    for (int i = 1; i < num_sched_threads+1; ++i) {
        threads[i] = new Thread__();
        threads[i]->create(scheduler_loop__, schedulers[i]);
    }

    //printf("Scheduler loop\n");
    scheduler_loop__(schedulers[0]);

    //join the threads here.

    //for (int i = 1; i < num_sched_threads; ++i) {
    //    threads[i]->join();
    //}

    return 0;
}
*/
int aquarium_main__(int argc, char *argv[], BOOL(*task)(Message__ *), BOOL pass_cmd_line) {
    Thread__ t;

    //FIXME: traditionally something like num_hw_threads would be a static
    int num_sched_threads = num_hw_threads__();
    Message_Channel__ *actor_updates = create_message_channel__();

    /*
    Scheduler__ *schedulers[num_sched_threads];

    for (int i = 0; i < num_sched_threads; ++i) {
        schedulers[i] = create_scheduler__(SCHEDULER_NORMAL__);
        schedulers[i]->actor_updates = actor_updates;
        schedulers[i]->scheduler_id = i;
    }
    */

    //special case: the kernel is new'd here because it's one of the only things that has to be C++ managed
    Kernel_Actor__ *kernel = new Kernel_Actor__();
    initialize_actor__(&kernel->base__);
    kernel->actor_updates = actor_updates;
    kernel->num_hw_threads = num_sched_threads;

    for (int i = 0; i < num_sched_threads; ++i) {
        kernel->schedulers.push_back(create_scheduler__(SCHEDULER_NORMAL__));
        kernel->schedulers[i]->actor_updates = actor_updates;
        kernel->schedulers[i]->scheduler_id = i;
    }

    //connect up channels
    for (int i = 0; i < num_sched_threads; ++i) {
        //printf("<%i>", i);
        kernel->incoming_channels.push_back(kernel->schedulers[i]->outgoing_channel);
        kernel->outgoing_channels.push_back(kernel->schedulers[i]->incoming_channel);
        kernel->schedule_weights.push_back(0);
    }

    //Schedule the main actor (like the main function in the program)
    Actor__ *main_actor = create_actor__(sizeof(Actor__));
    add_actor_to_sched__(kernel->schedulers[0], main_actor);
    add_actor_to_sched__(kernel->schedulers[0], &kernel->base__);

    //Set up the action that will fire off our kernel event
    Message__ *kernel_action = get_msg_from_cache__(kernel->schedulers[0]);
    kernel_action->act_data.task = kernel_loop__;
    kernel_action->recipient = kernel;
    kernel_action->sched = kernel->schedulers[0];
    kernel_action->message_type = MESSAGE_TYPE_ACTION;
    mail_to_actor__(kernel_action, &kernel->base__);

    if (pass_cmd_line) {
        //package up the commandline for the first actor
        Typeless_Vector__ *commandline_args = create_typeless_vector__(sizeof(Typeless_Vector__*),0);

        //Grab commandline args
        for (int i = 1; i < argc; ++i) {
            Typeless_Vector__ *cmd_arg = create_char_string_from_char_ptr__(argv[i]);

            push_onto_typeless_vector__(commandline_args, &cmd_arg);
        }

        //And send it the first message
        Message__ *main_action = get_msg_from_cache__(kernel->schedulers[0]);
        main_action->act_data.task = task;
        main_action->recipient = main_actor;
        main_action->sched = kernel->schedulers[0];
        main_action->args[0].VoidPtr = commandline_args;
        main_action->message_type = MESSAGE_TYPE_ACTION;

        mail_to_actor__(main_action, main_actor);
    }
    else {
        Message__ *main_action = get_msg_from_cache__(kernel->schedulers[0]);
        main_action->act_data.task = task;
        main_action->recipient = main_actor;
        main_action->sched = kernel->schedulers[0];
        main_action->message_type = MESSAGE_TYPE_ACTION;

        mail_to_actor__(main_action, main_actor);
    }

    /*
    Thread__ *threads[num_sched_threads];
    for (int i = 1; i < num_sched_threads; ++i) {
        threads[i] = new Thread__();
        threads[i]->create(scheduler_loop__, schedulers[i]);
    }
    */
    kernel->threads.push_back(new Thread__());
    for (int i = 1; i < num_sched_threads; ++i) {
        kernel->threads.push_back(NULL);
    }

    //printf("Scheduler loop\n");
    scheduler_loop__(kernel->schedulers[0]);

    //join the threads here.

    //for (int i = 1; i < num_sched_threads; ++i) {
    //    threads[i]->join();
    //}

    return 0;
}
