// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <stdlib.h>
#include "../Aquarium.hpp"

void print_char_string(Typeless_Vector__ *tv) {
    int i, end;

    for (i = 0, end = tv->current_size; i != end; ++i) {
        printf("%c", char_in_char_string__(tv, i));
    }
}

int main() {
    Typeless_Vector__ *tv = create_char_string__(0);
    concatenate_char_ptr__(tv, "This test");
    concatenate_char_ptr__(tv, " passed\n");
    print_char_string(tv);

    Typeless_Vector__ *tv2 = create_char_string__(0);
    concatenate_char_string__(tv2, tv);
    print_char_string(tv2);

    printf("Should be 0: %i\n", compare_char_string__(tv, tv2));

    pop_off_char_string__(tv2);
    push_onto_char_string__(tv2, '!');
    push_onto_char_string__(tv2, '\n');

    print_char_string(tv2);

    printf("Should be neg: %i\n", compare_char_string__(tv, tv2));
    delete_from_char_string_range__(tv2, 0, tv2->current_size);
    concatenate_char_ptr__(tv2, "A passing test\n");
    print_char_string(tv2);

    printf("Should be pos: %i\n", compare_char_string__(tv, tv2));

    insert_into_char_string__(tv2, 'm', 1);
    concatenate_char_string__(tv, tv2);
    print_char_string(tv);

    return 0;
}
