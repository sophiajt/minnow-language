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

void exit_i__(int i);
void exit__();

#ifdef __cplusplus
}
#endif

#endif /* MINNOW_PRELUDE_HPP_ */
