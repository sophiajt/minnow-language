// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <string.h>

#include "Char_String.hpp"

Typeless_Vector__ *create_char_string__(int num_elems) {
    return create_typeless_vector__(sizeof(char), num_elems);
}

Typeless_Vector__ *create_char_string_from_char_ptr__(char *source) {
    Typeless_Vector__ *ret_val = create_char_string__(0);
    concatenate_char_ptr__(ret_val, source);
    return ret_val;
}


void initialize_char_string__(Typeless_Vector__ *container, unsigned int num_elems) {
    initialize_typeless_vector__(container, sizeof(char), num_elems);
}

char char_in_char_string__(Typeless_Vector__ *container, int pos) {
    if ((pos >= 0) && (pos < (signed)container->current_size)) {
        return ((char*)container->contents)[pos];
    }
    else {
        return ((char*)container->contents)[(container->current_size + pos) % (container->current_size)];
    }
}

Typeless_Vector__ *concatenate_char_string__(Typeless_Vector__ *container, Typeless_Vector__ *source) {
    int i, end;
    for (i = 0, end = source->current_size; i < end; ++i) {
        push_onto_char_string__(container, ((char*)source->contents)[i]);
    }
    return container;
}

Typeless_Vector__ *concatenate_new_char_string__(Typeless_Vector__ *string1, Typeless_Vector__ *string2) {
    /*
    Typeless_Vector__ *new_c = create_char_string__(0);

    concatenate_char_string__(new_c, string1);
    concatenate_char_string__(new_c, string2);

    return new_c;
    */
    return concatenate_new_typeless_vector__(string1, string2);
}

Typeless_Vector__ *concatenate_char_ptr__(Typeless_Vector__ *container, char *source) {
    int i, end;
    for (i = 0, end = strlen(source); i < end; ++i) {
        push_onto_char_string__(container, source[i]);
    }
    return container;
}

void delete_char_string__(Typeless_Vector__ *container) {
    delete_typeless_vector__(container);
}

Typeless_Vector__ *push_onto_char_string__(Typeless_Vector__ *container, char value) {
    push_onto_typeless_vector__(container, &value);
    return container;
}

Typeless_Vector__ *push_onto_new_char_string__(Typeless_Vector__ *container, char value) {
    Typeless_Vector__ *new_c = create_char_string__(0);

    concatenate_char_string__(new_c, container);
    push_onto_char_string__(new_c, value);
    return new_c;
}

void pop_off_char_string__(Typeless_Vector__ *container) {
    pop_off_typeless_vector__(container);
}

Typeless_Vector__ *insert_into_char_string__(Typeless_Vector__ *container, char value, unsigned int pos) {
    insert_into_typeless_vector__(container, &value, pos);
    return container;
}

Typeless_Vector__ *insert_into_new_char_string__(Typeless_Vector__ *container, char value, unsigned int pos) {
    Typeless_Vector__ *new_c = create_char_string__(0);

    push_onto_char_string__(new_c, value);
    concatenate_char_string__(new_c, container);
    return new_c;
}

void delete_from_char_string__(Typeless_Vector__ *container, unsigned int pos) {
    delete_from_typeless_vector__(container, pos);
}

void delete_from_char_string_range__(Typeless_Vector__ *container, unsigned int pos,
        unsigned int amount) {

    delete_from_typeless_vector_range__(container, pos, amount);
}

int compare_char_string__(Typeless_Vector__ *container, Typeless_Vector__ *compare_to) {
    if (container->current_size == 0) {
        return -1;
    }
    else if (compare_to->current_size == 0) {
        return 1;
    }
    else {
        unsigned int end;
        if (container->current_size < compare_to->current_size) {
            end = container->current_size;
        }
        else {
            end = compare_to->current_size;
        }

        /*
        for (i = 0, end = container->current_size; i != end; ++i) {
            if (char_in_char_string__(container, i) < char_in_char_string__(compare_to, i)) {
                return -1;
            }
            else if (char_in_char_string__(container, i) > char_in_char_string__(compare_to, i)) {
                return 1;
            }
        }
        */
        int ret_val = memcmp(container->contents, compare_to->contents, end);
        if (ret_val == 0) {
            if (container->current_size < compare_to->current_size) {
                return -1;
            }
            else if (container->current_size > compare_to->current_size) {
                return 1;
            }
            else {
                return 0;
            }
        }
        else {
            return ret_val;
        }
    }
}
