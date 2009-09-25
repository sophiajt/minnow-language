// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include "Common.hpp"
#include "Codegen_C.hpp"

void Codegen::codegen_typesig(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    Type_Def *td = p->types[type_def_num];

    if (td->container == Container_Type::ARRAY) {
        output << "Typeless_Vector__*";
    }
    else if (td->container == Container_Type::DICT) {
        output << "Typeless_Dictionary__*";
    }
    else if (td->is_internal) {
        switch (internal_type_map[type_def_num]) {
            case (Internal_Type::VOID) : output << "void"; break;
            case (Internal_Type::BOOL) : output << "BOOL"; break;
            case (Internal_Type::INT) : output << "int"; break;
            case (Internal_Type::UINT) : output << "unsigned int"; break;
            case (Internal_Type::FLOAT) : output << "float"; break;
            case (Internal_Type::DOUBLE) : output << "double"; break;
            case (Internal_Type::STRING) : output << "Typeless_Vector__*"; break;
            case (Internal_Type::CHAR) : output << "char"; break;
            case (Internal_Type::POINTER) : output << "void*"; break;
            case (Internal_Type::OBJECT) : output << "void*"; break;
        }
    }
    else if (td->token->type == Token_Type::ENUM_DEF) {
        output << "int";
    }
    else {
        output << "struct type__" << type_def_num << "*";
    }
}

void Codegen::codegen_typesig_no_tail(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    Type_Def *td = p->types[type_def_num];

    if (td->container == Container_Type::ARRAY) {
        output << "Typeless_Vector__";
    }
    else if (td->container == Container_Type::DICT) {
        output << "Typeless_Dictionary__";
    }
    else if (td->is_internal) {
        switch (internal_type_map[type_def_num]) {
            case (Internal_Type::VOID) : output << "void"; break;
            case (Internal_Type::BOOL) : output << "BOOL"; break;
            case (Internal_Type::INT) : output << "int"; break;
            case (Internal_Type::UINT) : output << "unsigned int"; break;
            case (Internal_Type::FLOAT) : output << "float"; break;
            case (Internal_Type::DOUBLE) : output << "double"; break;
            case (Internal_Type::STRING) : output << "Typeless_Vector__"; break;
            case (Internal_Type::CHAR) : output << "char"; break;
            case (Internal_Type::POINTER) : output << "void*"; break;
            case (Internal_Type::OBJECT) : output << "void*"; break;
        }
    }
    else if (td->token->type == Token_Type::ENUM_DEF) {
        output << "int";
    }
    else {
        output << "struct type__" << type_def_num;
    }
}

void Codegen::codegen_tu_typesig(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    Type_Def *td = p->types[type_def_num];

    if (td->container == Container_Type::ARRAY) {
        output << "VoidPtr";
    }
    else if (td->container == Container_Type::DICT) {
        output << "VoidPtr";
    }
    else if (td->is_internal) {
        switch (internal_type_map[type_def_num]) {
            case (Internal_Type::BOOL) : output << "Bool"; break;
            case (Internal_Type::INT) : output << "Int32"; break;
            case (Internal_Type::UINT) : output << "UInt32"; break;
            case (Internal_Type::FLOAT) : output << "Float"; break;
            case (Internal_Type::DOUBLE) : output << "Double"; break;
            case (Internal_Type::STRING) : output << "VoidPtr"; break;
            case (Internal_Type::CHAR) : output << "Int8"; break;
            case (Internal_Type::POINTER) : output << "VoidPtr"; break;
            case (Internal_Type::OBJECT) : output << "VoidPtr"; break;
        }
    }
    else if (td->token->type == Token_Type::ENUM_DEF) {
        output << "Int32";
    }
    else {
        output << "VoidPtr";
    }
}

void Codegen::codegen_default_value(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    Type_Def *td = p->types[type_def_num];
    if (td->container == Container_Type::ARRAY) {
        output << "NULL";
    }
    else if (td->container == Container_Type::DICT) {
        output << "NULL";
    }
    else if (td->is_internal) {
        switch (internal_type_map[type_def_num]) {
            case (Internal_Type::VOID) : break;
            case (Internal_Type::BOOL) : output << "FALSE"; break;
            case (Internal_Type::INT) : output << "0"; break;
            case (Internal_Type::UINT) : output << "0"; break;
            case (Internal_Type::FLOAT) : output << "0.0"; break;
            case (Internal_Type::DOUBLE) : output << "0.0"; break;
            case (Internal_Type::STRING) : output << "NULL"; break;
            case (Internal_Type::CHAR) : output << "0"; break;
            case (Internal_Type::POINTER) : output << "NULL"; break;
            case (Internal_Type::OBJECT) : output << "NULL"; break;
        }
    }
    else if (td->token->type == Token_Type::ENUM_DEF) {
        output << "-1";
    }
    else {
        output << "NULL";
    }
}


void Codegen::codegen_block(Program *p, Token *t, std::ostringstream &output) {
    //output << "{" << std::endl;

    for (unsigned int i = 0; i < t->children.size(); ++i) {
        codegen_token(p, t->children[i], output);
        output << ";" << std::endl;
    }

    //output << "}" << std::endl;
}

void Codegen::codegen_fun_call(Program *p, Token *t, std::ostringstream &output) {
    Function_Def *fd = p->funs[t->definition_number];

    if (fd->is_internal == true) {
        Function_Def *owner = this->current_fun;

        if (t->children[0]->contents == "throw") {
            output << "delete__(m__, ((Actor__*)m__->recipient)->exception, " << p->global->local_types["object"] << ");" << std::endl;
            output << "((Actor__*)m__->recipient)->exception = ";
            codegen_token(p, t->children[1], output);
            output << ";" << std::endl;
            if (this->catch_jmp_name != "") {
                output << "goto " << this->catch_jmp_name << ";" << std::endl;
            }
            else {
                output << "((Actor__*)m__->recipient)->timeslice_remaining = 0;" << std::endl;
                if (owner->token->type == Token_Type::ACTION_DEF) {
                    output << "print_s__(create_char_string_from_char_ptr__(\"Uncaught exception\\n\"));" << std::endl;
                    output << "exit_i__(1); " << std::endl;
                }
                else {
                    output << "return ";
                    codegen_default_value(p, owner->return_type_def_num, output);
                    output << ";" << std::endl;
                }
            }
        }
        else if (t->children[0]->contents == "print") {
            output << "print_enum_" << t->children[1]->type_def_num << "__(";
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (fd->external_name != "") {
        output << fd->external_name << "(";
        if (t->children.size() > 1) {
            codegen_token(p, t->children[1], output);
        }
        output << ")";
    }
    else {
        output << "fun__" << t->definition_number << "(m__";

        if (t->children.size() > 1) {
            output << ", ";
            codegen_token(p, t->children[1], output);
        }
        output << ")";
    }
}

void Codegen::codegen_method_call(Program *p, Token *t, std::ostringstream &output) {
    Function_Def *fd = p->funs[t->children[1]->definition_number];

    if (fd->is_internal == false) {
        output << "fun__" << t->children[1]->definition_number << "(m__, ";
        codegen_token(p, t->children[0], output);

        if (t->children[1]->children.size() > 1) {
            output << ", ";
            codegen_token(p, t->children[1]->children[1], output);
        }
        output << ")";
    }
    else {
        Token *child = t->children[1];
        Type_Def *td = p->types[t->children[0]->type_def_num];
        if ((child->children[0]->contents == "push") && (td->container == Container_Type::ARRAY)) {
            ++temp_num;
            output << "{ ";
            codegen_typesig(p, td->contained_type_def_nums[0], output);
            output << " tmp_member__" << temp_num << " = ";
            codegen_token(p, child->children[1], output);
            output << "; push_onto_typeless_vector__(";
            codegen_token(p, t->children[0], output);
            output << ", &tmp_member__" << temp_num;
            output << "); }";
        }
        else if ((child->children[0]->contents == "size") && (td->container == Container_Type::ARRAY)) {
            codegen_token(p, t->children[0], output);
            output << "->";
            output << "current_size";
        }
        else if ((child->children[0]->contents == "empty") && (td->container == Container_Type::ARRAY)) {
            codegen_token(p, t->children[0], output);
            output << "->";
            output << "current_size == 0";
        }
        else if ((child->children[0]->contents == "pop") && (td->container == Container_Type::ARRAY)) {
            output << "pop_off_typeless_vector__(";
            codegen_token(p, t->children[0], output);
            output << ").";
            codegen_tu_typesig(p, td->contained_type_def_nums[0], output);
        }
        else if ((child->children[0]->contents == "insert") && (td->container == Container_Type::ARRAY)) {
            ++temp_num;
            output << "{ ";
            codegen_typesig(p, td->contained_type_def_nums[0], output);
            output << " tmp_member__" << temp_num << " = ";
            codegen_token(p, child->children[1]->children[0], output);
            output << "; insert_into_typeless_vector__(";
            codegen_token(p, t->children[0], output);
            output << ", &tmp_member__" << temp_num << ", ";
            codegen_token(p, child->children[1]->children[1], output);
            output << "); }";
        }
        else if ((child->children[0]->contents == "delete") && (td->container == Container_Type::ARRAY)) {
            output << "delete_from_typeless_vector__(";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, child->children[1], output);
            output << ").";
            codegen_tu_typesig(p, td->contained_type_def_nums[0], output);
        }
        else if ((child->children[0]->contents == "has_key") && (td->container == Container_Type::DICT)) {
            output << "contains_key_in_dictionary__(";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "bit_shl") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << " << ";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "bit_shr") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << " >> ";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "bit_xor") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << " ^ ";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "bit_or") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << " | ";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "bit_and") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << " & ";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "bit_not") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "(~";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if (child->children[0]->contents == "is_null") {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << " == NULL)";
        }
        else if (child->children[0]->contents == "exists") {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << " != NULL)";
        }

        else if (child->children[0]->contents == "delete") {
            output << "delete__(m__, ";
            codegen_token(p, t->children[0], output);
            output << ", " << t->children[0]->type_def_num;
            output << "); ";
            codegen_token(p, t->children[0], output);
            output << " = NULL;" << std::endl;
        }
        else if (child->children[0]->contents == "copy") {
            codegen_token(p, t->children[0], output);
            output << " = (";
            codegen_typesig(p, t->children[0]->type_def_num, output);
            output << ")copy__(m__, ";
            codegen_token(p, child->children[1], output);
            output << ", " << t->children[0]->type_def_num;
            output << "); ";
        }

        else if ((child->children[0]->contents == "to_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            output << "convert_s_to_i__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            output << "convert_s_to_f__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            output << "convert_s_to_d__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            output << "convert_s_to_c__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "convert_i_to_s__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "convert_i_to_f__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "convert_i_to_d__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            output << "convert_i_to_c__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            output << "convert_d_to_s__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            output << "convert_d_to_f__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            output << "convert_d_to_i__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            output << "convert_d_to_c__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            output << "convert_f_to_s__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            output << "convert_f_to_i__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            output << "convert_f_to_d__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            output << "convert_f_to_c__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            output << "convert_c_to_s__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            output << "convert_c_to_f__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            output << "convert_c_to_d__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "to_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            output << "convert_c_to_i__(";
            codegen_token(p, t->children[0], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_i_to_s__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_f_to_s__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_d_to_s__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["string"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_c_to_s__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_s_to_i__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_f_to_i__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_d_to_i__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["int"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_c_to_i__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_s_to_f__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_i_to_f__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_d_to_f__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["float"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_c_to_f__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_s_to_d__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_i_to_d__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_f_to_d__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_char") && (t->children[0]->type_def_num == (signed)p->global->local_types["double"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_c_to_d__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_string") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_s_to_c__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_int") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_i_to_c__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_float") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_f_to_c__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
        else if ((child->children[0]->contents == "from_double") && (t->children[0]->type_def_num == (signed)p->global->local_types["char"])) {
            codegen_token(p, t->children[0], output);
            output << " = convert_d_to_c__(";
            codegen_token(p, child->children[1], output);
            output << ")";
        }
    }
}

void Codegen::codegen_id(Program *p, Token *t, std::ostringstream &output) {
    output << "var__" << t->definition_number;
}

void Codegen::codegen_action_call(Program *p, Token *t, std::ostringstream &output) {
    output << "{";
    output << "Message__ *msg__ = get_msg_from_cache__(m__->sched);" << std::endl;
    output << "msg__->act_data.task = fun__" << t->children[1]->definition_number  << ";" << std::endl;
    output << "msg__->recipient = ";
    codegen_token(p, t->children[0], output);
    output << ";" << std::endl;
    output << "msg__->message_type = MESSAGE_TYPE_ACTION;" << std::endl;

    Function_Def *fd = p->funs[t->children[1]->definition_number];
    unsigned int num_args = fd->arg_def_nums.size();
    if (num_args > 0) {
	    Token *arg = t->children[1]->children[1];
        for (int i = (num_args - 1); i >= 0; --i) {
            if (i > 0) {
                output << "msg__->args[" << i << "].";
                codegen_tu_typesig(p, arg->children[1]->type_def_num, output);
                output << " = ";
                codegen_token(p, arg->children[1], output);
                arg = arg->children[0];
                output << ";" << std::endl;
            }
            else if (i == 0) {
                output << "msg__->args[" << i << "].";
                codegen_tu_typesig(p, arg->type_def_num, output);
                output << " = ";
                codegen_token(p, arg, output);
                output << ";" << std::endl;
            }
        }
    }
    output << "mail_to_actor__(msg__, (Actor__*)(m__->recipient));}" << std::endl;
}

void Codegen::codegen_symbol(Program *p, Token *t, std::ostringstream &output) {
    if (t->contents == ",") {
        codegen_token(p, t->children[0], output);
        output << t->contents;
        codegen_token(p, t->children[1], output);
    }
    else if (t->contents == "=") {
        int string_id = p->global->local_types["string"];

        if ((t->children[0]->type_def_num == string_id) && (t->children[0]->type != Token_Type::VAR_DECL)) {
        //else if (is_complex_type(p, t->children[0]->type_def_num)) {
            /*
            output << "safe_eq__(m__, (void**)&";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ", " << t->children[0]->type_def_num << ")";
            */

            codegen_token(p, t->children[0], output);
            output << " = ";
            codegen_token(p, t->children[1], output);
        }
        else if (t->children[1]->type == Token_Type::ARRAY_INIT) {
            Token *child = t->children[1];
            codegen_token(p, t->children[0], output);
            output << "=";
            output << "create_typeless_vector__(sizeof(";
            codegen_typesig(p, child->children[0]->type_def_num, output);
            output << "), " << child->children.size() << ");" << std::endl;
            codegen_token(p, t->children[0], output);
            output << "->current_size = " << child->children.size() << ";" << std::endl;

            for (unsigned int k = 0; k < child->children.size(); ++k) {
                output << "INDEX_AT__(";
                codegen_token(p, t->children[0], output);
                output << ", " << k << ", ";
                codegen_typesig(p, child->children[0]->type_def_num, output);
                output << ") = ";
                codegen_token(p, child->children[k], output);
                output << ";" << std::endl;
            }
        }
        else {
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
        }
    }
    else if (t->contents == "==") {
        int string_id = p->global->local_types["string"];
        if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == string_id)) {
            output << "compare_char_string__(";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ") == 0";
        }
        else {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (t->contents == "!=") {
        int string_id = p->global->local_types["string"];
        if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == string_id)) {
            output << "compare_char_string__(";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ") != 0";
        }
        else {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (t->contents == "<") {
        int string_id = p->global->local_types["string"];
        if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == string_id)) {
            output << "compare_char_string__(";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ") < 0";
        }
        else {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (t->contents == ">") {
        int string_id = p->global->local_types["string"];
        if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == string_id)) {
            output << "compare_char_string__(";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ") > 0";
        }
        else {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (t->contents == "<=") {
        int string_id = p->global->local_types["string"];
        if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == string_id)) {
            output << "compare_char_string__(";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ") <= 0";
        }
        else {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (t->contents == ">=") {
        int string_id = p->global->local_types["string"];
        if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == string_id)) {
            output << "compare_char_string__(";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ") >= 0";
        }
        else {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (t->contents == ":") {
        codegen_token(p, t->children[0], output);
    }
    else if (t->contents == "**") {
        output << "(";
        codegen_typesig(p, t->children[0]->type_def_num, output);
        output << ")";
        output << "pow(";
        codegen_token(p, t->children[0], output);
        output << ",";
        codegen_token(p, t->children[1], output);
        output << ")";
    }
    else if (t->contents == "<+") {
        if (t->children[0]->type_def_num == (signed)p->global->local_types["object"]) {
            /*
            if (t->children[0]->type == Token_Type::VAR_DECL) {
                codegen_token(p, t->children[0], output);
                output << " = add_primary_feature__((Object_Feature__*)";
                output << "var__" << t->children[0]->children[0]->definition_number;
                output << ", (Object_Feature__ *)";
                codegen_token(p, t->children[1], output);
                output << ")";

            }
            else {
                output << "add_primary_feature__((Object_Feature__*)(";
                codegen_token(p, t->children[0], output);
                output << "), (Object_Feature__ *)";
                codegen_token(p, t->children[1], output);
                output << ")";
            }
            */
            if (t->children[0]->contents != "<+") {
                codegen_token(p, t->children[0], output);
                output << " = add_primary_feature__((Object_Feature__*)";
                codegen_token(p, t->children[0], output);
                output << ", (Object_Feature__ *)";
                codegen_token(p, t->children[1], output);
                output << ")";
            }
            else {
                output << "add_primary_feature__((Object_Feature__*)(";
                codegen_token(p, t->children[0], output);
                output << "), (Object_Feature__ *)";
                codegen_token(p, t->children[1], output);
                output << ")";
            }
        }
        else {
            output << "add_child_feature__((Object_Feature__*)(";
            codegen_token(p, t->children[0], output);
            output << "), (Object_Feature__ *)";
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
    else if (t->contents == "+>") {
        output << "(";
        codegen_token(p, t->children[0], output);
        output << " = remove_feature__((Object_Feature__ *)";
        codegen_token(p, t->children[1], output);
        output << ") )";
    }
    else {
        if (p->funs[t->definition_number]->is_internal) {
            output << "(";
            codegen_token(p, t->children[0], output);
            output << t->contents;
            codegen_token(p, t->children[1], output);
            output << ")";
        }
        else {
            output << "fun__" << t->definition_number << "(m__, ";
            codegen_token(p, t->children[0], output);
            output << ", ";
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
}

void Codegen::codegen_concatenate(Program *p, Token *t, std::ostringstream &output) {
    int string_id = p->global->local_types["string"];
    int char_id = p->global->local_types["char"];
    if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == string_id)) {
        output << "concatenate_new_char_string__(";
        codegen_token(p, t->children[0], output);
        output << ", ";
        codegen_token(p, t->children[1], output);
        output << ")";
    }
    else if ((t->children[0]->type_def_num == char_id) && (t->children[1]->type_def_num == string_id)) {
        output << "insert_into_new_char_string__(";
        codegen_token(p, t->children[1], output);
        output << ", ";
        codegen_token(p, t->children[0], output);
        output << ", 0)";
    }
    else if ((t->children[0]->type_def_num == string_id) && (t->children[1]->type_def_num == char_id)) {
        output << "push_onto_new_char_string__(";
        codegen_token(p, t->children[0], output);
        output << ", ";
        codegen_token(p, t->children[1], output);
        output << ")";
    }
    else {
        output << "(";
        codegen_token(p, t->children[0], output);
        output << t->contents;
        codegen_token(p, t->children[1], output);
        output << ")";
    }
}

void Codegen::codegen_if(Program *p, Token *t, std::ostringstream &output) {
    unsigned int block_start = this->temp_num++;
    unsigned int block_skip = this->temp_num++;
    unsigned int block_end = this->temp_num++;

    if (t->children[1]->children.size() > 1) {
        for (unsigned int i = 0; i < t->children[1]->children.size() - 1; ++i) {
            codegen_token(p, t->children[1]->children[i], output);
            output << ";" << std::endl;
        }
        output << "if (";
        codegen_token(p, t->children[1]->children[t->children[1]->children.size() - 1], output);
        output << ") goto ifjmp" << block_start << "; else goto ifjmp" << block_skip << ";" << std::endl;
    }
    else {
        output << "if (";
        codegen_token(p, t->children[1]->children[0], output);
        output << ") goto ifjmp" << block_start << "; else goto ifjmp" << block_skip << ";" << std::endl;
    }

    output << "ifjmp" << block_start << ":" << std::endl;

    if (t->children.size() > 2) {
        //output << "{" << std::endl;

        codegen_block(p, t->children[2], output);
        output << "goto ifjmp" << block_end << ";" << std::endl;
        output << "ifjmp" << block_skip << ":" << std::endl;
        //output << "}" << std::endl;
        for (unsigned int i = 3; i < t->children.size(); ++i) {
            if (t->children[i]->type == Token_Type::ELSEIF_BLOCK) {
                Token *child = t->children[i];

                unsigned int ei_block_start = this->temp_num++;
                unsigned int ei_block_skip = this->temp_num++;

                if (child->children[1]->children.size() > 1) {
                    for (unsigned int j = 0; j < child->children[1]->children.size() - 1; ++j) {
                        codegen_token(p, child->children[1]->children[j], output);
                        output << ";" << std::endl;
                    }
                    output << "if (";
                    codegen_token(p, child->children[1]->children[child->children[1]->children.size() - 1], output);
                    output << ") goto ifjmp" << ei_block_start << "; else goto ifjmp" << ei_block_skip << ";" << std::endl;
                }
                else {
                    output << "if (";
                    codegen_token(p, child->children[1]->children[0], output);
                    output << ") goto ifjmp" << ei_block_start << "; else goto ifjmp" << ei_block_skip << ";" << std::endl;
                }
                output << "ifjmp" << ei_block_start << ":" << std::endl;
                if (child->children.size() > 2) {
                    codegen_block(p, child->children[2], output);
                }
                output << "goto ifjmp" << block_end << ";" << std::endl;
                output << "ifjmp" << ei_block_skip << ":" << std::endl;
            }
            else if (t->children[i]->type == Token_Type::ELSE_BLOCK) {
                Token *child = t->children[i];

                if (child->children.size() > 1) {
                    codegen_block(p, child->children[1], output);
                }
                output << "goto ifjmp" << block_end << ";" << std::endl;
            }
        }
    }
    output << "ifjmp" << block_end << ":" << std::endl;
}

void Codegen::codegen_try(Program *p, Token *t, std::ostringstream &output) {
    std::ostringstream catch_name;
    unsigned int catch_start = this->temp_num++;
    unsigned int catch_end = this->temp_num++;
    std::string prev_catch = this->catch_jmp_name;
    catch_name << "catchjmp" << catch_start;
    this->catch_jmp_name = catch_name.str();

    for (unsigned int i = 0; i < t->children[1]->children.size(); ++i) {
        codegen_token(p, t->children[1]->children[i], output);
        output << ";" << std::endl;
    }
    output << "goto catchjmp" << catch_end << ";" << std::endl;

    this->catch_jmp_name = prev_catch;

    output << catch_name.str() << ":" << std::endl;
    output << "delete__(m__, exception__, " << p->global->local_types["object"] << ");" << std::endl;
    output << "exception__ = ((Actor__*)m__->recipient)->exception;" << std::endl;
    output << "((Actor__*)m__->recipient)->exception = NULL;" << std::endl;
    for (unsigned int i = 1; i < t->children[2]->children.size(); ++i) {
        codegen_token(p, t->children[2]->children[i], output);
        output << ";" << std::endl;
    }
    output << "delete__(m__, exception__, " << p->global->local_types["object"] << "); exception__ = NULL;" << std::endl;
    output << "catchjmp" << catch_end << ":" << std::endl;
}
/*
void Codegen::codegen_while(Program *p, Token *t, std::ostringstream &output) {
    unsigned int top = this->temp_num++;
    unsigned int block_start = this->temp_num++;
    unsigned int block_end = this->temp_num++;
    bool cont_at_end = false;

    if (t->children[2]->children.size() > 1) {
        if (t->children[2]->children.back()->type == Token_Type::CONTINUATION_SITE) {
            if (t->children[2]->children.back()->children.size() == 0) {
                cont_at_end = true;
            }
        }
    }

    if (cont_at_end) {
        output << "while(1) {" << std::endl;
        output << "while(--timeslice__ > 0) {" << std::endl;
        if (t->children[1]->children.size() > 1) {
            output << "if (!(";
            codegen_token(p, t->children[1]->children[t->children[1]->children.size() - 1], output);
            output << ")) break;" << std::endl;
        }
        else {
            output << "if (!(";
            codegen_token(p, t->children[1]->children[0], output);
            output << ")) break;" << std::endl;
        }

        for (unsigned int i = 0; i < t->children[2]->children.size() - 1; ++i) {
            codegen_token(p, t->children[2]->children[i], output);
            output << ";" << std::endl;
        }
        //codegen_block(p, t->children[2], output);
        output << "}" << std::endl;
        output << "if (timeslice__ > 0) {" << std::endl;
        output << "  break;" << std::endl;
        output << "} else {" << std::endl;
        codegen_token(p, t->children[2]->children.back(), output);
        output << "}" << std::endl;
        output << "}" << std::endl;
    }
    else {
        output << "whilejmp" << top << ":" << std::endl;
        if (t->children[1]->children.size() > 1) {
            if (cont_at_end == false) {
                for (unsigned int i = 0; i < t->children[1]->children.size() - 1; ++i) {
                    codegen_token(p, t->children[1]->children[i], output);
                    output << ";" << std::endl;
                }
            }
            output << "if (";
            codegen_token(p, t->children[1]->children[t->children[1]->children.size() - 1], output);
            output << ") goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
        }
        else {
            output << "if (";
            codegen_token(p, t->children[1]->children[0], output);
            output << ") goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
        }

        output << "whilejmp" << block_start << ":" << std::endl;
        codegen_block(p, t->children[2], output);
        if (cont_at_end) {
            for (unsigned int i = 0; i < t->children[1]->children.size() - 1; ++i) {
                codegen_token(p, t->children[1]->children[i], output);
                output << ";" << std::endl;
            }
        }
        output << "goto whilejmp" << top << ";" << std::endl;
        output << "whilejmp" << block_end << ":" << std::endl;

    }
}
*/

void Codegen::codegen_while(Program *p, Token *t, std::ostringstream &output) {
    unsigned int top = this->temp_num++;
    unsigned int block_start = this->temp_num++;
    unsigned int block_end = this->temp_num++;
    bool cont_at_end = false;

    std::string prev_break = break_jmp_name;
    std::ostringstream jmp_name;
    jmp_name << "whilejmp" << block_end;
    break_jmp_name = jmp_name.str();

    if (t->children[1]->children.size() > 1) {
        if (t->children[1]->children[0]->type == Token_Type::CONTINUATION_SITE) {
            if (t->children[1]->children[0]->children.size() == 0) {
                cont_at_end = true;
            }
        }
    }

    output << "whilejmp" << top << ":" << std::endl;
    if (t->children[1]->children.size() > 1) {
        if (cont_at_end == false) {
            for (unsigned int i = 0; i < t->children[1]->children.size() - 1; ++i) {
                codegen_token(p, t->children[1]->children[i], output);
                output << ";" << std::endl;
            }
        }
        output << "if (";
        codegen_token(p, t->children[1]->children[t->children[1]->children.size() - 1], output);
        output << ") goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
    }
    else {
        output << "if (";
        codegen_token(p, t->children[1]->children[0], output);
        output << ") goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
    }

    output << "whilejmp" << block_start << ":" << std::endl;
    codegen_block(p, t->children[2], output);
    if (cont_at_end) {
        for (unsigned int i = 0; i < t->children[1]->children.size() - 1; ++i) {
            codegen_token(p, t->children[1]->children[i], output);
            output << ";" << std::endl;
        }
    }
    output << "goto whilejmp" << top << ";" << std::endl;
    output << "whilejmp" << block_end << ":" << std::endl;

    break_jmp_name = prev_break;
}

void Codegen::codegen_for(Program *p, Token *t, std::ostringstream &output) {
    unsigned int block_start = this->temp_num++;
    unsigned int block_start2 = this->temp_num++;
    unsigned int block_end = this->temp_num++;
    unsigned int block_end2 = this->temp_num++;

    std::string prev_break = break_jmp_name;
    std::ostringstream jmp_name;
    jmp_name << "forjmp" << block_end;
    break_jmp_name = jmp_name.str();

    output << "{ " << std::endl;
    codegen_token(p, t->children[1], output);
    output << ";" << std::endl;
    output << "forjmp" << block_start << ":" << std::endl;

    output << "if ((";
    codegen_token(p, t->children[1]->children[0], output);
    output << ")==(";
    codegen_token(p, t->children[2], output);
    output << "+1)) goto forjmp" << block_end << ";" << std::endl;
    output << "if ((";
    codegen_token(p, t->children[1]->children[0], output);
    output << ") + timeslice__ >= ";
    codegen_token(p, t->children[2], output);
    output << "+1) {" << std::endl;
    output << "  var__" << t->definition_number << " = ";
    codegen_token(p, t->children[2], output);
    output << "+1;" << std::endl;
    output << "  var__" << t->definition_number+1 << " = ";
    codegen_token(p, t->children[2], output);
    output << "+1 - ";
    codegen_token(p, t->children[1]->children[0], output);
    output << ";" << std::endl;

    output << "} else {" << std::endl;
    output << "  var__" << t->definition_number << " = ";
    output << "(";
    codegen_token(p, t->children[1]->children[0], output);
    output << ") + timeslice__;" << std::endl;
    output << "  var__" << t->definition_number+1 << " = timeslice__;" << std::endl;
    output << "}" << std::endl;

    output << "forjmp" << block_start2 << ":" << std::endl;
    output << "if ((";
    codegen_token(p, t->children[1]->children[0], output);
    output << ")== var__" << t->definition_number << ") goto forjmp" << block_end2 << ";" << std::endl;
    codegen_block(p, t->children[3], output);
    output << "++";
    codegen_token(p, t->children[1]->children[0], output);
    output << ";" << std::endl;
    output << "goto forjmp" << block_start2 << ";" << std::endl;

    output << "forjmp" << block_end2 << ":" << std::endl;

    output << "  timeslice__ -= var__" << t->definition_number+1 << ";" << std::endl;

    output << "  if (timeslice__ <= 0) { timeslice__ = 1;}" << std::endl;


    //output << "timeslice__ = 1;" << std::endl;
    codegen_continuation_site(p, t->children[4], output);
    output << "goto forjmp" << block_start << ";}" << std::endl;
    output << "forjmp" << block_end << ":" << std::endl;

    /*
    codegen_token(p, t->children[1], output);
    output << ";" << std::endl;
    output << "forjmp" << top << ":" << std::endl;
    output << "if (";
    codegen_token(p, t->children[1]->children[0], output);
    output << " == (";
    codegen_token(p, t->children[2], output);
    output << "+1)) goto forjmp" << block_end << ";" << std::endl;

    output << "forjmp" << block_start << ":" << std::endl;
    codegen_block(p, t->children[3], output);
    output << "++";
    codegen_token(p, t->children[1]->children[0], output);
    output << ";" << std::endl;
    output << "goto forjmp" << top << ";" << std::endl;
    output << "forjmp" << block_end << ":" << std::endl;
    */

    break_jmp_name = prev_break;
}


/*
void Codegen::codegen_while(Program *p, Token *t, std::ostringstream &output) {
    unsigned int top = this->temp_num++;
    unsigned int block_start = this->temp_num++;
    unsigned int block_end = this->temp_num++;
    bool cont_at_end = false;

    if (t->children[2]->children.size() > 0) {
        Token *last_token = t->children[2]->children[t->children[2]->children.size() - 1];
        if (last_token->type == Token_Type::CONTINUATION_SITE) {
            if (last_token->children.size() == 0) {
                cont_at_end = true;
            }
        }
    }

    output << "whilejmp" << top << ":" << std::endl;
    if (t->children[1]->children.size() > 1) {
        for (unsigned int i = 0; i < t->children[1]->children.size() - 1; ++i) {
            codegen_token(p, t->children[1]->children[i], output);
            output << ";" << std::endl;
        }
        output << "if (";
        codegen_token(p, t->children[1]->children[t->children[1]->children.size() - 1], output);
        if (cont_at_end) {
            output << " && (--timeslice__ > 0)) goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
        }
        else {
            output << ") goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
        }
    }
    else {
        output << "if (";
        codegen_token(p, t->children[1]->children[0], output);
        if (cont_at_end) {
            output << " && (--timeslice__ > 0)) goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
        }
        else {
            output << ") goto whilejmp" << block_start << "; else goto whilejmp" << block_end << ";" << std::endl;
        }
    }

    output << "whilejmp" << block_start << ":" << std::endl;
    if (cont_at_end) {
        for (unsigned int i = 0; i < t->children[2]->children.size() - 1; ++i) {
            codegen_token(p, t->children[2]->children[i], output);
            output << ";" << std::endl;
        }
        output << "case(" << (this->cont_id+1) << "):" << std::endl;
    }
    else {
        for (unsigned int i = 0; i < t->children[2]->children.size(); ++i) {
            codegen_token(p, t->children[2]->children[i], output);
            output << ";" << std::endl;
        }
    }
    output << "goto whilejmp" << top << ";" << std::endl;
    output << "whilejmp" << block_end << ":" << std::endl;
    if (cont_at_end) {
        codegen_token(p, t->children[2]->children[t->children[2]->children.size() - 1], output);
    }

}
*/

void Codegen::codegen_return(Program *p, Token *t, std::ostringstream &output) {
    output << "((Actor__*)m__->recipient)->timeslice_remaining = timeslice__;" << std::endl;

    if (current_fun->token->type == Token_Type::ACTION_DEF) {
        bool has_actor = false;
        Scope *scope = current_fun->token->scope;
        while (scope != NULL) {
            if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTOR_DEF) || (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF))) {
                has_actor = true;
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " this_ptr__ = (";
                codegen_typesig(p, scope->owner->definition_number, output);
                output << ")m__->recipient;" << std::endl;
            }
            scope = scope->parent;
        }

        if (has_actor) {
            output << "this_ptr__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;" << std::endl;
        }
        else {
            output << "((Actor__*)m__->recipient)->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;" << std::endl;
        }
        output << "delete__(m__, exception__, " << p->global->local_types["object"] << "); exception__ = NULL;" << std::endl;
        output << "return FALSE;" << std::endl;
    }
    else {
        output << "return";
        if (t->children.size() > 1) {
            output << "(";
            codegen_token(p, t->children[1], output);
            output << ")";
        }
    }
}

void Codegen::codegen_array_call(Program *p, Token *t, std::ostringstream &output) {
    /*
    Var_Def *vd;
    if (t->children[0]->definition_number >= 0) {
        vd = p->vars[t->children[0]->definition_number];
    }
    else {
        throw Compiler_Exception("Internal compiler error with array call", t->start_pos);
    }

    Type_Def *td;
    if (vd->type_def_num >= 0) {
        td = p->types[vd->type_def_num];
    }
    else {
        throw Compiler_Exception("Internal compiler error with array call", t->start_pos);
    }
    */
    Type_Def *td = p->types[t->children[0]->type_def_num];
    if (td->container == Container_Type::ARRAY) {
        output << "INDEX_AT__(";
        codegen_token(p, t->children[0], output);
        output << ", ";
        codegen_token(p, t->children[1], output);
        output << ", ";
        codegen_typesig(p, t->type_def_num, output);
        output << ")";
    }
    else if (td->container == Container_Type::DICT) {
        output << "DICT_LOOKUP_AT__(";
        codegen_token(p, t->children[0], output);
        output << ", ";
        codegen_token(p, t->children[1], output);
        output << ", ";
        codegen_typesig(p, t->type_def_num, output);
        output << ")";
    }
}

void Codegen::codegen_new(Program *p, Token *t, std::ostringstream &output) {
    Type_Def *td = p->types[t->type_def_num];

    if (td->container == Container_Type::SCALAR) {
        output << "ctr__" << t->children[1]->definition_number << "(m__";
        if (t->children[1]->contents != ".") {
            if (t->children[1]->children.size() > 1) {
                output << ", ";
                codegen_token(p, t->children[1]->children[1], output);
            }
        }
        else {
            if (t->children[1]->children[1]->children.size() > 1) {
                output << ", ";
                codegen_token(p, t->children[1]->children[1]->children[1], output);
            }
        }
        output << ")";
    }
    else if (td->container == Container_Type::ARRAY) {
        output << "create_typeless_vector__(sizeof(";
        codegen_typesig(p, td->contained_type_def_nums[0], output);
        output << "), 0)";
    }
    else if (td->container == Container_Type::DICT) {
        output << "create_typeless_dictionary__(sizeof(";
        codegen_typesig(p, td->contained_type_def_nums[0], output);
        output << "))";
    }
}

void Codegen::codegen_spawn(Program *p, Token *t, std::ostringstream &output) {
    Type_Def *td = p->types[t->type_def_num];

    if (td->container == Container_Type::SCALAR) {
        output << "ctr__" << t->children[1]->definition_number << "(m__";
        if (t->children[1]->contents != ".") {
            if (t->children[1]->children.size() > 1) {
                output << ", ";
                codegen_token(p, t->children[1]->children[1], output);
            }
        }
        else {
            if (t->children[1]->children[1]->children.size() > 1) {
                output << ", ";
                codegen_token(p, t->children[1]->children[1]->children[1], output);
            }
        }
        output << ")";
    }
}

void Codegen::codegen_reference_feature(Program *p, Token *t, std::ostringstream &output) {
    if (t->children[0]->type_def_num == (signed)p->global->local_types["object"]) {
        output << "(";
        codegen_typesig(p, t->type_def_num, output);
        output << ")find_primary_feature__((Object_Feature__*)(";
        codegen_token(p, t->children[0], output);
        output << "), " << t->type_def_num << ")";
    }
    else {
        output << "(";
        codegen_typesig(p, t->type_def_num, output);
        output << ")find_feature__((Object_Feature__*)(";
        codegen_token(p, t->children[0], output);
        output << "), " << t->type_def_num << ")";
    }
}

void Codegen::codegen_continuation_site(Program *p, Token *t, std::ostringstream &output) {
    Function_Def *owner = this->current_fun;

    //++this->cont_id;
    int cont_id = t->definition_number;


    if ((t->children.size() > 0) && (t->children[0]->type != Token_Type::DELETION_SITE)) {
        output << "((Actor__*)m__->recipient)->timeslice_remaining = timeslice__;" << std::endl;
        //Deletion if it's there
        if (t->children.size() > 1) {
            codegen_token(p, t->children[1], output);
        }
        output << "case(" << cont_id << "):" << std::endl;
        codegen_token(p, t->children[0], output);
        output << ";" << std::endl;

        output << "timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;
        output << "if (timeslice__ == 0) {" << std::endl;

        output << "if (((Actor__*)m__->recipient)->exception != NULL) {" << std::endl;
        if (this->catch_jmp_name != "") {
            output << "timeslice__ = 2000;" << std::endl;
            output << "goto " << this->catch_jmp_name << ";" << std::endl;
        }
        else {
            if (owner->token->type == Token_Type::ACTION_DEF) {
                output << "print_s__(create_char_string_from_char_ptr__(\"Uncaught exception\\n\"));" << std::endl;
                output << "exit_i__(1); " << std::endl;
            }
            else {
                output << "return ";
                codegen_default_value(p, owner->return_type_def_num, output);
                output << ";" << std::endl;
            }
        }
        output << "}" << std::endl;

        if (t->definition_number != -1) {
            for (unsigned int i = 0; i < p->var_sites[owner->continuation_sites[t->definition_number]].size(); ++i) {
                output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &var__" << p->var_sites[owner->continuation_sites[t->definition_number]][i] << ");" << std::endl;
            }
        }
        output << "cont_id__ = " << cont_id << ";" << std::endl;
        output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
        if (owner->token->type == Token_Type::ACTION_DEF) {
            //output << "((Actor__*)m__->recipient)->actor_state = ACTOR_STATE_ACTIVE__;" << std::endl;
            output << "return TRUE; } " << std::endl;

        }
        else {
            output << "return ";
            codegen_default_value(p, owner->return_type_def_num, output);
            output << "; } " << std::endl;
        }
        output << "else if (--timeslice__ == 0) {" << std::endl;
        output << "((Actor__*)m__->recipient)->timeslice_remaining = timeslice__;" << std::endl;

        ++cont_id;

        if (t->definition_number != -1) {
            for (unsigned int i = 0; i < p->var_sites[owner->continuation_sites[t->definition_number]].size(); ++i) {
                output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &var__" << p->var_sites[owner->continuation_sites[t->definition_number]][i] << ");" << std::endl;
            }
        }
        output << "cont_id__ = " << cont_id << ";" << std::endl;
        output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
        if (owner->token->type == Token_Type::ACTION_DEF) {
            output << "((Actor__*)m__->recipient)->actor_state = ACTOR_STATE_ACTIVE__;" << std::endl;
            output << "return TRUE; } " << std::endl;

        }
        else {
            output << "return ";
            codegen_default_value(p, owner->return_type_def_num, output);
            output << "; } " << std::endl;
        }

        output << "case(" << cont_id << "):" << std::endl;
    }
    else {
        if ((t->children.size() > 0) && (t->children[0]->type == Token_Type::DELETION_SITE)) {
            codegen_token(p, t->children[0], output);
        }
        output << "if (--timeslice__ == 0) {" << std::endl;
        output << "((Actor__*)m__->recipient)->timeslice_remaining = timeslice__;" << std::endl;
        if (t->definition_number != -1) {
            for (unsigned int i = 0; i < p->var_sites[owner->continuation_sites[t->definition_number]].size(); ++i) {
                output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &var__" << p->var_sites[owner->continuation_sites[t->definition_number]][i] << ");" << std::endl;
            }
        }
        output << "cont_id__ = " << cont_id << ";" << std::endl;
        output << "push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
        if (owner->token->type == Token_Type::ACTION_DEF) {
            output << "((Actor__*)m__->recipient)->actor_state = ACTOR_STATE_ACTIVE__;" << std::endl;
            output << "return TRUE; } " << std::endl;
        }
        else {
            output << "return ";
            codegen_default_value(p, owner->return_type_def_num, output);
            output << "; } " << std::endl;
        }
        output << "case(" << cont_id << "):" << std::endl;
    }
}

void Codegen::codegen_deletion_site(Program *p, Token *t, std::ostringstream &output) {
    if (t->definition_number != -1) {
        std::vector<int> vars = p->var_sites[t->definition_number];

        for (unsigned int i = 0; i < vars.size(); ++i) {
            Var_Def *vd = p->vars[vars[i]];
            output << "delete__(m__, var__" << vars[i] << ", " << vd->type_def_num << ");" << std::endl;
            output << "var__" << vars[i] << " = NULL;" << std::endl;
        }
    }
}

void Codegen::codegen_token(Program *p, Token *t, std::ostringstream &output) {

    switch (t->type) {
        case (Token_Type::BLOCK) : codegen_block(p, t, output); break;
        case (Token_Type::FUN_CALL) : codegen_fun_call(p, t, output); break;
        case (Token_Type::METHOD_CALL) : codegen_method_call(p, t, output); break;
        case (Token_Type::VAR_DECL) :
        case (Token_Type::VAR_CALL) : codegen_id(p, t, output); break;
        case (Token_Type::ARRAY_CALL) : codegen_array_call(p, t, output); break;
        case (Token_Type::FLOAT) :
        case (Token_Type::INT) :
        case (Token_Type::QUOTED_STRING) : output << t->contents; break;
        //case (Token_Type::EXCEPTION) : output << "((Actor__*)m__->recipient)->exception"; break;
        case  (Token_Type::EXCEPTION) : output << "exception__"; break;
        case (Token_Type::BOOL) : if (t->contents == "true") { output << "TRUE"; } else { output << "FALSE"; }; break;
        case (Token_Type::SINGLE_QUOTED_STRING) : output << "'" << t->contents << "'"; break;
        case (Token_Type::QUOTED_STRING_CONST) : output << "create_char_string_from_char_ptr__(\"" << t->contents << "\")"; break;
        case (Token_Type::REFERENCE_ENUM) : output << t->definition_number; break;
        case (Token_Type::LIBRARY_EXTERN_BLOCK) :
            codegen_token(p, t->children[2], output);
        break;
        case (Token_Type::ATTRIBUTE_CALL) : {
            codegen_token(p, t->children[0], output);
            output << "->";
            codegen_token(p, t->children[1], output);
        }
        break;
        case (Token_Type::COPY) : {
            output << "(";
            codegen_typesig(p, t->children[0]->type_def_num, output);
            output << ")";
            output << "copy__(m__, ";
            codegen_token(p, t->children[0], output);
            output << ", " << t->children[0]->type_def_num << ")";
        }
        break;
        case (Token_Type::DELETE) : {
            output << "delete__(m__, ";
            codegen_token(p, t->children[0], output);
            output << ", " << t->children[0]->type_def_num << ");";
            codegen_token(p, t->children[0], output);
            output << " = NULL;";
        }
        break;
        case (Token_Type::DELETION_SITE) : codegen_deletion_site(p, t, output); break;
        case (Token_Type::THIS) : output << "this_ptr__"; break;
        case (Token_Type::CONCATENATE) : codegen_concatenate(p, t, output); break;
        case (Token_Type::CONCATENATE_ARRAY) : {
            Type_Def *container = p->types[t->children[0]->type_def_num];
            if (is_complex_type(p, container->contained_type_def_nums[0])) {
                output << "concatenate_new_complex_array__(m__, ";
                codegen_token(p, t->children[0], output);
                output << ", ";
                codegen_token(p, t->children[1], output);
                output << ", " << container->contained_type_def_nums[0] << ")";
            }
            else {
                output << "concatenate_new_typeless_vector__(";
                codegen_token(p, t->children[0], output);
                output << ", ";
                codegen_token(p, t->children[1], output);
                output << ")";
            }
        } break;
        case (Token_Type::SYMBOL) : codegen_symbol(p, t, output); break;
        case (Token_Type::RETURN_CALL) : codegen_return(p, t, output); break;
        case (Token_Type::BREAK) : if (break_jmp_name != "") { output << "goto " << break_jmp_name << ";" << std::endl;} break;
        case (Token_Type::IF_BLOCK) : codegen_if(p, t, output); break;
        case (Token_Type::TRY_BLOCK) : codegen_try(p, t, output); break;
        case (Token_Type::WHILE_BLOCK) : codegen_while(p, t, output); break;
        case (Token_Type::FOR_BLOCK) : codegen_for(p, t, output); break;
        case (Token_Type::NEW_ALLOC) : codegen_new(p, t, output); break;
        case (Token_Type::SPAWN_ACTOR) : codegen_spawn(p, t, output); break;
        case (Token_Type::ACTION_CALL) : codegen_action_call(p, t, output); break;
        case (Token_Type::REFERENCE_FEATURE) : codegen_reference_feature(p, t, output); break;
        case (Token_Type::CONTINUATION_SITE) : codegen_continuation_site(p, t, output); break;
        default: break;
    }
}

void Codegen::codegen_constructor_internal_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        if (p->funs[i]->is_constructor) {
            //If it's internal, it's an implied constructor and doesn't need an inner call
            if (p->funs[i]->is_internal) {
                //Find the constructed type
                Scope *scope = p->funs[i]->token->scope;
                while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ctr__" << i << "(Message__ *m__);" << std::endl;
            }
        }
    }
}

void Codegen::codegen_constructor_not_internal_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        if (p->funs[i]->is_used == false) {
            continue;
        }

        if (p->funs[i]->is_constructor) {
            //If it's internal, it's an implied constructor and doesn't need an inner call
            if (p->funs[i]->is_internal == false) {
                Function_Def *fd = p->funs[i];

                Scope *scope = p->funs[i]->token->scope;
                while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ctr__" << i << "(";

                unsigned int argsize = fd->arg_def_nums.size();
                if (argsize == 0) {
                    output << "Message__ *m__";
                }
                else {
                    output << "Message__ *m__,";
                }
                for (unsigned int j = 0; j < argsize; ++j) {
                    if (j > 0) {
                        output << ", ";
                    }
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j];
                }
                output << ");" << std::endl;
            }
        }
    }
}

void Codegen::codegen_action_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        if (p->funs[i]->is_used == false) {
            continue;
        }

        if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name == "")) {
            Function_Def *fd = p->funs[i];
            if (fd->token->type == Token_Type::ACTION_DEF) {
                output << "BOOL fun__" << i << "(Message__ *m__);" << std::endl;
            }
        }
    }
}

void Codegen::codegen_fun_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        if (p->funs[i]->is_used == false) {
            continue;
        }

        if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name == "")) {
            Function_Def *fd = p->funs[i];
            if (fd->token->type == Token_Type::FUN_DEF) {
                codegen_typesig(p, fd->return_type_def_num, output);
                output << " fun__" << i << "(";

                unsigned int argsize = fd->arg_def_nums.size();
                if (argsize == 0) {
                    output << "Message__ *m__";
                }
                else {
                    output << "Message__ *m__,";
                }

                Scope *scope = fd->token->scope;
                while ((scope != NULL) && (scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }

                if ((scope != NULL) /*&& (scope->owner->type == Token_Type::FEATURE_DEF)*/) {
                    if (argsize == 0) {
                        output << ", ";
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__";
                    }
                    else {
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__,";
                    }
                }


                for (unsigned int j = 0; j < argsize; ++j) {
                    if (j > 0) {
                        output << ", ";
                    }
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j];
                }
                output << ");" << std::endl;
            }
        }
        else if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name != "")) {
            Function_Def *fd = p->funs[i];
#if defined (__SVR4) && defined (__sun)
            output << "extern \"C\" ";
#else
            output << "extern ";
#endif
            codegen_typesig(p, fd->return_type_def_num, output);
            output << " "  << fd->external_name << "(";

            unsigned int argsize = fd->arg_def_nums.size();
            /*
            if (argsize == 0) {
                output << "Message__ *m__";
            }
            else {
                output << "Message__ *m__,";
            }
            */
            for (unsigned int j = 0; j < argsize; ++j) {
                if (j > 0) {
                    output << ", ";
                }
                codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                output << " var__" << fd->arg_def_nums[j];
            }
            output << ");" << std::endl;
        }
    }
}

void Codegen::codegen_constructor_internal_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        this->current_fun = p->funs[i];
        if (p->funs[i]->is_constructor) {
            //If it's internal, it's an implied constructor and doesn't need an inner call
            if (p->funs[i]->is_internal) {
                //Find the constructed type
                Scope *scope = p->funs[i]->token->scope;
                while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ctr__" << i << "(Message__ *m__)" << std::endl <<"{" << std::endl;
                if (scope->owner->type == Token_Type::ACTOR_DEF) {
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")create_actor__(sizeof(";
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                            end = scope->local_vars.end(); iter != end; ++iter) {

                        output << "ret_val__->var__" << iter->second << " = ";
                        codegen_default_value(p, p->vars[iter->second]->type_def_num, output);
                        output << ";" << std::endl;
                    }
                    output << "add_actor_to_sched__((Scheduler__*)m__->sched, (Actor__*)ret_val__);" << std::endl;
                }
                else if (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF) {
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")create_actor__(sizeof(";
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                            end = scope->local_vars.end(); iter != end; ++iter) {

                        output << "ret_val__->var__" << iter->second << " = ";
                        codegen_default_value(p, p->vars[iter->second]->type_def_num, output);
                        output << ";" << std::endl;
                    }

                    output << "Message__ *msg__ = get_msg_from_cache__(m__->sched);" << std::endl;
                    output << "msg__->message_type = MESSAGE_TYPE_CREATE_ISOLATED_ACTOR;" << std::endl;
                    output << "msg__->args[0].VoidPtr = ret_val__;" << std::endl;
                    output << "msg__->next = NULL;" << std::endl;
                    output << "send_messages__(((Scheduler__*)m__->sched)->outgoing_channel, msg__);" << std::endl;
                }
                else if (scope->owner->type == Token_Type::FEATURE_DEF) {
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    codegen_typesig(p, scope->owner->definition_number, output);
#ifdef USE_MEMBLOCK_CACHE
                    output << ")get_memblock_from_cache__(m__->sched,sizeof(";
#else
                    output << ")malloc(sizeof(";
#endif
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    output << "initialize_feature__(&ret_val__->base__, " << scope->owner->definition_number << ");" << std::endl;
                    for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                            end = scope->local_vars.end(); iter != end; ++iter) {

                        output << "ret_val__->var__" << iter->second << " = ";
                        codegen_default_value(p, p->vars[iter->second]->type_def_num, output);
                        output << ";" << std::endl;
                    }
                }
                output << "return ret_val__;" << std::endl << "}" << std::endl;
            }
        }
    }
}

void Codegen::codegen_constructor_not_internal_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        this->current_fun = p->funs[i];
        if (p->funs[i]->is_used == false) {
            continue;
        }

        if (p->funs[i]->is_constructor) {
            //If it's not internal, it needs an inner call
            if (p->funs[i]->is_internal == false) {
                //Find the constructed type
                Function_Def *fd = p->funs[i];
                Scope *scope = p->funs[i]->token->scope;
                while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ctr__" << i << "(";

                unsigned int argsize = fd->arg_def_nums.size();
                if (argsize == 0) {
                    output << "Message__ *m__";
                }
                else {
                    output << "Message__ *m__,";
                }
                for (unsigned int j = 0; j < argsize; ++j) {
                    if (j > 0) {
                        output << ", ";
                    }
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j];
                }
                output << ")" << std::endl << "{" << std::endl;

                output << "unsigned int cont_id__ = 0;" << std::endl;
                output << "unsigned int timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;
                codegen_typesig(p, scope->owner->definition_number, output);
                output << " ret_val__;" << std::endl;


                output << "if (((Actor__*)m__->recipient)->continuation_stack->current_size > 0) {" << std::endl;
                output << "  cont_id__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
                output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                output << "  ret_val__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, ";
                codegen_typesig(p, scope->owner->definition_number, output);
                output << ");" << std::endl;
                output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                output << "}" << std::endl;

                output << "switch (cont_id__) {" << std::endl;
                output << " case(0) : " << std::endl;
                if (scope->owner->type == Token_Type::ACTOR_DEF) {
                    //codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")create_actor__(sizeof(";
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                            end = scope->local_vars.end(); iter != end; ++iter) {

                        output << "ret_val__->var__" << iter->second << " = ";
                        codegen_default_value(p, p->vars[iter->second]->type_def_num, output);
                        output << ";" << std::endl;
                    }
                    output << " case(1) : " << std::endl;
                    output << "fun__" << i << "(m__, ret_val__";
                    if (argsize > 0) {
                        output << ", ";
                    }
                    for (unsigned int j = 0; j < argsize; ++j) {
                        if (j > 0) {
                            output << ", ";
                        }
                        output << " var__" << fd->arg_def_nums[j];
                    }
                    output << ");" << std::endl;
                    output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &ret_val__);" << std::endl;
                    output << "  cont_id__ = 1;" <<std::endl;
                    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                    output << "  return NULL;" << std::endl;
                    output << "}" << std::endl;
                    output << "add_actor_to_sched__((Scheduler__*)m__->sched, (Actor__*)ret_val__);" << std::endl;
                }
                else if (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF) {
                    //codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << ")create_actor__(sizeof(";
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
                    for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                            end = scope->local_vars.end(); iter != end; ++iter) {

                        output << "ret_val__->var__" << iter->second << " = ";
                        codegen_default_value(p, p->vars[iter->second]->type_def_num, output);
                        output << ";" << std::endl;
                    }
                    output << " case(1) : " << std::endl;
                    output << "fun__" << i << "(m__, ret_val__";
                    if (argsize > 0) {
                        output << ", ";
                    }
                    for (unsigned int j = 0; j < argsize; ++j) {
                        if (j > 0) {
                            output << ", ";
                        }
                        output << " var__" << fd->arg_def_nums[j];
                    }
                    output << ");" << std::endl;
                    output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &ret_val__);" << std::endl;
                    output << "  cont_id__ = 1;" <<std::endl;
                    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                    output << "  return NULL;" << std::endl;
                    output << "}" << std::endl;
                    output << "Message__ *msg__ = get_msg_from_cache__(m__->sched);" << std::endl;
                    output << "msg__->message_type = MESSAGE_TYPE_CREATE_ISOLATED_ACTOR;" << std::endl;
                    output << "msg__->args[0].VoidPtr = ret_val__;" << std::endl;
                    output << "msg__->next = NULL;" << std::endl;
                    output << "send_messages__(((Scheduler__*)m__->sched)->outgoing_channel, msg__);" << std::endl;
                }
                else if (scope->owner->type == Token_Type::FEATURE_DEF) {
                    //codegen_typesig(p, scope->owner->definition_number, output);
                    output << " ret_val__ = (";
                    codegen_typesig(p, scope->owner->definition_number, output);
#ifdef USE_MEMBLOCK_CACHE
                    output << ")get_memblock_from_cache__(m__->sched, sizeof(";
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
#else
                    output << ")malloc(sizeof(";
                    codegen_typesig_no_tail(p, scope->owner->definition_number, output);
                    output << "));" << std::endl;
#endif
                    output << "initialize_feature__(&ret_val__->base__, " << scope->owner->definition_number << ");" << std::endl;
                    for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                            end = scope->local_vars.end(); iter != end; ++iter) {

                        output << "ret_val__->var__" << iter->second << " = ";
                        codegen_default_value(p, p->vars[iter->second]->type_def_num, output);
                        output << ";" << std::endl;
                    }

                    output << " case(1) : " << std::endl;
                    output << "fun__" << i << "(m__, ret_val__";
                    if (argsize > 0) {
                        output << ", ";
                    }
                    for (unsigned int j = 0; j < argsize; ++j) {
                        if (j > 0) {
                            output << ", ";
                        }
                        output << " var__" << fd->arg_def_nums[j];
                    }
                    output << ");" << std::endl;
                    output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &ret_val__);" << std::endl;
                    output << "  cont_id__ = 1;" <<std::endl;
                    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                    output << "  return NULL;" << std::endl;
                    output << "}" << std::endl;
                }
                output << "}" << std::endl;
                output << "return ret_val__;" << std::endl << "}" << std::endl;
            }
        }
    }
}

void Codegen::codegen_action_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        this->current_fun = p->funs[i];
        if (p->funs[i]->is_used == false) {
            continue;
        }
        if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name == "")) {
            Function_Def *fd = p->funs[i];

            if (fd->token->type == Token_Type::ACTION_DEF) {
                output << "BOOL fun__" << i << "(Message__ *m__)" << std::endl;
                output << "{" << std::endl;
                output << "void *exception__ = ((Actor__*)m__->recipient)->exception;" << std::endl;
                output << "unsigned int cont_id__ = 0;" << std::endl;
                output << "int timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;

                if (fd->token->children.size() > 2) {
                    Scope *scope_var = fd->token->children[2]->scope;
                    if (scope_var != NULL) {
                        for (std::map<std::string, unsigned int>::iterator iter = scope_var->local_vars.begin(),
                                end = scope_var->local_vars.end(); iter != end; ++iter)
                        {
                            Var_Def *vd = p->vars[iter->second];
                            codegen_typesig(p, vd->type_def_num, output);
                            output << " var__" << iter->second << ";" << std::endl;
                        }
                    }
                }

                unsigned int argsize = fd->arg_def_nums.size();
                for (unsigned int j = 0; j < argsize; ++j) {
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j] << ";" << std::endl;;
                }

                //find "this" ptr
                bool has_actor = false;
                Scope *scope = fd->token->scope;
                while (scope != NULL) {
                    if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTOR_DEF) || (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF))) {
                        has_actor = true;
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__ = (";
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << ")m__->recipient;" << std::endl;
                    }
                    scope = scope->parent;
                }

                if (fd->continuation_sites.size() > 1) {

                    output << "if (((Actor__*)m__->recipient)->continuation_stack->current_size > 0) {" << std::endl;
                    output << "  cont_id__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
                    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                    output << "  switch(cont_id__) {" << std::endl;

                    for (unsigned int k = 0; k < fd->continuation_sites.size(); ++k) {
                        if ((fd->continuation_sites[k] != -1) && (p->var_sites[fd->continuation_sites[k]].size() > 0)) {
                            output << "  case(" << k << ") : " << std::endl;

                            for (int j = p->var_sites[fd->continuation_sites[k]].size() - 1; j >= 0; --j) {

                                Var_Def *vd = p->vars[p->var_sites[fd->continuation_sites[k]][j]];
                                output << "  var__" << p->var_sites[fd->continuation_sites[k]][j] << " = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack,";
                                output << "    ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, ";
                                codegen_typesig(p, vd->type_def_num, output);
                                output << ");" << std::endl;
                                output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                            }
                            for (std::map<std::string, unsigned int>::iterator v_iter = fd->token->children[2]->scope->local_vars.begin(), v_end = fd->token->children[2]->scope->local_vars.end();
                                v_iter != v_end; ++v_iter) {

                                bool found = false;
                                for (int j = p->var_sites[fd->continuation_sites[k]].size() - 1; j >= 0; --j) {
                                    if (p->var_sites[fd->continuation_sites[k]][j] == (signed)v_iter->second) {
                                        found = true;
                                    }
                                }

                                Var_Def *vd = p->vars[v_iter->second];
                                if (!found) {
                                    output << "  var__" << v_iter->second << " = ";
                                    codegen_default_value(p, vd->type_def_num, output);
                                    output << ";" << std::endl;
                                }
                            }
                            output << "  break;" << std::endl;
                        }
                        else {
                            output << "  case(" << k << ") : " << std::endl;
                            for (std::map<std::string, unsigned int>::iterator v_iter = fd->token->children[2]->scope->local_vars.begin(), v_end = fd->token->children[2]->scope->local_vars.end();
                                v_iter != v_end; ++v_iter) {

                                Var_Def *vd = p->vars[v_iter->second];
                                output << "  var__" << v_iter->second << " = ";
                                codegen_default_value(p, vd->type_def_num, output);
                                output << ";" << std::endl;
                            }
                            output << "  break;" << std::endl;
                        }
                    }

                    output << "} }" << std::endl;
                    /*
                    output << "else {" << std::endl;
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {
                            Var_Def *vd = p->vars[iter->second];
                            output << "var__" << iter->second << " = ";
                            codegen_default_value(p, vd->type_def_num, output);
                            output << ";" << std::endl;
                        }
                        for (unsigned int j = 0; j < argsize; ++j) {
                            output << " var__" << fd->arg_def_nums[j] << " = ";
                            output << "(";
                            codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ")m__->args[" << j << "].";
                            codegen_tu_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ";" << std::endl;
                        }

                    }
                    output << "}" << std::endl;
                    */

                    //this->cont_id = 0;
                    output << "switch(cont_id__) {" << std::endl;
                    output << "case(0):" << std::endl;
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {

                            if (is_complex_var(p, iter->second)) {
                                Var_Def *vd = p->vars[iter->second];
                                output << "var__" << iter->second << " = ";
                                codegen_default_value(p, vd->type_def_num, output);
                                output << ";" << std::endl;
                            }
                        }

                        for (unsigned int j = 0; j < argsize; ++j) {
                            output << " var__" << fd->arg_def_nums[j] << " = ";
                            output << "(";
                            codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ")m__->args[" << j << "].";
                            codegen_tu_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ";" << std::endl;
                        }

                    }
                    codegen_block(p, fd->token->children[2], output);
                    output << "break; }" << std::endl;
                }
                else {
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {

                            if (is_complex_var(p, iter->second)) {
                                Var_Def *vd = p->vars[iter->second];
                                output << "var__" << iter->second << " = ";
                                codegen_default_value(p, vd->type_def_num, output);
                                output << ";" << std::endl;
                            }
                        }
                        for (unsigned int j = 0; j < argsize; ++j) {
                            output << " var__" << fd->arg_def_nums[j] << " = ";
                            output << "(";
                            codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ")m__->args[" << j << "].";
                            codegen_tu_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                            output << ";" << std::endl;
                        }
                        codegen_block(p, fd->token->children[2], output);
                    }
                }

                if (has_actor) {
                    output << "this_ptr__->base__.actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;" << std::endl;
                }
                else {
                    output << "((Actor__*)m__->recipient)->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;" << std::endl;
                }
                output << "((Actor__*)m__->recipient)->timeslice_remaining = timeslice__;" << std::endl;
                output << "return FALSE;" << std::endl << "}" << std::endl;

            }
        }
    }
}

void Codegen::codegen_fun_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->funs.size(); ++i) {
        this->current_fun = p->funs[i];
        if (p->funs[i]->is_used == false) {
            continue;
        }
        if ((p->funs[i]->is_internal == false) && (p->funs[i]->external_name == "")) {

            Function_Def *fd = p->funs[i];
            if (fd->token->type == Token_Type::FUN_DEF) {
                codegen_typesig(p, fd->return_type_def_num, output);
                output << " fun__" << i << "(";

                /*
                if (p->funs[i]->is_constructor) {
                    Scope *scope = p->funs[i]->token->scope;
                    while ((scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                            (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                        scope = scope->parent;
                    }

                    //output << "type__" << scope->owner->definition_number << "*";
                    codegen_typesig(p, scope->owner->definition_number, output);
                    output << " this_ptr__, ";
                }
                */

                unsigned int argsize = fd->arg_def_nums.size();
                if (argsize == 0) {
                    output << "Message__ *m__";
                }
                else {
                    output << "Message__ *m__,";
                }

                Scope *scope = fd->token->scope;
                while ((scope != NULL) && (scope->owner->type != Token_Type::ACTOR_DEF) && (scope->owner->type != Token_Type::FEATURE_DEF) &&
                        (scope->owner->type != Token_Type::ISOLATED_ACTOR_DEF)) {
                    scope = scope->parent;
                }

                if ((scope != NULL) /*&& (scope->owner->type == Token_Type::FEATURE_DEF)*/) {
                    if (argsize == 0) {
                        output << ", ";
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__";
                    }
                    else {
                        codegen_typesig(p, scope->owner->definition_number, output);
                        output << " this_ptr__,";
                    }
                }


                for (unsigned int j = 0; j < argsize; ++j) {
                    if (j > 0) {
                        output << ", ";
                    }
                    codegen_typesig(p, p->vars[fd->arg_def_nums[j]]->type_def_num, output);
                    output << " var__" << fd->arg_def_nums[j];
                }
                output << ")" << std::endl;
                output << "{" << std::endl;
                output << "void *exception__ = ((Actor__*)m__->recipient)->exception;" << std::endl;
                output << "int timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;
                output << "unsigned int cont_id__ = 0;" << std::endl;

                if (fd->token->children.size() > 2) {
                    for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                            end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter)
                    {
                        Var_Def *vd = p->vars[iter->second];
                        codegen_typesig(p, vd->type_def_num, output);
                        output << " var__" << iter->second << ";" << std::endl;
                    }
                }

                if (fd->continuation_sites.size() > 1) {
                    output << "if (((Actor__*)m__->recipient)->continuation_stack->current_size > 0) {" << std::endl;
                    output << "  cont_id__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
                    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                    output << "  switch(cont_id__) {" << std::endl;

                    for (unsigned int k = 0; k < fd->continuation_sites.size(); ++k) {
                        if ((fd->continuation_sites[k] != -1) && (p->var_sites[fd->continuation_sites[k]].size() > 0)) {
                                output << "  case(" << k << ") : " << std::endl;

                                for (int j = p->var_sites[fd->continuation_sites[k]].size() - 1; j >= 0; --j) {
                                    Var_Def *vd = p->vars[p->var_sites[fd->continuation_sites[k]][j]];
                                    output << "  var__" << p->var_sites[fd->continuation_sites[k]][j] << " = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack,";
                                    output << "    ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, ";
                                    codegen_typesig(p, vd->type_def_num, output);
                                    output << ");" << std::endl;
                                    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
                                }
                                for (std::map<std::string, unsigned int>::iterator v_iter = fd->token->children[2]->scope->local_vars.begin(), v_end = fd->token->children[2]->scope->local_vars.end();
                                    v_iter != v_end; ++v_iter) {

                                    bool found = false;
                                    for (int j = p->var_sites[fd->continuation_sites[k]].size() - 1; j >= 0; --j) {
                                        if (p->var_sites[fd->continuation_sites[k]][j] == (signed)v_iter->second) {
                                            found = true;
                                        }
                                    }

                                    Var_Def *vd = p->vars[v_iter->second];
                                    if (!found) {
                                        output << "  var__" << v_iter->second << " = ";
                                        codegen_default_value(p, vd->type_def_num, output);
                                        output << ";" << std::endl;
                                    }
                                }

                                output << "  break;" << std::endl;
                        }
                        else {
                            output << "  case(" << k << ") : " << std::endl;
                            for (std::map<std::string, unsigned int>::iterator v_iter = fd->token->children[2]->scope->local_vars.begin(), v_end = fd->token->children[2]->scope->local_vars.end();
                                v_iter != v_end; ++v_iter) {

                                Var_Def *vd = p->vars[v_iter->second];
                                output << "  var__" << v_iter->second << " = ";
                                codegen_default_value(p, vd->type_def_num, output);
                                output << ";" << std::endl;
                            }
                            output << "  break;" << std::endl;
                        }
                    }

                    output << "} }" << std::endl;
                    /*
                    output << "else {" << std::endl;
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {
                            Var_Def *vd = p->vars[iter->second];
                            output << "var__" << iter->second << " = ";
                            codegen_default_value(p, vd->type_def_num, output);
                            output << ";" << std::endl;
                        }
                    }
                    output << "}" << std::endl;
                    */
                    //this->cont_id = 0;
                    output << "switch(cont_id__) {" << std::endl;
                    output << "case(0):" << std::endl;
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {

                            if (is_complex_var(p, iter->second)) {
                                Var_Def *vd = p->vars[iter->second];
                                output << "var__" << iter->second << " = ";
                                codegen_default_value(p, vd->type_def_num, output);
                                output << ";" << std::endl;
                            }
                        }
                    }


                    codegen_block(p, fd->token->children[2], output);
                    output << "}" << std::endl;
                }
                else {
                    if (fd->token->children.size() > 2) {
                        for (std::map<std::string, unsigned int>::iterator iter = fd->token->children[2]->scope->local_vars.begin(),
                                end = fd->token->children[2]->scope->local_vars.end(); iter != end; ++iter) {
                            Var_Def *vd = p->vars[iter->second];
                            output << "var__" << iter->second << " = ";
                            codegen_default_value(p, vd->type_def_num, output);
                            output << ";" << std::endl;
                        }
                        codegen_block(p, fd->token->children[2], output);
                    }
                }
                //output << "((Actor__*)m__->recipient)->timeslice_remaining = --timeslice__;" << std::endl << "}" << std::endl;
                output << "((Actor__*)m__->recipient)->timeslice_remaining = timeslice__;" << std::endl << "}" << std::endl;
            }
        }
    }
}

void Codegen::codegen_copy_predecl(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    output << "void *copy__(Message__ *m__, void *v__, unsigned int t__);" << std::endl;
}

void Codegen::codegen_delete_predecl(Program *p, std::ostringstream &output) {
    output << "void delete__(Message__ *m__, void *v__, unsigned int t__);" << std::endl;
}

void Codegen::codegen_copy_decl(Program *p, unsigned int type_def_num, std::ostringstream &output) {
    int obj_id = p->global->local_types["object"];
    int string_id = p->global->local_types["string"];
    int current_cont = 1;

    output << "void *copy__(Message__ *m__, void *v__, unsigned int t__)" << std::endl << "{" << std::endl;
    output << "  unsigned int i,j;" << std::endl;
    output << "  void *ret_val__;" << std::endl;
    output << "  void *tmp_val__;" << std::endl;
    output << "unsigned int cont_id__ = 0;" << std::endl;
    output << "if (((Actor__*)m__->recipient)->continuation_stack->current_size > 0) {" << std::endl;
    output << "  cont_id__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  i = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  j = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  t__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  ret_val__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, void*);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  v__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, void*);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  tmp_val__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, void*);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "}" << std::endl;
    output << "if (v__ == NULL) return NULL;" << std::endl;

    output << " switch(cont_id__) {" << std::endl;
    output << " case(0):" << std::endl;

    //output << "switch(t__)" << "{" << std::endl;
    //output << "  case(" << string_id << "): {" << std::endl << "  ";
    output << "if (t__ == " << string_id << ") {" << std::endl;
    output << "  ret_val__ = create_char_string__(0);" << std::endl;
    output << "  concatenate_char_string__((Typeless_Vector__*)ret_val__, (";
    codegen_typesig(p, string_id, output);
    output << ")v__);" << std::endl;
    output << "  return ret_val__;" << std::endl;
    output << "}" << std::endl;
    //output << "  }; break;" << std::endl;
    for (unsigned int i = obj_id; i < p->types.size(); ++i) {
        output << "else if (t__ == " << i << ") {" << std::endl << "  ";
        //output << "  case(" << i << "): {" << std::endl;
        Type_Def *td = p->types[i];

        if (i == (unsigned)obj_id) {
            //output << "  Object_Feature__ *ret_val__ = (Object_Feature__*)copy__(m__, v__, ((Object_Feature__*)v__)->feature_id);" << std::endl;
            output << "  ret_val__ = (Object_Feature__*)copy__(m__, v__, ((Object_Feature__*)v__)->feature_id);" << std::endl;
            output << "  return ret_val__;" << std::endl;
        }
        else if (td->container == Container_Type::ARRAY) {
            //output << "  Typeless_Vector__ *ret_val__ = create_typeless_vector__(((Typeless_Vector__ *)v__)->elem_size, ((Typeless_Vector__ *)v__)->current_size);" << std::endl;
            output << "  ret_val__ = create_typeless_vector__(((Typeless_Vector__ *)v__)->elem_size, ((Typeless_Vector__ *)v__)->current_size);" << std::endl;
            output << "  for (i = 0; i < ((Typeless_Vector__ *)v__)->current_size; ++i)" << std::endl << "  {" << std::endl;
            if ((td->contained_type_def_nums[0] >= (signed)obj_id) || (td->contained_type_def_nums[0] == (signed)string_id)) {
                output << "case(" << current_cont << "):" << std::endl;
                output << "    INDEX_AT__(((Typeless_Vector__ *)ret_val__), i, ";
                codegen_typesig(p, td->contained_type_def_nums[0], output);
                output << ") = ";
                output << "(";
                codegen_typesig(p, td->contained_type_def_nums[0], output);
                output << ")copy__(m__, INDEX_AT__(((Typeless_Vector__ *)v__), i, ";
                codegen_typesig(p, td->contained_type_def_nums[0], output);
                output << "), " << td->contained_type_def_nums[0] << ");" << std::endl;
                output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp_val__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &ret_val__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                output << "  return NULL;" << std::endl;
                output << "}" << std::endl;
                ++current_cont;
            }
            else {
                output << "    INDEX_AT__(((Typeless_Vector__ *)ret_val__), i, ";
                codegen_typesig(p, td->contained_type_def_nums[0], output);
                output << ") = ";
                output << "INDEX_AT__(((Typeless_Vector__ *)v__), i, ";
                codegen_typesig(p, td->contained_type_def_nums[0], output);
                output << ");" << std::endl;
            }
            output << "  }" << std::endl;
            output << "  ((Typeless_Vector__ *)ret_val__)->current_size = ((Typeless_Vector__ *)v__)->current_size;" << std::endl;
            output << "  return ret_val__;" << std::endl;
        }
        else if (td->container == Container_Type::DICT) {
            //output << "  unsigned int i, j;" << std::endl;
            //output << "  Typeless_Dictionary__ *ret_val__ = create_typeless_dictionary__(((Typeless_Dictionary__ *)v__)->elem_size);" << std::endl;
            output << "  ret_val__ = create_typeless_dictionary__(((Typeless_Dictionary__ *)v__)->elem_size);" << std::endl;
            output << "  for (i = 0; i < DEFAULT_TYPELESS_DICTIONARY_SIZE__; ++i)" << std::endl << "  {" << std::endl;
            output << "    if (((Typeless_Dictionary__*)v__)->contents[i] == NULL) {" << std::endl;
            output << "      ((Typeless_Dictionary__ *)ret_val__)->contents[i] = NULL;" << std::endl;
            output << "    }" << std::endl;
            output << "    else {" << std::endl;
            output << "      ((Typeless_Dictionary__ *)ret_val__)->contents[i] = create_typeless_vector__(sizeof(Dict_Unit__), 0);" << std::endl;
            output << "      for (j = 0; j < ((Typeless_Dictionary__*)v__)->contents[i]->current_size; ++j) {" << std::endl;
            //if ((td->contained_type_def_nums[0] >= (signed)obj_id) || (td->contained_type_def_nums[0] == (signed)string_id)) {
            if (is_complex_type(p, td->contained_type_def_nums[0])) {
                output << "case(" << current_cont << "):" << std::endl;
                output << "        tmp_val__ = ";
                output << "(";
                codegen_typesig(p, td->contained_type_def_nums[0], output);
                output << ")copy__(m__, INDEX_AT__(((Typeless_Dictionary__*)v__)->contents[i], j, Dict_Unit__).data.VoidPtr, " << td->contained_type_def_nums[0] << ");" << std::endl;
                output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp_val__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &ret_val__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                output << "  return NULL;" << std::endl;
                output << "}" << std::endl;
                ++current_cont;
            }
            output << "        Dict_Unit__ du;" << std::endl;
            output << "        du.key = (Typeless_Vector__*)copy__(m__, INDEX_AT__(((Typeless_Dictionary__*)v__)->contents[i], j, Dict_Unit__).key, " << string_id << ");" << std::endl;
            output << "        du.data.";
            codegen_tu_typesig(p, td->contained_type_def_nums[0], output);
            output << "  = ";
            if (is_complex_type(p, td->contained_type_def_nums[0])) {
                output << "tmp_val__;" << std::endl;
            }
            else {
                output << "INDEX_AT__(((Typeless_Dictionary__*)v__)->contents[i], j, Dict_Unit__).data.";
                codegen_tu_typesig(p, td->contained_type_def_nums[0], output);
                //codegen_typesig(p, td->contained_type_def_nums[0], output);
                output << ";" << std::endl;
            }
            output << "       push_onto_typeless_vector__(((Typeless_Dictionary__ *)ret_val__)->contents[i], &du);" << std::endl;
            output << "     }" << std::endl;
            output << "   }" << std::endl;
            output << "  }" << std::endl;
            output << "  return ret_val__;" << std::endl;
        }
        else if ((td->token->type == Token_Type::ACTOR_DEF) || (td->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
            output << "  return v__;" << std::endl;
        }
        else if (td->token->type == Token_Type::FEATURE_DEF) {

#ifdef USE_MEMBLOCK_CACHE
            //output << "  Object_Feature__ *ret_val__ = (Object_Feature__ *)get_memblock_from_cache__(m__->sched, sizeof(";
            output << "  ret_val__ = (Object_Feature__ *)get_memblock_from_cache__(m__->sched, sizeof(";
            codegen_typesig_no_tail(p, i, output);
            output << "));" << std::endl;
            output << "  initialize_feature__((Object_Feature__*)ret_val__, " << i << ");" << std::endl;
#else
            //output << "  Object_Feature__ *ret_val__ = (Object_Feature__ *)create_feature__(sizeof(";
            output << "  ret_val__ = (Object_Feature__ *)create_feature__(sizeof(";
            codegen_typesig_no_tail(p, i, output);
            output << "), " << i << ");" << std::endl;
#endif
            std::ostringstream copy_fn;
            copy_fn << "copy__" << i;

            std::map<std::string, unsigned int>::iterator cpy_meth = td->token->scope->local_funs.find(copy_fn.str());
            if ((cpy_meth != td->token->scope->local_funs.end()) && (p->funs[cpy_meth->second]->is_internal == false)) {
                //Function_Def *delete_fd = p->funs[td->token->scope->local_funs["delete"]];
                output << "case(" << current_cont << "):" << std::endl;
                output << "  fun__" << td->token->scope->local_funs[copy_fn.str()] << "(m__, (";
                codegen_typesig(p, i, output);
                output << ")ret_val__, (";
                codegen_typesig(p, i, output);
                output << ")v__);" << std::endl;
                output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp_val__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &ret_val__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                output << "  return NULL;" << std::endl;
                output << "}" << std::endl;
                ++current_cont;
            }
            else {
                for (std::map<std::string, unsigned int>::iterator iter = td->token->scope->local_vars.begin(),
                    end = td->token->scope->local_vars.end(); iter != end; ++iter) {
                    Var_Def *vd = p->vars[iter->second];
                    //if ((vd->type_def_num >= obj_id) || (vd->type_def_num == string_id)) {
                    if (is_complex_var(p, iter->second)) {
                        output << "case(" << current_cont << "):" << std::endl;
                        output << "  ((";
                        codegen_typesig(p, i, output);
                        output << ")ret_val__)->var__" << iter->second << " = (";
                        codegen_typesig(p, vd->type_def_num, output);
                        output << ")copy__(m__, ((";
                        codegen_typesig(p, i, output);
                        output << ")v__)->var__"
                            << iter->second << ", " << vd->type_def_num << ");" << std::endl;
                        output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp_val__);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &ret_val__);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                        output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                        output << "  return NULL;" << std::endl;
                        output << "}" << std::endl;
                        ++current_cont;
                    }
                    else {
                        output << "  ((";
                        codegen_typesig(p, i, output);
                        output << ")ret_val__)->var__" << iter->second << " = ((";
                        codegen_typesig(p, i, output);
                        output << ")v__)->var__" << iter->second << ";" << std::endl;
                    }
                }
            }
            output << "  if (((Object_Feature__*)v__)->next != NULL) {" << std::endl;
            output << "case(" << current_cont << "):" << std::endl;
            output << "    ((Object_Feature__*)ret_val__)->next = (Object_Feature__*)copy__(m__, ((Object_Feature__*)v__)->next, ((Object_Feature__*)(((Object_Feature__*)v__)->next))->feature_id);" << std::endl;
            output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp_val__);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &ret_val__);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
            output << "  cont_id__ = " << current_cont << ";" <<std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
            output << "  return NULL;" << std::endl;
            output << "}" << std::endl;
            ++current_cont;
            output << "  }" << std::endl;
            output << "  else {" << std::endl;
            output << "    ((Object_Feature__*)ret_val__)->next = NULL;" << std::endl;
            output << "  }" << std::endl;
            output << "  return ret_val__;" << std::endl;
        }
        //output << "  }; break;" << std::endl;
        output << "}" << std::endl;
    }

    output << "}" << std::endl;

    output << "}" << std::endl;
}

void Codegen::codegen_delete_decl(Program *p, std::ostringstream &output) {
    int obj_id = p->global->local_types["object"];
    int string_id = p->global->local_types["string"];
    int current_cont = 1;

    output << "void delete__(Message__ *m__, void *v__, unsigned int t__)" << std::endl << "{" << std::endl;
    //output << "switch(t__)" << "{" << std::endl;
    //output << "  case(" << string_id << "): {" << std::endl;
    output << "  unsigned int i, j;" << std::endl;
    output << "unsigned int cont_id__ = 0;" << std::endl;
    output << "unsigned int timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;
    output << "if (((Actor__*)m__->recipient)->continuation_stack->current_size > 0) {" << std::endl;
    output << "  cont_id__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  i = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  j = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  t__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  v__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, void*);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "}" << std::endl;
    output << "if (v__ == NULL) return;" << std::endl;

    output << " switch(cont_id__) {" << std::endl;
    output << " case(0):" << std::endl;

    output << "if (t__ == " << string_id << ") {" << std::endl;
    output << "  delete_char_string__ ((";
    codegen_typesig(p, string_id, output);
    output << ")v__);" << std::endl;
    //output << "  }; break;" << std::endl;
    output << "  }" << std::endl;
    for (unsigned int i = obj_id; i < p->types.size(); ++i) {
        //output << "  case(" << i << "): {" << std::endl;
        output << "else if (t__ == " << i << ") { " << std::endl;
        Type_Def *td = p->types[i];

        if (i == (unsigned)obj_id) {
            output << "case(" << current_cont << "):" << std::endl;
            output << "  delete__(m__, v__, ((Object_Feature__*)v__)->feature_id);" << std::endl;
            output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
            output << "  cont_id__ = " << current_cont << ";" <<std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
            output << "  return;" << std::endl;
            output << "}" << std::endl;
            ++current_cont;
        }
        else if (td->container == Container_Type::ARRAY) {
            if (is_complex_type(p, td->contained_type_def_nums[0])) {
                output << "  for (i = 0; i < ((Typeless_Vector__ *)v__)->current_size; ++i)" << std::endl << "  {" << std::endl;
                output << "case(" << current_cont << "):" << std::endl;
                output << "    delete__(m__, INDEX_AT__(((Typeless_Vector__ *)v__), i, ";
                codegen_typesig(p, td->contained_type_def_nums[0], output);
                output << "), " << td->contained_type_def_nums[0] << ");" << std::endl;
                output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                output << "  return;" << std::endl;
                output << "}" << std::endl;
                ++current_cont;
                output << "  }" << std::endl;
            }
            output << "  delete_typeless_vector__((Typeless_Vector__ *)v__);" << std::endl;
        }
        else if (td->container == Container_Type::DICT) {
            if (is_complex_type(p, td->contained_type_def_nums[0])) {
                output << "  for (i = 0; i < DEFAULT_TYPELESS_DICTIONARY_SIZE__; ++i)" << std::endl << "  {" << std::endl;
                output << "    if (((Typeless_Dictionary__*)v__)->contents[i] != NULL) {" << std::endl;
                output << "      for (j = 0; j < ((Typeless_Dictionary__*)v__)->contents[i]->current_size; ++j) {" << std::endl;
                output << "case(" << current_cont << "):" << std::endl;
                output << "        delete__(m__, INDEX_AT__(((Typeless_Dictionary__*)v__)->contents[i], j, Dict_Unit__).data.VoidPtr, " << td->contained_type_def_nums[0] << ");" << std::endl;
                output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                output << "  return;" << std::endl;
                output << "}" << std::endl;
                ++current_cont;
                output << "      }" << std::endl;
                output << "    }" << std::endl;
                output << "  }" << std::endl;
            }
            output << "  delete_typeless_dictionary__((Typeless_Dictionary__ *)v__);" << std::endl;
        }
        else if ((td->token->type == Token_Type::ACTOR_DEF) || (td->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
            std::map<std::string, unsigned int>::iterator del_meth = td->token->scope->local_funs.find("delete");

            if ((del_meth != td->token->scope->local_funs.end()) &&
                    (p->funs[del_meth->second]->is_internal == false)){
                //Function_Def *delete_fd = p->funs[td->token->scope->local_funs["delete"]];
                output << "case(" << current_cont << "):" << std::endl;
                output << "  fun__" << td->token->scope->local_funs["delete"] << "(m__,  (";
                codegen_typesig(p, i, output);
                output << ")v__);" << std::endl;
                output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                output << "  return;" << std::endl;
                output << "}" << std::endl;
                ++current_cont;
            }
            else {
                for (std::map<std::string, unsigned int>::iterator iter = td->token->scope->local_vars.begin(),
                        end = td->token->scope->local_vars.end(); iter != end; ++iter) {
                    Var_Def *vd = p->vars[iter->second];
                    //if ((vd->type_def_num >= obj_id) || (vd->type_def_num == string_id)) {
                    if (is_complex_type(p, vd->type_def_num)) {
                        output << "case(" << current_cont << "):" << std::endl;
                        output << "  delete__(m__, ((";
                        codegen_typesig(p, i, output);
                        output << ")v__)->var__"
                            << iter->second << ", " << vd->type_def_num << ");" << std::endl;
                        output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                        output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                        output << "  return;" << std::endl;
                        output << "}" << std::endl;
                        ++current_cont;
                    }
                }
            }
        }
        else if (td->token->type == Token_Type::FEATURE_DEF) {
            std::map<std::string, unsigned int>::iterator del_meth = td->token->scope->local_funs.find("delete");

            if ((del_meth != td->token->scope->local_funs.end()) &&
                    (p->funs[del_meth->second]->is_internal == false)){
                //Function_Def *delete_fd = p->funs[td->token->scope->local_funs["delete"]];
                output << "case(" << current_cont << "):" << std::endl;
                output << "  fun__" << td->token->scope->local_funs["delete"] << "(m__,  (";
                codegen_typesig(p, i, output);
                output << ")v__);" << std::endl;
                output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                output << "  return;" << std::endl;
                output << "}" << std::endl;
                ++current_cont;
            }
            else {
                for (std::map<std::string, unsigned int>::iterator iter = td->token->scope->local_vars.begin(),
                        end = td->token->scope->local_vars.end(); iter != end; ++iter) {
                    Var_Def *vd = p->vars[iter->second];
                    //if ((vd->type_def_num >= obj_id) || (vd->type_def_num == string_id)) {
                    if (is_complex_type(p, vd->type_def_num)) {
                        output << "case(" << current_cont << "):" << std::endl;
                        output << "  delete__(m__, ((";
                        codegen_typesig(p, i, output);
                        output << ")v__)->var__"
                            << iter->second << ", " << vd->type_def_num << ");" << std::endl;
                        output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
                        output << "  cont_id__ = " << current_cont << ";" <<std::endl;
                        output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
                        output << "  return;" << std::endl;
                        output << "}" << std::endl;
                        ++current_cont;
                    }
                }
            }
            output << "  if (((Object_Feature__*)v__)->next != NULL) {" << std::endl;
            output << "case(" << current_cont << "):" << std::endl;
            output << "    delete__(m__,  ((Object_Feature__*)v__)->next, ((Object_Feature__*)(((Object_Feature__*)v__)->next))->feature_id);" << std::endl;
            output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &v__);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &t__);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &j);" << std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
            output << "  cont_id__ = " << current_cont << ";" <<std::endl;
            output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
            output << "  return;" << std::endl;
            output << "}" << std::endl;
            ++current_cont;
            output << "  }" << std::endl;
#ifdef USE_MEMBLOCK_CACHE
            output << "  recycle_memblock__(m__->sched, v__, sizeof(";
            codegen_typesig_no_tail(p, i, output);
            output << "));" << std::endl;
#else
            output << "  free(v__);" << std::endl;
#endif
        }
        //output << "  }; break;" << std::endl;
        output << "}" << std::endl;
    }
    output << "}" << std::endl;

    output << "}" << std::endl;
}

void Codegen::codegen_class_predecl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->types.size(); ++i) {
		if ((p->types[i]->is_internal == false) && (p->types[i]->container == Container_Type::SCALAR) && (p->types[i]->token->type != Token_Type::ENUM_DEF)) {
            output << "struct type__" << i << ";" << std::endl;
        }
    }
}

void Codegen::codegen_class_decl(Program *p, Token *t, std::ostringstream &output) {
    for (unsigned int i = 0; i < p->types.size(); ++i) {

		if ((p->types[i]->is_internal == false) && (p->types[i]->container == Container_Type::SCALAR) && (p->types[i]->token->type != Token_Type::ENUM_DEF)) {
            output << "struct type__" << i << "{" << std::endl;
            if ((p->types[i]->token->type == Token_Type::ACTOR_DEF) || (p->types[i]->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
                output << "Actor__ base__;" << std::endl;
            }
            else if (p->types[i]->token->type == Token_Type::FEATURE_DEF) {
                output << "Object_Feature__ base__;" << std::endl;
            }
            for (std::map<std::string, unsigned int>::iterator iter = p->types[i]->token->scope->local_vars.begin(),
                    end = p->types[i]->token->scope->local_vars.end(); iter != end; ++iter)
            {
                Var_Def *vd = p->vars[iter->second];
                codegen_typesig(p, vd->type_def_num, output);
                output << " var__" << iter->second << ";" << std::endl;
            }
            output << "};" << std::endl;
        }
    }
}

void Codegen::codegen_main_action(Program *p, std::ostringstream &output) {
    std::map<std::string, unsigned int>::iterator iter = p->global->local_funs.find("main");
    bool use_cmdline = false;

    if (iter == p->global->local_funs.end()) {
        Position pos;
        pos.line = 0;

        //if the blank one isn't find, look for the commandline one
        std::ostringstream typename_t;
        typename_t << "Array___" << p->global->local_types["string"];
        std::map<std::string, unsigned int>::iterator iter_t = p->global->local_types.find(typename_t.str());

        if (iter_t == p->global->local_types.end()) {
            throw Compiler_Exception("Main action not found or incorrect main action", pos, pos);
        }

        std::ostringstream mainname;
        mainname << "main__" << iter_t->second;
        iter = p->global->local_funs.find(mainname.str());

        if (iter == p->global->local_funs.end()) {
            throw Compiler_Exception("Main action not found or incorrect main action", pos, pos);
        }
        else {
            use_cmdline = true;
        }
    }
    Function_Def *main_action = p->funs[iter->second];
    if (main_action->token->type != Token_Type::ACTION_DEF) {
        throw Compiler_Exception("Main action not found.  Use 'action main' to define a starting point in your application",
                main_action->token->start_pos, main_action->token->end_pos);
    }


    output << "int main(int argc, char *argv[]) {" << std::endl;
    if (use_cmdline == false) {
        output << "aquarium_main__(argc, argv, fun__" << iter->second << ", FALSE);" << std::endl;
    }
    else {
        output << "aquarium_main__(argc, argv, fun__" << iter->second << ", TRUE);" << std::endl;
    }
    output << "}" << std::endl;
}

void Codegen::codegen_array_concat_decl(Program *p, std::ostringstream &output) {
    output << "Typeless_Vector__ *concatenate_new_complex_array__(Message__ * m__, Typeless_Vector__ *tv1, Typeless_Vector__ *tv2, int type_def) {" << std::endl;

    output << "  int i;" << std::endl;
    output << "  Typeless_Vector__ *output = create_typeless_vector__(tv1->elem_size, 0);" << std::endl;
    output << "  void *tmp;" << std::endl;

    output << "unsigned int cont_id__ = 0;" << std::endl;
    output << "unsigned int timeslice__ = ((Actor__*)m__->recipient)->timeslice_remaining;" << std::endl;
    output << "if (((Actor__*)m__->recipient)->continuation_stack->current_size > 0) {" << std::endl;
    output << "  cont_id__ = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, unsigned int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  i = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  output = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, Typeless_Vector__*);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  tmp = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, void*);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  tv1 = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, Typeless_Vector__*);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  tv2 = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, Typeless_Vector__*);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "  type_def = INDEX_AT__(((Actor__*)m__->recipient)->continuation_stack, ((Actor__*)m__->recipient)->continuation_stack->current_size - 1, int);" << std::endl;
    output << "  pop_off_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack);" << std::endl;
    output << "}" << std::endl;

    output << " switch(cont_id__) {" << std::endl;
    output << " case(0):" << std::endl;
    output << "  for (i = 0; i < tv1->current_size; ++i) {" << std::endl;
    output << " case(1):" << std::endl;
    output << "    tmp = copy__(m__, INDEX_AT__(tv1, i, void*), type_def);" << std::endl;
    output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &type_def);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tv2);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tv1);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &output);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
    output << "  cont_id__ = 1;" <<std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
    output << "  return NULL;" << std::endl;
    output << "}" << std::endl;
    /*
    output << "if (--((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &output);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
    output << "  cont_id__ = 2;" <<std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
    output << "  return NULL;" << std::endl;
    output << "}" << std::endl;
    output << " case(2):" << std::endl;
    */
    output << "    push_onto_typeless_vector__(output, &tmp);" << std::endl;;
    output << "  }" << std::endl;
    output << "  for (i = 0; i < tv2->current_size; ++i) {" << std::endl;
    output << " case(2):" << std::endl;
    output << "    tmp = copy__(m__, INDEX_AT__(tv2, i, void*), type_def);" << std::endl;
    output << "if (((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &type_def);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tv2);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tv1);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &output);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
    output << "  cont_id__ = 2;" <<std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
    output << "  return NULL;" << std::endl;
    output << "}" << std::endl;
    /*
    output << "if (--((Actor__*)m__->recipient)->timeslice_remaining == 0) {" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &tmp);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &output);" << std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &i);" << std::endl;
    output << "  cont_id__ = 4;" <<std::endl;
    output << "  push_onto_typeless_vector__(((Actor__*)m__->recipient)->continuation_stack, &cont_id__);" << std::endl;
    output << "  return NULL;" << std::endl;
    output << "}" << std::endl;
    output << " case(4):" << std::endl;
    */
    output << "    push_onto_typeless_vector__(output, &tmp);" << std::endl;;
    output << "  }" << std::endl;
    output << " }" << std::endl;
    output << "  return output;" << std::endl;
    output << "}" << std::endl;
}

void Codegen::codegen_enum_pretty_print(Program *p, Token *t, std::ostringstream &output) {

    for (unsigned int i = 0; i < p->types.size(); ++i) {
        Type_Def *td = p->types[i];
        if (td->token != NULL) {
            if (td->token->type == Token_Type::ENUM_DEF) {

                output << "void print_enum_" << i << "__(int val) {" << std::endl;
                output << "  switch(val) {" << std::endl;
                for (unsigned int j = 2; j < td->token->children.size(); ++j) {
                    output << "  case(" << j-2 << "): printf(\""<< td->token->children[j]->contents << "\"); break;" << std::endl;
                }
                output << "  }" << std::endl;
                output << "}" << std::endl;
            }
        }
    }
}

void Codegen::codegen(Program *p, Token *t, std::ostringstream &output) {
    internal_type_map[p->global->local_types["void"]] = Internal_Type::VOID;
    internal_type_map[p->global->local_types["int"]] = Internal_Type::INT;
    internal_type_map[p->global->local_types["bool"]] = Internal_Type::BOOL;
    internal_type_map[p->global->local_types["uint"]] = Internal_Type::UINT;
    internal_type_map[p->global->local_types["string"]] = Internal_Type::STRING;
    internal_type_map[p->global->local_types["float"]] = Internal_Type::FLOAT;
    internal_type_map[p->global->local_types["double"]] = Internal_Type::DOUBLE;
    internal_type_map[p->global->local_types["char"]] = Internal_Type::CHAR;
    internal_type_map[p->global->local_types["pointer"]] = Internal_Type::POINTER;
    internal_type_map[p->global->local_types["object"]] = Internal_Type::OBJECT;

    output << "#include <Aquarium.hpp>" << std::endl;
    output << "#include <math.h>" << std::endl;
    output << "#include <stdio.h>" << std::endl;

    codegen_enum_pretty_print(p, t, output);
    codegen_class_predecl(p, t, output);
    codegen_constructor_internal_predecl(p, t, output);
    codegen_constructor_not_internal_predecl(p, t, output);
    codegen_fun_predecl(p, t, output);
    codegen_action_predecl(p, t, output);
    codegen_copy_predecl(p, 0, output);
    codegen_delete_predecl(p, output);

    codegen_array_concat_decl(p, output);

    codegen_class_decl(p, t, output);
    codegen_constructor_internal_decl(p, t, output);
    codegen_constructor_not_internal_decl(p, t, output);
    codegen_fun_decl(p, t, output);
    codegen_action_decl(p, t, output);
    codegen_copy_decl(p, 0, output);
    codegen_delete_decl(p, output);

    codegen_main_action(p, output);


}
