// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

/*
 * Note: this is in C rather than C++ because it tests outwardly-facing library functionality (which is C-based)
 */

#include <stdio.h>
#include "../Typeless_Dictionary.hpp"

int main() {
    Typeless_Dictionary__ *dict = create_typeless_dictionary__(sizeof(int));

    DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Bob"), int) = 5;
    printf("Lookup: %i\n", DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Bobby"), int));
    printf("Lookup: %i\n", DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Fred"), int));
    printf("Lookup: %i\n", DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Bob"), int));

    DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Bob"), int) = 2;
    printf("Lookup: %i\n", DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Bob"), int));

    DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Bob"), int)++;
    printf("Lookup: %i\n", DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Bob"), int));

    DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Fred"), int) = 7;
    printf("Lookup: %i\n", DICT_LOOKUP_AT__(dict, create_char_string_from_char_ptr__("Fred"), int));

    return 0;
}
