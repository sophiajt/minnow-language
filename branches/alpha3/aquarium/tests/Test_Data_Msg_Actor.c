// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <stdlib.h>
#include "../Aquarium.hpp"

typedef struct {
    Actor__ base__;
    int id;
} Msg_Actor__;

BOOL Msg_Actor_recv_data__(Message__ *m) {
    Msg_Actor__ *actor__ = (Msg_Actor__*)m->recipient;

    unsigned int cont_id__ = 0;

    if (actor__->base__.continuation_stack->current_size > 0) {
        cont_id__ = INDEX_AT__(actor__->base__.continuation_stack, actor__->base__.continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->base__.continuation_stack);
    }

    switch (cont_id__) {
        case (0): {
            cont_id__ = 1;
            push_onto_typeless_vector__(actor__->base__.continuation_stack, &cont_id__);
            actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_DATA__;
            return FALSE;
        }
        case (1): {
            switch (m->act_data.data_mask) {
                case (0x00000001) : {
                    int val = m->args[0].Int32;

                    printf("Data received: %i", val);
                    exit(0);
                }
            }
        }
    }

    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_DATA__;
    return FALSE;
}

BOOL main_action__(Message__ *m) {
    Actor__ *actor__ = (Actor__*)m->recipient;

    Msg_Actor__ *recipient = (Msg_Actor__*)create_actor__(sizeof(Msg_Actor__));
    add_actor_to_sched__((Scheduler__*)m->sched, (Actor__*)recipient);

    Message__ *msg = get_msg_from_cache__(m->sched);
    msg->act_data.task = Msg_Actor_recv_data__;
    msg->recipient = recipient;
    msg->sched = m->sched;
    msg->message_type = MESSAGE_TYPE_ACTION;
    //printf("Mailing main_action\n");
    mail_to_actor__(msg, msg->recipient);

    msg = get_msg_from_cache__(m->sched);
    msg->act_data.data_mask = 0x00000001;
    msg->recipient = recipient;
    msg->sched = m->sched;
    msg->message_type = MESSAGE_TYPE_DATA;
    msg->args[0].Int32 = 12345;

    //printf("Mailing main_action\n");
    mail_to_actor__(msg, msg->recipient);

    actor__->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}
int main(int argc, char *argv[]) {
    aquarium_main__(argc, argv, main_action__, FALSE);

    return 0;
}
