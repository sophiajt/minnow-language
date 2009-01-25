// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

#include "Common.hpp"
#include "Message.hpp"
#include "Actor.hpp"

#ifdef __cplusplus

#include <vector>
#include <map>
#include <set>

#endif

/**
 * @todo These aren't really used anymore, remove and refactor
 */
#define SCHEDULER_KERNEL__ 1
#define SCHEDULER_MAILMAN__ 2
#define SCHEDULER_NORMAL__ 3

/**
 * The scheduler, used one per OS thread, to manage running the microthread-based actors.
 */
typedef struct {
    unsigned int scheduler_id; /**< The numeric id of the scheduler */
    unsigned int actor_queue_rev_id; /**< The actor list rev id, used by message caches inside individual actors */
    Typeless_Vector__ *local_actors; /**< All actors local to the scheduler */
    Message__ *maint; /**< The maint message that will run maintenance functions on the scheduler */
    Message__ *msg_cache; /**< The cache of recycled messages for the scheduler */
    unsigned int msg_cache_size; /**< The current size of the cache of recycled messages */
    Typeless_Vector__ *mem_cache_blocks; /**< Cache of recycled memory blocks */
    unsigned int prev_active_actor_count; /**< Previous actor count.  When this changes, we have to notify the kernel */
    Message_Channel__ *incoming_channel; /**< Channel of incoming messages */
    Message_Channel__ *outgoing_channel; /**< Channel of outgoing messages */
    Message_Channel__ *actor_updates; /**< Channel of updates on what happens to the actors,
        all schedulers share this, so the changes reach the kernel atomically */
    BOOL is_running; /**< The "is running" bool, which lets the scheduler know if it's still active */
} Scheduler__;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup Functions
 */
/*@{*/
Message__ *get_msg_from_cache__(void *sched);
void recycle_msg__(void *sched, Message__ *msg);
void *get_memblock_from_cache__(void *sched, unsigned int size); //todo: should this be long long int?
void recycle_memblock__(void *sched, void *memblock, unsigned int size);
Scheduler__ *create_scheduler__(unsigned int scheduler_type);
void add_actor_to_sched__(Scheduler__ *s, Actor__ *new_actor);
void mail_to_actor__(Message__ *msg, Actor__ *sender);
void *scheduler_loop__(void *sched);
/*@}*/

#ifdef __cplusplus
}
#endif


#endif /* SCHEDULER_HPP_ */
