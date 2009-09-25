// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef MAIL_CHANNEL_HPP_
#define MAIL_CHANNEL_HPP_

#include "Common.hpp"
#include "Message.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \todo Don't know which must be volatile, so I'm going on the safe side here until we can circle back to it
 * The message channel between schedulers.  Uses locking to be thread-safe.
 */
typedef struct  {
    volatile Message__ *base; /**< The base of the linked list, used to detect new incoming messages */
    volatile Message__ *write_head; /**< The point where new outgoing messages will be attached */
    Message__ *buffer_base; /**< The write queue for buffered messages */
    Message__ *buffer_write_head; /**< The write head for buffered messages */
    volatile void *mutex; /**< The protection mutex used to atomically lock the channel when in use */
} Message_Channel__;

/**
 * \addtogroup Functions
 */
/*@{*/
void send_messages__(Message_Channel__ *channel, Message__ *message);
void queue_messages__(Message_Channel__ *channel, Message__ *message);
void flush_message_queue__(Message_Channel__ *channel);
Message__ *recv_messages__(Message_Channel__ *channel);
Message_Channel__ *create_message_channel__();
void delete_message_channel__(Message_Channel__ *channel);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* MAIL_CHANNEL_HPP_ */
