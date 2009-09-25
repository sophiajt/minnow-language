// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

/*
 * Note: this is in C rather than C++ because it tests outwardly-facing library functionality (which is C-based)
 */

#include <stdio.h>
#include "../Typeless_Vector.hpp"

void print_contents(Typeless_Vector__ *tv) {
    unsigned int i;

    for (i = 0; i < tv->current_size; ++i) {
        int tmp = INDEX_AT__(tv, i, int);
        printf("%i: %i\n", i, tmp);
    }
    printf("====\n");
}
int main() {
    Typeless_Vector__ *tv;
    int tmp;

    tv = create_typeless_vector__(sizeof(int), 0);
    tmp = 1;
    push_onto_typeless_vector__(tv, STORAGE_VAR__(tmp));
    tmp = 2;
    push_onto_typeless_vector__(tv, STORAGE_VAR__(tmp));
    tmp = 3;
    push_onto_typeless_vector__(tv, STORAGE_VAR__(tmp));
    tmp = 4;
    push_onto_typeless_vector__(tv, STORAGE_VAR__(tmp));
    print_contents(tv);

    tmp = 0;
    insert_into_typeless_vector__(tv, STORAGE_VAR__(tmp), 0);
    print_contents(tv);

    delete_from_typeless_vector_range__(tv, 1, 2);
    print_contents(tv);

    delete_from_typeless_vector_range__(tv, 999, 999);
    print_contents(tv);

    tmp = 5;
    insert_into_typeless_vector__(tv, STORAGE_VAR__(tmp), 999);
    print_contents(tv);

    pop_off_typeless_vector__(tv);
    print_contents(tv);

    delete_from_typeless_vector__(tv, 0);
    print_contents(tv);

    delete_typeless_vector__(tv);

    return 0;
}
