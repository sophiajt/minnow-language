// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Common.hpp"
#include "Typeless_Vector.hpp"
#include "Minnow_Prelude.hpp"
/**
 * Creates a default typeless vector
 * @param elem_size The size of each element in the vector
 * @param num_elems The number of elements to pre-allocate.  0=default
 */
Typeless_Vector__ *create_typeless_vector__(unsigned int elem_size, unsigned int num_elems) {
    Typeless_Vector__ *container = (Typeless_Vector__*) malloc(sizeof(Typeless_Vector__));

    if (container == NULL) {
        printf("Internal Error: Memory full - can not continue\n");
        exit(-1);
    }

    initialize_typeless_vector__(container, elem_size, num_elems);

    return container;
}

/**
 * Initializes a typeless vector
 * @param container The container to initialize
 * @param elem_size The size of each element in the vector
 * @param num_elems The number of elements to pre-allocate.  0=default
 */
void initialize_typeless_vector__(Typeless_Vector__ *container, unsigned int elem_size, unsigned int num_elems) {
    int n_elems = num_elems;

    if (n_elems == 0)
        n_elems = DEFAULT_TYPELESS_VECTOR_SIZE__;

    if (elem_size == 0) {
        printf("Internal Error: Can not create an array of elem size 0 - can not continue\n");
        exit(-1);
    }

    container->contents = (void*) malloc(n_elems * elem_size);
    if (container->contents == NULL) {
        printf("Internal Error: Memory full - can not continue\n");
        exit(-1);
    }
    container->buffer_size = n_elems;

    container->current_size = 0;
    container->elem_size = elem_size;
}

/**
 * Deletes the contents of a typeless vector
 * @param container The vector to delete
 */
void delete_typeless_vector__(Typeless_Vector__ *container) {
    if (container != NULL) {
        if (container->contents != NULL) {
            free(container->contents);
        }
        free(container);
    }
}

/**
 * Pushes a value onto the vector
 * @param container The container to push onto
 * @param value The value to push
 */
void push_onto_typeless_vector__(Typeless_Vector__ *container, void *value) {
    if (container->current_size >= container->buffer_size) {
        container->buffer_size = container->buffer_size * 2 + 1;

        void* temp = (void*) realloc(container->contents, container->buffer_size
                * container->elem_size);

        if (temp == NULL) {
            printf("Memory limited exceeded, exiting.\n");
            exit(-1);
        }
        container->contents = temp;
    }

    switch (container->elem_size) {
        case (4):
            ((int*) (container->contents))[container->current_size]
                    = *(int *) value;
            break;
        case (2):
            ((short*) (container->contents))[container->current_size]
                    = *(short *) value;
            break;
        case (1):
            ((char*) (container->contents))[container->current_size]
                    = *(char *) value;
            break;

        default:
            memcpy((char*) container->contents + container->current_size
                    * container->elem_size, value, container->elem_size);
            break;
    }
    ++(container->current_size);
}

Typeless_Vector__ *concatenate_new_typeless_vector__(Typeless_Vector__ *tv1, Typeless_Vector__ *tv2) {
    Typeless_Vector__ *new_c = create_typeless_vector__(tv1->elem_size, 0);

    unsigned int total_1 = tv1->elem_size * tv1->current_size;
    unsigned int total_2 = tv2->elem_size * tv2->current_size;

    new_c->contents = realloc(new_c->contents, total_1 + total_2);
    if (new_c->contents == NULL) {
        printf("Memory exhausted during vector concatentation\n");
        exit(0);
    }
    if (total_1 > 0) {
        memcpy(new_c->contents, tv1->contents, total_1);
    }
    if (total_2 > 0) {
        memcpy((char *)new_c->contents + total_1, tv2->contents, total_2);
    }
    new_c->current_size = tv1->current_size + tv2->current_size;
    new_c->buffer_size = tv1->current_size + tv2->current_size;

    return new_c;
}
/**
 * Pops a value off the vector
 * @param container The container to pop off
 */
Type_Union__ pop_off_typeless_vector__(Typeless_Vector__ *container) {
    Type_Union__ ret_val = INDEX_AT__(container, container->current_size - 1, Type_Union__);
    if (container->current_size > 0)
        --(container->current_size);

    return ret_val;
}

/**
 * Inserts a value at some point in the vector
 * @param container The container to use
 * @param value The value to insert
 * @param pos The position at which to insert
 */
void insert_into_typeless_vector__(Typeless_Vector__ *container, void* value, unsigned int pos) {
    if (pos > container->current_size) {
        push_onto_typeless_vector__(container, value);
    }
    else {
        if (container->current_size >= container->buffer_size) {
            container->buffer_size *= 2;
            void* temp =
                    (void*) realloc(container->contents, container->buffer_size
                            * container->elem_size);
            container->contents = temp;
        }

        memmove((char*) container->contents + (pos + 1) * container->elem_size, (char*) container->contents
                + pos * container->elem_size, container->elem_size
                * (container->current_size - pos));

        switch (container->elem_size) {
            case (4):
                ((int*) (container->contents))[pos] = *(int *) value;
                break;
            case (2):
                ((short*) (container->contents))[pos] = *(short *) value;
                break;
            case (1):
                ((char*) (container->contents))[pos] = *(char *) value;
                break;
            default:
                memcpy((char*) container->contents + pos * container->elem_size, value, container->elem_size);
                break;
        }
        ++(container->current_size);
    }
}

/**
 * Deletes an element from the vector
 * @param container The container to use
 * @param pos The position to delete
 */
Type_Union__ delete_from_typeless_vector__(Typeless_Vector__ *container, unsigned int pos) {
    Type_Union__ ret_val;
    if (pos >= container->current_size) {
        ret_val = INDEX_AT__(container, container->current_size - 1, Type_Union__ );
        pop_off_typeless_vector__(container);
    }
    else {
        ret_val = INDEX_AT__(container, pos, Type_Union__ );
        memmove((char*) container->contents + pos * container->elem_size, (char*) container->contents
                + (pos + 1) * container->elem_size, container->elem_size
                * (container->current_size - pos));
        --(container->current_size);
    }
    return ret_val;
}

/**
 * Deletes a range of elements from the vector
 * @param container The container to use
 * @param pos The position to begin
 * @param amount The number of positions to delete
 */
void delete_from_typeless_vector_range__(Typeless_Vector__ *container, unsigned int pos,
        unsigned int amount) {
    if (pos >= container->current_size) {
        pop_off_typeless_vector__(container);
    }
    else {
        if ((pos+amount) > container->current_size) {
            amount = container->current_size - pos;
        }
        memmove((char*) container->contents + pos * container->elem_size, (char*) container->contents
                + (pos + amount) * container->elem_size, container->elem_size
                * (container->current_size - pos));
        container->current_size -= amount;
    }
}
