// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <stdlib.h>
#include "../Aquarium.hpp"

typedef struct {
    Actor__ base__;
    int id;
} Msg_Actor__;

BOOL Msg_Actor_print_and_quit__(Message__ *m) {
    Msg_Actor__ *actor__ = (Msg_Actor__*)m->recipient;

    printf("Id: %i\n", actor__->id);
    exit(0);
    //actor__->base__.actor_state = ACTOR_STATE_DELETED__;
    return FALSE;
}

BOOL Msg_Actor_set_id__(Message__ *m) {
    Msg_Actor__ *actor__ = (Msg_Actor__*)m->recipient;

    int id = m->args[0].Int32;

    printf("Setting id: %i\n", id);
    actor__->id = id;

    actor__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}

BOOL main_action__(Message__ *m) {
    Actor__ *actor__ = (Actor__*)m->recipient;

    Msg_Actor__ *recipient = (Msg_Actor__*)create_actor__(sizeof(Msg_Actor__));
    //add_actor_to_sched__((Scheduler__*)m->sched, (Actor__*)recipient);

    Message__ *msg = get_msg_from_cache__(m->sched);
    msg->message_type = MESSAGE_TYPE_CREATE_ISOLATED_ACTOR;
    msg->args[0].VoidPtr = recipient;
    msg->next = NULL;
    send_messages__(((Scheduler__*)m->sched)->outgoing_channel, msg);

    msg = get_msg_from_cache__(m->sched);
    msg->act_data.task = Msg_Actor_set_id__;
    msg->recipient = recipient;
    msg->message_type = MESSAGE_TYPE_ACTION;
    msg->args[0].Int32 = 321;
    //printf("Mailing main_action\n");
    mail_to_actor__(msg, msg->recipient);

    msg = get_msg_from_cache__(m->sched);
    msg->act_data.task = Msg_Actor_print_and_quit__;
    msg->recipient = recipient;
    msg->message_type = MESSAGE_TYPE_ACTION;
    //printf("Mailing main_action\n");
    mail_to_actor__(msg, msg->recipient);

    actor__->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
    return FALSE;
}
int main(int argc, char *argv[]) {
    aquarium_main__(argc, argv, main_action__, FALSE);

    return 0;
}
