// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CODEGEN_HPP_
#define CODEGEN_HPP_

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

#include <string.h>
#include <stdlib.h>
#include "Common.hpp"
#include "LexParser.hpp"
#include "Type_Analyzer.hpp"

class Internal_Type { public: enum Type { ERROR, VOID, BOOL, INT, CHAR, FLOAT, DOUBLE, UINT, STRING, POINTER, OBJECT }; };

class Codegen {
    std::map<unsigned int, unsigned int> internal_type_map;

    //unsigned int cont_id;
    unsigned int temp_num;

    Function_Def *current_fun;

    std::string break_jmp_name;
    std::string catch_jmp_name;
public:
    void codegen_typesig(Program *p, unsigned int type_def_num, std::ostringstream &output);
    void codegen_typesig_no_tail(Program *p, unsigned int type_def_num, std::ostringstream &output);
    void codegen_tu_typesig(Program *p, unsigned int type_def_num, std::ostringstream &output);
    void codegen_default_value(Program *p, unsigned int type_def_num, std::ostringstream &output);

    void codegen_class_predecl(Program *p, Token *t, std::ostringstream &output);
    void codegen_class_decl(Program *p, Token *t, std::ostringstream &output);

    void codegen_block(Program *p, Token *t, std::ostringstream &output);
    void codegen_action_call(Program *p, Token *t, std::ostringstream &output);
    void codegen_fun_call(Program *p, Token *t, std::ostringstream &output);
    void codegen_method_call(Program *p, Token *t, std::ostringstream &output);
    void codegen_id(Program *p, Token *t, std::ostringstream &output);
    void codegen_symbol(Program *p, Token *t, std::ostringstream &output);

    void codegen_if(Program *p, Token *t, std::ostringstream &output);
    void codegen_try(Program *p, Token *t, std::ostringstream &output);
    void codegen_while(Program *p, Token *t, std::ostringstream &output);
    void codegen_for(Program *p, Token *t, std::ostringstream &output);

    void codegen_return(Program *p, Token *t, std::ostringstream &output);

    void codegen_array_call(Program *p, Token *t, std::ostringstream &output);
    void codegen_reference_feature(Program *p, Token *t, std::ostringstream &output);

    void codegen_copy_predecl(Program *p, unsigned int type_def_num, std::ostringstream &output);
    void codegen_delete_predecl(Program *p, std::ostringstream &output);
    void codegen_copy_decl(Program *p, unsigned int type_def_num, std::ostringstream &output);
    void codegen_delete_decl(Program *p, std::ostringstream &output);

    void codegen_concatenate(Program *p, Token *t, std::ostringstream &output);

    void codegen_new(Program *p, Token *t, std::ostringstream &output);
    void codegen_spawn(Program *p, Token *t, std::ostringstream &output);
    void codegen_token(Program *p, Token *t, std::ostringstream &output);
    void codegen_continuation_site(Program *p, Token *t, std::ostringstream &output);
    void codegen_deletion_site(Program *p, Token *t, std::ostringstream &output);

    void codegen_constructor_internal_predecl(Program *p, Token *t, std::ostringstream &output);
    void codegen_constructor_not_internal_predecl(Program *p, Token *t, std::ostringstream &output);
    void codegen_constructor_internal_decl(Program *p, Token *t, std::ostringstream &output);
    void codegen_constructor_not_internal_decl(Program *p, Token *t, std::ostringstream &output);

    void codegen_action_predecl(Program *p, Token *t, std::ostringstream &output);
    void codegen_action_decl(Program *p, Token *t, std::ostringstream &output);
    void codegen_fun_predecl(Program *p, Token *t, std::ostringstream &output);
    void codegen_fun_decl(Program *p, Token *t, std::ostringstream &output);
    void codegen_array_concat_decl(Program *p, std::ostringstream &output);

    void codegen_main_action(Program *p, std::ostringstream &output);

    void codegen_enum_pretty_print(Program *p, Token *t, std::ostringstream &output);

    void codegen(Program *p, Token *t, std::ostringstream &output);

    Codegen() : temp_num(0) { break_jmp_name = ""; catch_jmp_name = ""; }
};

#endif /* CODEGEN_HPP_ */
