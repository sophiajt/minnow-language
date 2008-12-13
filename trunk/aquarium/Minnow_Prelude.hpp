// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef MINNOW_PRELUDE_HPP_
#define MINNOW_PRELUDE_HPP_


#ifdef __cplusplus
extern "C" {
#endif

#include "Char_String.hpp"

void print_i__(int i);
void print_s__(Typeless_Vector__ *s);
void print_d__(double d);
void print_c__(char c);
void print_f__(float f);
int convert_s_to_i__(Typeless_Vector__ *s);
double convert_s_to_d__(Typeless_Vector__ *s);
char convert_s_to_c__(Typeless_Vector__ *s);
float convert_s_to_f__(Typeless_Vector__ *s);
Typeless_Vector__ *convert_i_to_s__(int i);
double convert_i_to_d__(int i);
char convert_i_to_c__(int i);
float convert_i_to_f__(int i);
Typeless_Vector__ *convert_d_to_s__(double d);
int convert_d_to_i__(double d);
char convert_d_to_c__(double d);
float convert_d_to_f__(double d);
Typeless_Vector__ *convert_c_to_s__(char c);
double convert_c_to_d__(char c);
int convert_c_to_i__(char c);
float convert_c_to_f__(char c);
Typeless_Vector__ *convert_f_to_s__(float f);
double convert_f_to_d__(float f);
char convert_f_to_c__(float f);
int convert_f_to_i__(float f);

void exit_i__(int i);
void exit__();

#ifdef __cplusplus
}
#endif

#endif /* MINNOW_PRELUDE_HPP_ */
