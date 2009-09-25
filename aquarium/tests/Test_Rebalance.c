// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <stdlib.h>
#include "../Aquarium.hpp"

#define NUM_ITERS 1000000
#define NUM_ACTORS 1000

typedef struct {
    Actor__ base__;
} Work_Actor__;

typedef struct {
    Actor__ base__;
    unsigned int total;
} Collector_Actor__;

BOOL Collector_Actor_collect__(Message__ *m) {
    Collector_Actor__ *actor__ = (Collector_Actor__*)m->recipient;

    if (++actor__->total == NUM_ACTORS) {
        fflush(stdout);
        exit(0);
    }

    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}

BOOL Work_Actor_do_work__(Message__ *m) {
    Work_Actor__ *actor__ = (Work_Actor__*)m->recipient;
    unsigned int cont_id__ = 0;
    unsigned int pos;
    unsigned int timeslice__ = actor__->base__.timeslice_remaining;

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
            while (pos < NUM_ITERS) {
                ++pos;
                if (--timeslice__ == 0) {
                    //printf("[%i]", pos); fflush(stdout);
                    cont_id__ = 1;
                    actor__->base__.timeslice_remaining = timeslice__;
                    push_onto_typeless_vector__(actor__->base__.continuation_stack, &pos);
                    push_onto_typeless_vector__(actor__->base__.continuation_stack, &cont_id__);
                    actor__->base__.actor_state = ACTOR_STATE_ACTIVE__;
                    return TRUE;
                }
            }
            //printf("D"); fflush(stdout);
            break;
    }

    Message__ *msg = get_msg_from_cache__(m->sched);
    msg->act_data.task = Collector_Actor_collect__;
    msg->recipient = (Actor__*)m->args[0].VoidPtr;
    msg->message_type = MESSAGE_TYPE_ACTION;
    //printf("Mailing main_action\n");
    mail_to_actor__(msg, msg->recipient);

    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}

BOOL main_action__(Message__ *m) {
    Actor__ *actor__ = (Actor__*)m->recipient;

    unsigned int cont_id__ = 0;
    unsigned int pos;
    unsigned int timeslice__ = actor__->timeslice_remaining;
    Collector_Actor__ *collector;

    if (actor__->continuation_stack->current_size > 0) {
        cont_id__ = INDEX_AT__(actor__->continuation_stack, actor__->continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->continuation_stack);
        pos = INDEX_AT__(actor__->continuation_stack, actor__->continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->continuation_stack);
        collector = INDEX_AT__(actor__->continuation_stack, actor__->continuation_stack->current_size - 1, Collector_Actor__*);
        pop_off_typeless_vector__(actor__->continuation_stack);
    }

    switch (cont_id__) {
        case (0) : {
            pos = 0;
            collector = (Collector_Actor__*)create_actor__(sizeof(Collector_Actor__));
            collector->total = 0;
            add_actor_to_sched__((Scheduler__*)m->sched, (Actor__*)collector);
        }
        case (1) :
            while (pos < NUM_ACTORS) {
                ++pos;
                Work_Actor__ *recipient = (Work_Actor__*)create_actor__(sizeof(Work_Actor__));
                add_actor_to_sched__((Scheduler__*)m->sched, (Actor__*)recipient);

                Message__ *msg = get_msg_from_cache__(m->sched);
                msg->act_data.task = Work_Actor_do_work__;
                msg->recipient = recipient;
                msg->message_type = MESSAGE_TYPE_ACTION;
                msg->args[0].VoidPtr = collector;
                //printf("Mailing main_action\n");
                mail_to_actor__(msg, msg->recipient);

                if (--timeslice__ == 0) {
                    //printf("[%i]", pos); fflush(stdout);
                    actor__->timeslice_remaining = timeslice__;
                    cont_id__ = 1;
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
