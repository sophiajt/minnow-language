// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef TYPELESS_VECTOR_H_
#define TYPELESS_VECTOR_H_

#include "Common.hpp"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type-Erasing Vector.  We use this structure to store items in a typeless way,
 * hoping that the programmer knows the type to pull it out later.
 *
 * @todo Need to shrink a vector also if the number of elements shrinks significantly
 */
typedef struct {
    void* contents; /**< The contents of the vector */
    unsigned int elem_size; /**< The size of each element in the vector */
    unsigned int current_size; /**< The number of elements in the vector */
    unsigned int buffer_size; /**< The number of elements buffered, used for growing the vector */
} Typeless_Vector__;

#define STORAGE_VAR__(var) ((void*)&(var))
#define INDEX_AT__(array,index,type) (*((type*)((char *)(array->contents) + (index) * array->elem_size)))
#define DEFAULT_TYPELESS_VECTOR_SIZE__ 4

/**
 * \addtogroup Functions
 */
/*@{*/
Typeless_Vector__ *create_typeless_vector__(unsigned int elem_size, unsigned int num_elems);
void initialize_typeless_vector__(Typeless_Vector__ *container, unsigned int elem_size, unsigned int num_elems);
void delete_typeless_vector__(Typeless_Vector__ *container);
void push_onto_typeless_vector__(Typeless_Vector__ *container, void *value);
Typeless_Vector__ *concatenate_new_typeless_vector__(Typeless_Vector__ *string1, Typeless_Vector__ *string2);
Type_Union__ pop_off_typeless_vector__(Typeless_Vector__ *container);
void insert_into_typeless_vector__(Typeless_Vector__ *container, void* value, unsigned int pos);
Type_Union__ delete_from_typeless_vector__(Typeless_Vector__ *container, unsigned int pos);

//todo: figure out how to do the next one safely with codegen memory management:
void delete_from_typeless_vector_range__(Typeless_Vector__ *container, unsigned int pos,
        unsigned int amount);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* TYPELESS_VECTOR_H_ */
