// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <string.h>
#include <stdio.h>

#include "Common.hpp"
#include "Typeless_Dictionary.hpp"

Typeless_Dictionary__ *create_typeless_dictionary__(unsigned int elem_size) {
    Typeless_Dictionary__ *container = (Typeless_Dictionary__*) malloc(sizeof(Typeless_Dictionary__));

    if (container == NULL) {
        printf("Internal Error: Memory full - can not continue\n");
        exit(-1);
    }

    initialize_typeless_dictionary__(container, elem_size);

    return container;
}

void initialize_typeless_dictionary__(Typeless_Dictionary__ *container, unsigned int elem_size) {
    container->elem_size = elem_size;
    memset(container->contents, 0, DEFAULT_TYPELESS_DICTIONARY_SIZE__ * sizeof(Typeless_Vector__*));
}

void delete_typeless_slot__(Typeless_Vector__ *tv) {
    //we're assuming the data has been destructed outside of this section
    //so all we're interested in is deleting the keys and the hashtable slot
    for (unsigned int i = 0; i < tv->current_size; ++i) {
        Dict_Unit__ du = INDEX_AT__(tv, i, Dict_Unit__);
        delete_char_string__(du.key);
    }
    delete_typeless_vector__(tv);
}

void delete_typeless_dictionary__(Typeless_Dictionary__ *container) {
    if (container != NULL) {
        for (unsigned int i = 0; i < DEFAULT_TYPELESS_DICTIONARY_SIZE__; ++i) {
            if (container->contents[i] != NULL) {
                delete_typeless_slot__(container->contents[i]);
            }
        }
        free(container);
    }
}

unsigned int hash_key__(Typeless_Vector__ *key) {
    //djb-style hash
    unsigned int hash = 5381;
    const char *dataset = (const char *)(key->contents);

    for (unsigned int i = 0; i < key->current_size; ++i) {
        hash = ((hash << 5) + hash) + dataset[i];
    }

    return (hash & 0x7FFFFFFF);
}

void *lookup_key_in_slot__(Typeless_Vector__ *slot, Typeless_Vector__ *key) {
    for (unsigned int i = 0; i < slot->current_size; ++i) {
        if (compare_char_string__(INDEX_AT__(slot, i, Dict_Unit__).key, key) == 0) {
            return &(INDEX_AT__(slot, i, Dict_Unit__).data);
        }
    }

    //if we get here, we didn't find the key in this slot, so we make a new 'unit' in the slot
    Dict_Unit__ du;
    du.key = create_char_string__(0);
    concatenate_char_string__(du.key, key);
    du.data.UInt64 = 0;

    push_onto_typeless_vector__(slot, &du);
    return &(INDEX_AT__(slot, slot->current_size-1, void*));
}

BOOL contains_key_in_slot__(Typeless_Vector__ *slot, Typeless_Vector__ *key) {
    for (unsigned int i = 0; i < slot->current_size; ++i) {
        if (compare_char_string__(INDEX_AT__(slot, i, Dict_Unit__).key, key) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

void *lookup_key_in_dictionary__(Typeless_Dictionary__ *container, Typeless_Vector__ *key) {
    unsigned int slot_index = hash_key__(key) % DEFAULT_TYPELESS_DICTIONARY_SIZE__;

    Typeless_Vector__ *slot = container->contents[slot_index];
    if (slot != NULL) {
        return lookup_key_in_slot__(slot, key);
    }
    else {
        container->contents[slot_index] = create_typeless_vector__(sizeof(Dict_Unit__), 0);
        return lookup_key_in_slot__(container->contents[slot_index], key);
    }
}

BOOL contains_key_in_dictionary__(Typeless_Dictionary__ *container, Typeless_Vector__ *key) {
    unsigned int slot_index = hash_key__(key) % DEFAULT_TYPELESS_DICTIONARY_SIZE__;

    Typeless_Vector__ *slot = container->contents[slot_index];
    if (slot != NULL) {
        return contains_key_in_slot__(slot, key);
    }
    else {
        return FALSE;
    }
}
