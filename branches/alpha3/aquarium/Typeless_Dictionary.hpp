// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef DICTIONARY_HPP_
#define DICTIONARY_HPP_

#include "Aquarium.hpp"
#include "Char_String.hpp"
#include "Common.hpp"
#include "Typeless_Vector.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_TYPELESS_DICTIONARY_SIZE__ 1000
#define DICT_LOOKUP_AT__(dict,key,type) (*((type*)lookup_key_in_dictionary__(dict, key)))

typedef struct {
    Type_Union__ data;
    Typeless_Vector__ *key;
} Dict_Unit__;

typedef struct {
    unsigned int elem_size;
    Typeless_Vector__ *contents[DEFAULT_TYPELESS_DICTIONARY_SIZE__];
} Typeless_Dictionary__;

Typeless_Dictionary__ *create_typeless_dictionary__(unsigned int elem_size);
void initialize_typeless_dictionary__(Typeless_Dictionary__ *container, unsigned int elem_size);
void delete_typeless_dictionary__(Typeless_Dictionary__ *container);
void *lookup_key_in_dictionary__(Typeless_Dictionary__ *container, Typeless_Vector__ *key);
BOOL contains_key_in_dictionary__(Typeless_Dictionary__ *container, Typeless_Vector__ *key);

#ifdef __cplusplus
}
#endif

#endif /* DICTIONARY_HPP_ */
