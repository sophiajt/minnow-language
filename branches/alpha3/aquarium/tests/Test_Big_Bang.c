// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

/*
 * This is an adaptation of the Big Bang benchmark for Erlang by Rickard Green
 */

#include <stdio.h>
#include <stdlib.h>
#include "../Aquarium.hpp"

typedef struct Bang_ {
    Actor__ base__;
    int response_count;
    struct Bang_ *collector;
} Bang_Actor;

#define NUM_ACTORS 1500

BOOL Bang_Actor_collect__(Message__ *m) {
    Bang_Actor *actor__ = (Bang_Actor*)m->recipient;

    //printf("C(%i)", actor__->response_count);
    //fflush(stdout);

    if (++actor__->response_count == NUM_ACTORS) {
        exit(0);
    }

    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}

BOOL Bang_Actor_recv__(Message__ *m) {
    Bang_Actor *actor__ = (Bang_Actor*)m->recipient;

    //printf("V(%p, %i)", actor__, actor__->response_count);
    //fflush(stdout);

    if (++actor__->response_count == NUM_ACTORS) {
        Message__ *msg = get_msg_from_cache__(m->sched);
        msg->act_data.task = Bang_Actor_collect__;
        msg->recipient = actor__->collector;
        msg->message_type = MESSAGE_TYPE_ACTION;
        //printf("Mailing main_action\n");
        mail_to_actor__(msg, &actor__->base__);
    }

    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}

BOOL Bang_Actor_reply__(Message__ *m) {
    Bang_Actor *actor__ = (Bang_Actor*)m->recipient;

    //printf("R(%p, %i)", actor__, actor__->response_count);
    //fflush(stdout);

    Message__ *msg = get_msg_from_cache__(m->sched);
    //printf("Collector: %p", actor__->collector); fflush(stdout);
    msg->act_data.task = Bang_Actor_recv__;
    msg->recipient = m->args[0].VoidPtr;
    msg->message_type = MESSAGE_TYPE_ACTION;
    //printf("Mailing main_action\n");
    mail_to_actor__(msg, &actor__->base__);

    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}

BOOL Bang_Actor_msg_others__(Message__ *m) {
    Bang_Actor *actor__ = (Bang_Actor*)m->recipient;
    unsigned int cont_id__ = 0;
    unsigned int pos;
    unsigned int timeslice__ = actor__->base__.timeslice_remaining;
    Message__ *msg;
    Typeless_Vector__ *tv;

    if (actor__->base__.continuation_stack->current_size > 0) {
        cont_id__ = INDEX_AT__(actor__->base__.continuation_stack, actor__->base__.continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->base__.continuation_stack);
        pos = INDEX_AT__(actor__->base__.continuation_stack, actor__->base__.continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->base__.continuation_stack);
    }

    switch (cont_id__) {
        case (0) : {
            pos = 0;
        }
        case (1) :
            tv = (Typeless_Vector__*)m->args[0].VoidPtr;
            while (pos < NUM_ACTORS) {
                Bang_Actor *ba = INDEX_AT__(tv, pos, Bang_Actor*);
                ++pos;
                //printf("O<%p, %p>", ba, actor__); fflush(stdout);
                msg = get_msg_from_cache__(m->sched);
                msg->act_data.task = Bang_Actor_reply__;
                msg->recipient = ba;
                msg->message_type = MESSAGE_TYPE_ACTION;
                msg->args[0].VoidPtr = actor__;
                //printf("Mailing main_action\n");
                mail_to_actor__(msg, &actor__->base__);

                if (--timeslice__ == 0) {
                    //printf("[%i]", pos); fflush(stdout);
                    actor__->base__.timeslice_remaining = timeslice__;
                    cont_id__ = 1;
                    push_onto_typeless_vector__(actor__->base__.continuation_stack, &pos);
                    push_onto_typeless_vector__(actor__->base__.continuation_stack, &cont_id__);
                    actor__->base__.actor_state = ACTOR_STATE_ACTIVE__;
                    return TRUE;
                }
            }
            //printf("D"); fflush(stdout);
            break;
    }

    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}

BOOL main_action__(Message__ *m) {
    Actor__ *actor__ = (Actor__*)m->recipient;
    unsigned int cont_id__ = 0;
    unsigned int timeslice__ = actor__->timeslice_remaining;
    unsigned int pos;
    int i;

    Typeless_Vector__ *all_actors;

    Bang_Actor *collector;

    if (actor__->continuation_stack->current_size > 0) {
        cont_id__ = INDEX_AT__(actor__->continuation_stack, actor__->continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->continuation_stack);
        pos = INDEX_AT__(actor__->continuation_stack, actor__->continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->continuation_stack);
        collector = INDEX_AT__(actor__->continuation_stack, actor__->continuation_stack->current_size - 1, Bang_Actor*);
        pop_off_typeless_vector__(actor__->continuation_stack);
        all_actors = INDEX_AT__(actor__->continuation_stack, actor__->continuation_stack->current_size - 1, Typeless_Vector__*);
        pop_off_typeless_vector__(actor__->continuation_stack);
    }

    switch (cont_id__) {
        case (0) :
            pos = 0;
            collector = (Bang_Actor*)create_actor__(sizeof(Bang_Actor));
            collector->response_count = 0;
            add_actor_to_sched__((Scheduler__*)m->sched, (Actor__*)collector);

            all_actors = create_typeless_vector__(sizeof(Bang_Actor*),0);

            for (i = 0; i < NUM_ACTORS; ++i) {
                Bang_Actor *ba = (Bang_Actor*)create_actor__(sizeof(Bang_Actor));
                ba->collector = collector;
                ba->response_count = 0;
                add_actor_to_sched__((Scheduler__*)m->sched, &ba->base__);
                push_onto_typeless_vector__(all_actors, &ba);
            }
        case (1) :
            //printf(">>%i<<", pos); fflush(stdout);
            while (pos < NUM_ACTORS) {
                //printf("{%i}", pos); fflush(stdout);
                Bang_Actor *recipient = INDEX_AT__(all_actors, pos, Bang_Actor*);
                //add_actor_to_sched__((Scheduler__*)m->sched, (Actor__*)recipient);

                Message__ *msg = get_msg_from_cache__(m->sched);
                msg->act_data.task = Bang_Actor_msg_others__;
                msg->recipient = recipient;
                msg->message_type = MESSAGE_TYPE_ACTION;
                msg->args[0].VoidPtr = all_actors;
                //msg->args[1].VoidPtr = collector;
                //printf("Mailing main_action\n");
                mail_to_actor__(msg, actor__);

                ++pos;
                if (--timeslice__ == 0) {
                    //printf("[%i]", pos); fflush(stdout);
                    actor__->timeslice_remaining = timeslice__;
                    cont_id__ = 1;
                    push_onto_typeless_vector__(actor__->continuation_stack, &all_actors);
                    push_onto_typeless_vector__(actor__->continuation_stack, &collector);
                    push_onto_typeless_vector__(actor__->continuation_stack, &pos);
                    push_onto_typeless_vector__(actor__->continuation_stack, &cont_id__);
                    actor__->actor_state = ACTOR_STATE_ACTIVE__;
                    return TRUE;
                }
            }
    }

    actor__->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}
int main(int argc, char *argv[]) {
    aquarium_main__(argc, argv, main_action__, FALSE);

    return 0;
}
