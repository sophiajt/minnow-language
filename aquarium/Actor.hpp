// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef ACTOR_HPP_
#define ACTOR_HPP_

#include <stdlib.h>

#include "Common.hpp"
#include "Message.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/** The size of the receiver cache in each actor */
#define ACTOR_MSG_CACHE_SIZE__ 3

/** Denotes an active actor, one in the middle of an action */
#define ACTOR_STATE_ACTIVE__ 1

/** Denotes an inactive actor, waiting for an action */
#define ACTOR_STATE_WAITING_FOR_ACTION__ 2

/** Denotes an active actor, one in the middle of an action, but waiting for data */
#define ACTOR_STATE_WAITING_FOR_DATA__ 3

/** Denotes a deleted actor */
#define ACTOR_STATE_DELETED__ 999

/**
 * Base actor structure, all actors will have this as their first member
 */
typedef struct actor__ {
    /** A cache of recipients of recent messages, cuts down on lookup time if you keep sending to the same ones */
    struct actor__ *receiver_cache[ACTOR_MSG_CACHE_SIZE__];

    /** The actor and the scheduler have to stay in sync, so if the scheduler adds/removes actors, they won't match */
    unsigned int cache_id;

    /** The position in the cache to put the next actor you message */
    unsigned int next_cache_pos;

    /** The current action/task the actor is working on.  Used when receiving data messages. */
    BOOL (*task)(Message__*);

    /** Used when an actor's timeslice finishes and it must stop what it's doing.  This acts as the stack to freeze/resume. */
    Typeless_Vector__ *continuation_stack;

    /** The message you last froze at, so you can continue from it next go around */
    Message__ *resume_message;

    /** The exception that was thrown, if one was thrown */
    void *exception;

    /** The state the actor is in at the moment, whether active or waiting, or being deleted */
    unsigned int actor_state;

    /** The remainer of the current timeslice */
    unsigned int timeslice_remaining;
} Actor__;

/**
 * \defgroup Functions Functions
 */
/*@{*/
Actor__ *create_actor__(size_t total_size);
void initialize_actor__(Actor__ *actor);
/*@}*/

#ifdef __cplusplus
}
#endif


#endif /* ACTOR_HPP_ */
