// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>

#include "Aquarium.hpp"
#include "Message_Channel.hpp"
#include "Threading.hpp"

/**
 * Sends a linked list of messages, NULL-terminated, on the message channel
 * @param channel The channel to send on
 * @param message The message to send
 */

void send_messages__(Message_Channel__ *channel, Message__ *message) {
    Scoped_Lock__ sl((Mutex__*)channel->mutex);

    channel->write_head->next = message;
    while (message->next != NULL) {
        message = message->next;
    }
    channel->write_head = message;
    //is_empty = false;
}

/*
void send_messages__(Message_Channel__ *channel, Message__ *message) {
    queue_messages__(channel, message);
    flush_message_queue__(channel);
}
*/
void queue_messages__(Message_Channel__ *channel, Message__ *message) {

    channel->buffer_write_head->next = message;
    while (message->next != NULL) {
        message = message->next;
    }
    channel->buffer_write_head = message;

    /*
    channel->write_head->next = message;
    while (message->next != NULL) {
        message = message->next;
    }
    channel->write_head = message;
    */
    //is_empty = false;
}

void flush_message_queue__(Message_Channel__ *channel) {
    if (channel->buffer_base != channel->buffer_write_head) {
        Scoped_Lock__ sl((Mutex__*)channel->mutex);

        channel->write_head->next = channel->buffer_base->next;
        channel->write_head = channel->buffer_write_head;

        channel->buffer_write_head = channel->buffer_base;
    }
}

/**
 * Receives a linked list of messages, NULL-terminated, from the channel
 * @param channel The channel to use
 */
Message__ *recv_messages__(Message_Channel__ *channel) {
    Scoped_Lock__ sl((Mutex__*)channel->mutex);
    Message__ *return_val = channel->base->next;
    channel->write_head = channel->base;
    channel->base->next = NULL;
    //is_empty = true;
    return return_val;
}

/**
 * Creates a message channel with defaults, ready for sending and receiving
 */
Message_Channel__ *create_message_channel__() {
    Message_Channel__ *return_val = (Message_Channel__*)malloc(sizeof(Message_Channel__));
    if (return_val == NULL) {
        printf("Error, memory exhausted, exiting.\n");
        exit(-1);
    }

    return_val->base = (Message__*)malloc(sizeof(Message__));
    if (return_val->base == NULL) {
        printf("Error, memory exhausted, exiting.\n");
        exit(-1);
    }
    return_val->base->next = NULL;
    return_val->base->prev = NULL;

    return_val->buffer_base = (Message__*)malloc(sizeof(Message__));
    if (return_val->buffer_base == NULL) {
        printf("Error, memory exhausted, exiting.\n");
        exit(-1);
    }
    return_val->buffer_base->next = NULL;
    return_val->buffer_base->prev = NULL;

    return_val->write_head = return_val->base;
    return_val->buffer_write_head = return_val->buffer_base;

    return_val->mutex = new Mutex__();

    return return_val;
}

/**
 * Frees resources for a message channel, including releasing the related mutex
 * @param channel The channel to delete
 * @todo Finish fleshing this out, by deleting the messages still in the queue
 */
void delete_message_channel__(Message_Channel__ *channel) {
    delete((Mutex__*)channel->mutex);

    free(channel);
}


