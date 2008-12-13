// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include "Minnow_Prelude.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_i__(int i) {
    printf("%i", i);
}

void print_s__(Typeless_Vector__ *s) {
    if (s == NULL) return;
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
    if (s == NULL) return 0;
    int retval;
    push_onto_char_string__(s, 0);
    retval = atoi((char *)(s->contents));
    pop_off_char_string__(s);

    return retval;
}

float convert_s_to_f__(Typeless_Vector__ *s) {
    if (s == NULL) return 0;
    float retval;
    push_onto_char_string__(s, 0);
    retval = atof((char *)(s->contents));
    pop_off_char_string__(s);

    return retval;
}

double convert_s_to_d__(Typeless_Vector__ *s) {
    if (s == NULL) return 0;
    double retval;
    push_onto_char_string__(s, 0);
    retval = atof((char *)(s->contents));
    pop_off_char_string__(s);

    return retval;
}

char convert_s_to_c__(Typeless_Vector__ *s) {
    if (s == NULL) return 0;

    if (s->current_size == 0)
        return 0;
    else
        return ((char *)s->contents)[0];
}

Typeless_Vector__ *convert_i_to_s__(int i) {
    Typeless_Vector__ *tv = create_char_string__(10);
    snprintf((char*)tv->contents, 9, "%i", i);
    tv->current_size = strlen((char*)tv->contents);
    pop_off_char_string__(tv); //remove the trailing 0
    return tv;
}

float convert_i_to_f__(int i) {
    return (float)i;
}

double convert_i_to_d__(int i) {
    return (double)i;
}

char convert_i_to_c__(int i) {
    return (char)i;
}

Typeless_Vector__ *convert_f_to_s__(float f) {
    Typeless_Vector__ *tv = create_char_string__(10);
    snprintf((char*)tv->contents, 9, "%f", f);
    tv->current_size = strlen((char*)tv->contents);
    pop_off_char_string__(tv); //remove the trailing 0
    return tv;
}

int convert_f_to_i__(float f) {
    return (int)f;
}

double convert_f_to_d__(float f) {
    return (double)f;
}

char convert_f_to_c__(float f) {
    return (char)f;
}

Typeless_Vector__ *convert_d_to_s__(double d) {
    Typeless_Vector__ *tv = create_char_string__(10);
    snprintf((char*)tv->contents, 9, "%f", d);
    tv->current_size = strlen((char*)tv->contents);
    pop_off_char_string__(tv); //remove the trailing 0
    return tv;
}

int convert_d_to_i__(double d) {
    return (int)d;
}

float convert_d_to_f__(double d) {
    return (float)d;
}

char convert_f_to_c__(double d) {
    return (char)d;
}

Typeless_Vector__ *convert_c_to_s__(char c) {
    Typeless_Vector__ *tv = create_char_string__(10);
    snprintf((char*)tv->contents, 9, "%c", c);
    tv->current_size = strlen((char*)tv->contents);
    pop_off_char_string__(tv); //remove the trailing 0
    return tv;
}

int convert_c_to_i__(char c) {
    return (int)c;
}

double convert_c_to_d__(char c) {
    return (double)c;
}

float convert_c_to_f__(char c) {
    return (float)c;
}

