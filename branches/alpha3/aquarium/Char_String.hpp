// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAR_STRING_HPP_
#define CHAR_STRING_HPP_

#include "Typeless_Vector.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup Functions
 */
/*@{*/
Typeless_Vector__ *create_char_string__(int num_elems);
Typeless_Vector__ *create_char_string_from_char_ptr__(char *source);
void initialize_char_string__(Typeless_Vector__ *container, unsigned int num_elems);
char char_in_char_string__(Typeless_Vector__ *container, int pos);
Typeless_Vector__ *concatenate_char_string__(Typeless_Vector__ *container, Typeless_Vector__ *source);
Typeless_Vector__ *concatenate_new_char_string__(Typeless_Vector__ *string1, Typeless_Vector__ *string2);
Typeless_Vector__ *concatenate_char_ptr__(Typeless_Vector__ *container, char *source);
void delete_char_string__(Typeless_Vector__ *container);
Typeless_Vector__ *push_onto_char_string__(Typeless_Vector__ *container, char value);
Typeless_Vector__ *push_onto_new_char_string__(Typeless_Vector__ *container, char value);
void pop_off_char_string__(Typeless_Vector__ *container);
Typeless_Vector__ *insert_into_char_string__(Typeless_Vector__ *container, char value, unsigned int pos);
Typeless_Vector__ *insert_into_new_char_string__(Typeless_Vector__ *container, char value, unsigned int pos);
void delete_from_char_string__(Typeless_Vector__ *container, unsigned int pos);
void delete_from_char_string_range__(Typeless_Vector__ *container, unsigned int pos,
        unsigned int amount);
int compare_char_string__(Typeless_Vector__ *container, Typeless_Vector__ *compare_to);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* CHAR_STRING_HPP_ */
