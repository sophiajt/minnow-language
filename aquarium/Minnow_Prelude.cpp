// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include "Minnow_Prelude.hpp"

#include <stdio.h>
#include <stdlib.h>

void print_i__(int i) {
    printf("%i", i);
}

void print_s__(Typeless_Vector__ *s) {
    push_onto_char_string__(s, 0);
    printf("%s", (char *)(s->contents));
    pop_off_char_string__(s);
}

void print_d__(double d) {
    printf("%f", d);
}

void print_c__(char c) {
    printf("%c", c);
}

void print_f__(float f) {
    printf("%f", f);
}

void exit_i__(int i) {
    exit(i);
}

void exit__() {
    exit(0);
}

int convert_s_to_i__(Typeless_Vector__ *s) {
    int retval;
    push_onto_char_string__(s, 0);
    retval = atoi((char *)(s->contents));
    pop_off_char_string__(s);

    return retval;
}
