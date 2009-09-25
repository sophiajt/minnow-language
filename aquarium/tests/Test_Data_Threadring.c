// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <stdlib.h>
#include "../Aquarium.hpp"

typedef struct {
    Actor__ base__;
    int id;
    Actor__ *next;
} Msg_Actor;


BOOL Msg_Actor_set_id_and_actor__(Message__ *m){
    Msg_Actor *actor__ = (Msg_Actor*)m->recipient;
    Message__ *msg;

    unsigned int cont_id__ = 0;

    if (actor__->base__.continuation_stack->current_size > 0) {
        cont_id__ = INDEX_AT__(actor__->base__.continuation_stack, actor__->base__.continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->base__.continuation_stack);
    }

    switch (cont_id__) {
        case (0) : {
            Msg_Actor *next = (Msg_Actor*)m->args[1].VoidPtr;
            int id = m->args[0].Int32;
            //printf("Id: %i continued from 0\n", id);

            actor__->next = (Actor__*)next;
            actor__->id = id;
        }
        case (1) :
            //simulate an infinite loop receiving messages.  here we receive in two stages, first, we
            //set up the continuation to restart right at the pattern match, then we do the pattern
            //match itself when we receive a message
            while (1) {
                cont_id__ = 2;
                push_onto_typeless_vector__(actor__->base__.continuation_stack, &cont_id__);
                actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_DATA__;
                --actor__->base__.timeslice_remaining;
                return FALSE;
        case (2) : {
            //printf("Id: %i continued from 1\n", actor__->id);

            switch (m->act_data.data_mask) {
                case (0x00000001) : {
                    int token = m->args[0].Int32;

                    //printf("%i %i\n", actor__->id, token);

                    if (token == 0) {
                        printf("%i\n", actor__->id);
                        exit(0);
                    }
                    else {
                        //printf("Mailing next in line\n");
                        msg = get_msg_from_cache__(m->sched);
                        msg->act_data.data_mask = 0x00000001;
                        msg->recipient = actor__->next;
                        msg->sched = m->sched;
                        msg->message_type = MESSAGE_TYPE_DATA;
                        msg->args[0].Int32 = token-1;
                        //printf("Mailing next in line\n");
                        mail_to_actor__(msg, msg->recipient);
                    }
                }
                break;
                default: {
                    printf("Unhandled data mask case: %x", m->act_data.data_mask);
                    exit(-1);
                }
            }

        }
        }
        break;

        default: {
            printf("Unhandled continuation: %i", cont_id__);
            exit(-1);
        }
    }

    //printf("Actor: %p, id: %i\n", next, id);
    return FALSE;
}

BOOL main_action__(Message__ *m) {
    Actor__ *actor__ = (Actor__*)m->recipient;
    int i;
    int num_recipients = 503;
    int token;
    Message__ *msg;

    Msg_Actor *recipients[num_recipients];

    Typeless_Vector__ *tv = (Typeless_Vector__*)m->args[0].VoidPtr;

    if (tv->current_size == 0) {
        printf("Please specify number of runs for the token\n");
        exit(0);
    }

    //char *arg = (char*)(INDEX_AT__(tv, 0, union Type_Union__).VoidPtr);
    token = convert_s_to_i__(INDEX_AT__(tv, 0, Typeless_Vector__*));

    //token = atoi(arg);

    //printf("Scheduling recipients\n");
    for (i = 0; i < num_recipients; ++i) {
        recipients[i] = (Msg_Actor*)create_actor__(sizeof(Msg_Actor));
        add_actor_to_sched__((Scheduler__*)m->sched, (Actor__*)recipients[i]);
        //scheduler_schedule_new_actor(actor__->parent, (Actor*)recipients[i]);
    }
    //printf("Done scheduling recipients\n");

    for (i = 0; i < num_recipients-1; ++i) {
        msg = get_msg_from_cache__(m->sched);
        msg->act_data.task = Msg_Actor_set_id_and_actor__;
        msg->recipient = recipients[i];
        msg->sched = m->sched;
        msg->message_type = MESSAGE_TYPE_ACTION;
        msg->args[0].Int32 = i+1;
        msg->args[1].VoidPtr = recipients[i+1];
        //printf("Mailing main_action\n");
        mail_to_actor__(msg, msg->recipient);
    }
    msg = get_msg_from_cache__(m->sched);
    msg->act_data.task = Msg_Actor_set_id_and_actor__;
    msg->recipient = recipients[num_recipients-1];
    msg->sched = m->sched;
    msg->message_type = MESSAGE_TYPE_ACTION;
    msg->args[0].Int32 = num_recipients;
    msg->args[1].VoidPtr = recipients[0];
    //printf("Mailing main_action\n");
    mail_to_actor__(msg, msg->recipient);

    msg = get_msg_from_cache__(m->sched);
    msg->act_data.data_mask = 0x00000001;
    msg->recipient = recipients[0];
    msg->sched = m->sched;
    msg->message_type = MESSAGE_TYPE_DATA;
    msg->args[0].Int32 = token;
    mail_to_actor__(msg, msg->recipient);

    actor__->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}
int main(int argc, char *argv[]) {
    aquarium_main__(argc, argv, main_action__, TRUE);

    return 0;
}
