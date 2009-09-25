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

void print_d_i__(double d, int depth) {
    switch (depth) {
        case (1) : printf("%.1f", d); break;
        case (2) : printf("%.2f", d); break;
        case (3) : printf("%.3f", d); break;
        case (4) : printf("%.4f", d); break;
        case (5) : printf("%.5f", d); break;
        case (6) : printf("%.6f", d); break;
        case (7) : printf("%.7f", d); break;
        case (8) : printf("%.8f", d); break;
        case (9) : printf("%.9f", d); break;
    }
}

void print_c__(char c) {
    printf("%c", c);
}

void print_f__(float f) {
    printf("%f", f);
}

/*
 * todo: do a proper file open
 */
void* file_open_read_file_s__(Typeless_Vector__ *s) {
    FILE *in;

    push_onto_char_string__(s, 0);
    in = fopen((char *)(s->contents), "rb");
    if (in == NULL) {
        return NULL;
    }
    pop_off_char_string__(s);
    return in;
}

/*
 * todo: do a proper file open
 */
void* file_open_write_file_s__(Typeless_Vector__ *s) {
    FILE *in;

    push_onto_char_string__(s, 0);
    in = fopen((char *)(s->contents), "wb");
    if (in == NULL) {
        return NULL;
    }
    pop_off_char_string__(s);
    return in;
}

void file_close_file_p__(void *p) {
    if (p != NULL) {
        FILE *in = (FILE*)p;
        fclose(in);
    }
}

/*
 * todo: do a proper file length
 */
int file_length_p__(void *p) {
    if (p != NULL) {
        FILE *in = (FILE*)p;
        unsigned int filelen;
        fseek(in, 0L, SEEK_END);
        filelen = ftell(in);
        fseek(in, 0L, SEEK_SET);

        return (int)filelen;
    }
    else {
        return -1;
    }
}

Typeless_Vector__ *file_read_all_p__(void *p) {
    if (p == NULL) return NULL;

    FILE *in = (FILE*)p;
    int length = file_length_p__(p);
    Typeless_Vector__ *tv = create_char_string__(length);
    tv->current_size = (unsigned int)length;

    int read = fread(tv->contents, length, 1, in);

    if (read != 1) {
        //printf("Warning: file contents could not be completely read.\n");
        delete_char_string__(tv);
        return NULL;
    }

    return tv;
}

Typeless_Vector__ *file_read_line_p__(void *p) {
    if (p == NULL) return NULL;

    FILE *in = (FILE*)p;
    if (feof(in)) {
        return NULL;
    }

    Typeless_Vector__ *tv = create_char_string__(0);

    char c = fgetc(in);
    while (!feof(in)) {
        push_onto_char_string__(tv, c);
        c = getc(in);
        if (c == '\n') {
            return tv;
        }
        else if (c == '\r') {
            if (!feof(in)) {
                c = fgetc(in);

                if (c == '\n') {
                    return tv;
                }
                else {
                    push_onto_char_string__(tv, '\r');
                }
            }
        }
    }

    return tv;
}

BOOL file_write_p_s__(void *p, Typeless_Vector__ *tv) {
    if (p == NULL) return FALSE;

    FILE *out = (FILE*)p;

    if (fwrite(tv->contents, tv->current_size, 1, out) != 1) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

BOOL file_eof_p__(void *p) {
    if (p == NULL) {
        return TRUE;
    }
    if (feof((FILE*)p)) {
        return TRUE;
    }
    else {
        return FALSE;
    }
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
    Typeless_Vector__ *tv = create_char_string__(15);
#if _MSC_VER > 1000
    _snprintf((char*)tv->contents, 14, "%i", i);
#else 
    snprintf((char*)tv->contents, 14, "%i", i);
#endif
    tv->current_size = strlen((char*)tv->contents);
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
    Typeless_Vector__ *tv = create_char_string__(15);
#if _MSC_VER > 1000
    _snprintf((char*)tv->contents, 14, "%f", f);
#else
    snprintf((char*)tv->contents, 14, "%f", f);
#endif // _MSC_VER > 1000 
    tv->current_size = strlen((char*)tv->contents);
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
    Typeless_Vector__ *tv = create_char_string__(15);
#if _MSC_VER > 1000
    _snprintf((char*)tv->contents, 14, "%f", d);
#else
    snprintf((char*)tv->contents, 14, "%f", d);
#endif
    tv->current_size = strlen((char*)tv->contents);
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
    Typeless_Vector__ *tv = create_char_string__(5);
#if _MSC_VER > 1000
    _snprintf((char*)tv->contents, 4, "%c", c);
#else
    snprintf((char*)tv->contents, 4, "%c", c);
#endif
    tv->current_size = strlen((char*)tv->contents);
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

/* STRING */

Typeless_Vector__ *substr_s_i_i__(Typeless_Vector__ *s, int start, int length) {
    if (length < 0) { length = 0; }
    Typeless_Vector__ *retval = create_typeless_vector__(sizeof(char), length);

    if ((unsigned)start > s->current_size) {
        return retval;
    }

    if ((unsigned)(start+length) > s->current_size) {
        length = s->current_size - start;
    }

    for (int i = start; i < (start+length); ++i) {
        push_onto_char_string__(retval, ((char*)s->contents)[i]);
    }

    return retval;
}
