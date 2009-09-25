// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include "Common.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/** Ununsed, should throw up a red flag if something goes wrong and it appears */
#define MESSAGE_TYPE_NULL 0

/** An action sent to an actor */
#define MESSAGE_TYPE_ACTION 1

/** A data message sent to an active actor who is waiting for data */
#define MESSAGE_TYPE_DATA 2

/** A kernel message letting it know the scheduler as added an actor */
#define MESSAGE_TYPE_ADD_ACTOR 10

/** A kernel message letting it know the scheduler has removed an actor */
#define MESSAGE_TYPE_REMOVE_ACTOR 11

/** A kernel message letting it know the scheduler has removed a group of actors */
#define MESSAGE_TYPE_REMOVE_ACTORS 12

/**
 * A kernel message letting it know the scheduler should handle this deleted actor
 * @todo Handle deleted actors in the kernel
 */
#define MESSAGE_TYPE_DELETE_ACTOR 13

/**
 * A kernel message letting it know the scheduler should handle these deleted actors
 * @todo Handle deleted actors in the kernel
 */
#define MESSAGE_TYPE_DELETE_ACTORS 14

/** A message from the kernel telling a scheduler to send some of its active actors to another scheduler */
#define MESSAGE_TYPE_REBALANCE_ACTORS 20

/** A message to the scheduler to receive the actor in the message and add it to the local list */
#define MESSAGE_TYPE_RECV_ACTOR 21

/** A request from the scheduler to create an isolated actor the kernel knows about */
#define MESSAGE_TYPE_CREATE_ISOLATED_ACTOR 30

/** A message from the scheduler to the kernel with the number of active actors.  In the future may say more */
#define MESSAGE_TYPE_LOAD_STATUS 40

/** Should throw up a red flag if something goes wrong and it appears */
#define MESSAGE_TYPE_RECYCLED 0xFFFFFFFF

/**
 * Message type, used in sending messages between actors
 */
typedef struct message__ {
    Type_Union__ args[8]; /**< The arguments of the messaage */
    void *recipient; /**< The recipient of the message */
    void *sched; /**< The scheduler the message came from */
    Action_Data_Union__ act_data; /**< The action/data in the message */
    unsigned int message_type; /**< The type of the message */
    struct message__ *prev; /**< The previous message in the dl-list */
    struct message__ *next; /**< The next message in the dl-list */
} Message__;


#ifdef __cplusplus
}
#endif

#endif /* MESSAGE_HPP_ */
