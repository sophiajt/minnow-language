// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef COMMON_HPP_
#define COMMON_HPP_

#ifdef __cplusplus

extern "C" {
#endif

/** A bool for compatibility with older C */
#define BOOL char
/** A true for compatibility with older C */
#define TRUE 1
/** A false for compatibility with older C */
#define FALSE 0

struct message__;

/**
 * The union that allows us to collapse all the datatypes into one value type
 */
typedef union {
    signed char Int8;
    unsigned char UInt8;
    signed short Int16;
    unsigned short UInt16;
    signed int Int32;
    unsigned int UInt32;
    signed long long Int64;
    unsigned long long UInt64;
    float Float;
    double Double;
    BOOL Bool;
    void(*Function)(void *);
    void *VoidPtr;
}  Type_Union__;

/**
 * A union to collapse action and data messages into one value type
 */
typedef union {
    BOOL (*task)(struct message__*); /**< The task, if it's an action */
    unsigned int data_mask; /**< The data mask, which lets you know the argument types, if data */
} Action_Data_Union__;

#ifdef __cplusplus
}
#endif

#include "Typeless_Vector.hpp"


#endif /* COMMON_HPP_ */
