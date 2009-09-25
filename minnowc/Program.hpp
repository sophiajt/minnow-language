// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef PROGRAM_HPP_
#define PROGRAM_HPP_

#include "Common.hpp"

#include <map>
#include <vector>
#include <sstream>

class Program {
public:
    std::vector<Function_Def*> funs;
    std::vector<Type_Def*> types;
    std::vector<Var_Def*> vars;
    std::vector< std::vector<int> > var_sites;
    std::vector<Token *> files;
    std::vector<std::string> libs;

    Scope *global;

    void build_internal_conversion_methods(Type_Def *owner, const char *target, unsigned int target_type) {
        std::ostringstream to_name, from_name;
        to_name << "to_" << target;
        from_name << "from_" << target << "__" << target_type;

        Function_Def *to_fd = new Function_Def(true);
        to_fd->return_type_def_num = target_type;
        to_fd->token = new Token(Token_Type::FUN_DEF);

        Function_Def *from_fd = new Function_Def(true);
        from_fd->return_type_def_num = global->local_types["void"];
        from_fd->arg_def_nums.push_back(target_type);
        from_fd->token = new Token(Token_Type::FUN_DEF);

        funs.push_back(to_fd);
        owner->token->scope->local_funs[to_name.str()] = funs.size() - 1;
        funs.push_back(from_fd);
        owner->token->scope->local_funs[from_name.str()] = funs.size() - 1;
    }

    void build_internal_bit_methods(Type_Def *owner) {
        unsigned int int_def_num = global->local_types["int"];
        const char* method_names[] = {"bit_shl", "bit_shr", "bit_and", "bit_or", "bit_xor"};
        for (int i = 0; i < 5; ++i) {
            std::ostringstream method_name;
            method_name << method_names[i] << "__" << int_def_num;

            Function_Def *method_fd = new Function_Def(true);
            method_fd->return_type_def_num = global->local_types["int"];
            method_fd->arg_def_nums.push_back(int_def_num);
            method_fd->token = new Token(Token_Type::FUN_DEF);

            funs.push_back(method_fd);
            owner->token->scope->local_funs[method_name.str()] = funs.size() - 1;
        }
        Function_Def *method_fd = new Function_Def(true);
        method_fd->return_type_def_num = global->local_types["int"];
        method_fd->token = new Token(Token_Type::FUN_DEF);

        funs.push_back(method_fd);
        owner->token->scope->local_funs["bit_not"] = funs.size() - 1;

    }

    void build_internal_array_methods(Type_Def *owner, unsigned int type_def_num) {
        std::ostringstream push_name, pop_name, size_name, empty_name, insert_name, delete_name;
        push_name << "push__" << owner->contained_type_def_nums[0];
        pop_name << "pop";
        size_name << "size";
        empty_name << "empty";
        insert_name << "insert__" << owner->contained_type_def_nums[0] << "__"  << global->local_types["int"];
        delete_name << "delete__" << global->local_types["int"];

        Function_Def *push_fd = new Function_Def(true);
        push_fd->return_type_def_num = global->local_types["void"];
        push_fd->arg_def_nums.push_back(owner->contained_type_def_nums[0]);
        push_fd->token = new Token(Token_Type::FUN_DEF);
        push_fd->is_port_of_exit = true;

        Function_Def *pop_fd = new Function_Def(true);
        pop_fd->return_type_def_num = owner->contained_type_def_nums[0];
        pop_fd->token = new Token(Token_Type::FUN_DEF);

        Function_Def *size_fd = new Function_Def(true);
        size_fd->return_type_def_num = global->local_types["int"];
        size_fd->token = new Token(Token_Type::FUN_DEF);

        Function_Def *empty_fd = new Function_Def(true);
        empty_fd->return_type_def_num = global->local_types["bool"];
        empty_fd->token = new Token(Token_Type::FUN_DEF);

        Function_Def *insert_fd = new Function_Def(true);
        insert_fd->return_type_def_num = global->local_types["void"];
        insert_fd->arg_def_nums.push_back(owner->contained_type_def_nums[0]);
        insert_fd->arg_def_nums.push_back(global->local_types["int"]);
        insert_fd->token = new Token(Token_Type::FUN_DEF);
        insert_fd->is_port_of_exit = true;

        Function_Def *delete_fd = new Function_Def(true);
        delete_fd->return_type_def_num = owner->contained_type_def_nums[0];
        delete_fd->arg_def_nums.push_back(global->local_types["int"]);
        delete_fd->token = new Token(Token_Type::FUN_DEF);

        //Add a check for null method for each type
        Function_Def *null_check = new Function_Def(true);
        null_check->return_type_def_num = global->local_types["bool"];
        null_check->is_port_of_exit = false;
        null_check->token = new Token(Token_Type::FUN_DEF);

        //Also add "exists", the compliment of is_null, and easier to type
        Function_Def *exists_check = new Function_Def(true);
        exists_check->return_type_def_num = global->local_types["bool"];
        exists_check->is_port_of_exit = false;
        exists_check->token = new Token(Token_Type::FUN_DEF);

        funs.push_back(push_fd);
        owner->token->scope->local_funs[push_name.str()] = funs.size() - 1;
        funs.push_back(pop_fd);
        owner->token->scope->local_funs[pop_name.str()] = funs.size() - 1;
        funs.push_back(size_fd);
        owner->token->scope->local_funs[size_name.str()] = funs.size() - 1;
        funs.push_back(empty_fd);
        owner->token->scope->local_funs[empty_name.str()] = funs.size() - 1;
        funs.push_back(insert_fd);
        owner->token->scope->local_funs[insert_name.str()] = funs.size() - 1;
        funs.push_back(delete_fd);
        owner->token->scope->local_funs[delete_name.str()] = funs.size() - 1;
        funs.push_back(null_check);
        owner->token->scope->local_funs["is_null"] = funs.size() - 1;
        funs.push_back(exists_check);
        owner->token->scope->local_funs["exists"] = funs.size() - 1;
    }

    void build_internal_dict_methods(Type_Def *owner, unsigned int type_def_num) {
        std::ostringstream contains_key_name;
        contains_key_name << "has_key__" << global->local_types["string"];

        Function_Def *contains_key_fd = new Function_Def(true);
        contains_key_fd->return_type_def_num = global->local_types["bool"];
        contains_key_fd->arg_def_nums.push_back(global->local_types["string"]);
        contains_key_fd->token = new Token(Token_Type::FUN_DEF);

        //Add a check for null method for each type
        Function_Def *null_check = new Function_Def(true);
        null_check->return_type_def_num = global->local_types["bool"];
        null_check->is_port_of_exit = false;
        null_check->token = new Token(Token_Type::FUN_DEF);

        //Also add "exists", the compliment of is_null, and easier to type
        Function_Def *exists_check = new Function_Def(true);
        exists_check->return_type_def_num = global->local_types["bool"];
        exists_check->is_port_of_exit = false;
        exists_check->token = new Token(Token_Type::FUN_DEF);

        funs.push_back(contains_key_fd);
        owner->token->scope->local_funs[contains_key_name.str()] = funs.size() - 1;
        funs.push_back(null_check);
        owner->token->scope->local_funs["is_null"] = funs.size() - 1;
        funs.push_back(exists_check);
        owner->token->scope->local_funs["exists"] = funs.size() - 1;

    }

    void add_default_types() {
        //KEEP OBJECT LAST !!!!!!!!  At least for the time being it's used for meld operator type checking
        //KEEP STRING AS FIRST COMPLEX!!!
        const char *internal_types[] = {"error", "var", "void", "int", "uint", "bool", "float", "double", "char", "string", "pointer", "object"};

        for (unsigned int i = 0; i < (sizeof(internal_types) / sizeof(char*)) ; ++i) {
            Type_Def *td = new Type_Def(true);
            //todo: add functions to internal type
            //todo: fix error message for reduxing an internal type or function
            td->token = new Token(Token_Type::FEATURE_DEF);
            td->token->scope = new Scope();
            types.push_back(td);

            unsigned int type_def_num = types.size() - 1;
            global->local_types[internal_types[i]] = type_def_num;

            for (unsigned int j = 3; j <= 9 ; ++j) {
                if ((j != i) && (i >= 3) && (i <= 9)) {
                    build_internal_conversion_methods(td, internal_types[j], j);
                }
            }

            if ((strcmp(internal_types[i], "string") == 0) ||
                    (strcmp(internal_types[i], "pointer") == 0) ||
                    (strcmp(internal_types[i], "object") == 0)) {
                //Add a check for null method for each type
                Function_Def *null_check = new Function_Def(true);
                null_check->return_type_def_num = global->local_types["bool"];
                null_check->is_port_of_exit = false;
                null_check->token = new Token(Token_Type::FUN_DEF);

                funs.push_back(null_check);
                td->token->scope->local_funs["is_null"] = funs.size() - 1;

                //Also add "exists", the compliment of is_null, and easier to type
                Function_Def *exists_check = new Function_Def(true);
                exists_check->return_type_def_num = global->local_types["bool"];
                exists_check->is_port_of_exit = false;
                exists_check->token = new Token(Token_Type::FUN_DEF);

                funs.push_back(exists_check);
                td->token->scope->local_funs["exists"] = funs.size() - 1;
            }
            else if (strcmp(internal_types[i], "int") == 0) {
                build_internal_bit_methods(td);
            }
        }
        //Handle string differently
        unsigned int td_type_def_num = this->global->local_types["string"];
        Type_Def *td_string = this->types[td_type_def_num];
        td_string->container = Container_Type::ARRAY;
        td_string->contained_type_def_nums.push_back(this->global->local_types["char"]);
        build_internal_array_methods(td_string, td_type_def_num);

    }

    void build_internal_func(unsigned int arg_type1, unsigned int arg_type2, unsigned int ret_type, const char *op) {
        std::ostringstream name;

        Function_Def *fd = new Function_Def(true, ret_type, arg_type1, arg_type2);

        funs.push_back(fd);
        name << op << "__" << arg_type1 << "__" << arg_type2;
        global->local_funs[name.str()] = funs.size() - 1;
    }

    void build_throw_func() {
        std::ostringstream name;

        Function_Def *throw_fd = new Function_Def(true);
        throw_fd->return_type_def_num = global->local_types["void"];
        throw_fd->arg_def_nums.push_back(global->local_types["object"]);
        funs.push_back(throw_fd);

        name << "throw__" << global->local_types["object"];
        global->local_funs[name.str()] = funs.size() - 1;
    }

    void add_default_funcs() {
        unsigned int int_def_num = global->local_types["int"];
        unsigned int bool_def_num = global->local_types["bool"];
        unsigned int uint_def_num = global->local_types["uint"];
        unsigned int float_def_num = global->local_types["float"];
        unsigned int double_def_num = global->local_types["double"];
        unsigned int string_def_num = global->local_types["string"];
        unsigned int char_def_num = global->local_types["char"];

        build_throw_func();

        build_internal_func(int_def_num, int_def_num, int_def_num, "=");
        build_internal_func(int_def_num, int_def_num, int_def_num, "+");
        build_internal_func(int_def_num, int_def_num, int_def_num, "-");
        build_internal_func(int_def_num, int_def_num, int_def_num, "*");
        build_internal_func(int_def_num, int_def_num, int_def_num, "/");
        build_internal_func(int_def_num, int_def_num, int_def_num, "%");
        build_internal_func(int_def_num, int_def_num, int_def_num, "**");
        build_internal_func(int_def_num, int_def_num, bool_def_num, "==");
        build_internal_func(int_def_num, int_def_num, bool_def_num, "!=");
        build_internal_func(int_def_num, int_def_num, bool_def_num, ">");
        build_internal_func(int_def_num, int_def_num, bool_def_num, "<");
        build_internal_func(int_def_num, int_def_num, bool_def_num, ">=");
        build_internal_func(int_def_num, int_def_num, bool_def_num, "<=");

        build_internal_func(bool_def_num, bool_def_num, bool_def_num, "=");
        build_internal_func(bool_def_num, bool_def_num, bool_def_num, "==");
        build_internal_func(bool_def_num, bool_def_num, bool_def_num, "!=");
        build_internal_func(bool_def_num, bool_def_num, bool_def_num, "&&");
        build_internal_func(bool_def_num, bool_def_num, bool_def_num, "||");

        build_internal_func(uint_def_num, uint_def_num, uint_def_num, "=");
        build_internal_func(uint_def_num, uint_def_num, uint_def_num, "+");
        build_internal_func(uint_def_num, uint_def_num, uint_def_num, "-");
        build_internal_func(uint_def_num, uint_def_num, uint_def_num, "*");
        build_internal_func(uint_def_num, uint_def_num, uint_def_num, "/");
        build_internal_func(uint_def_num, uint_def_num, uint_def_num, "**");
        build_internal_func(uint_def_num, uint_def_num, bool_def_num, "==");
        build_internal_func(uint_def_num, uint_def_num, bool_def_num, "!=");
        build_internal_func(uint_def_num, uint_def_num, bool_def_num, ">");
        build_internal_func(uint_def_num, uint_def_num, bool_def_num, "<");
        build_internal_func(uint_def_num, uint_def_num, bool_def_num, ">=");
        build_internal_func(uint_def_num, uint_def_num, bool_def_num, "<=");

        build_internal_func(float_def_num, float_def_num, float_def_num, "=");
        build_internal_func(float_def_num, float_def_num, float_def_num, "+");
        build_internal_func(float_def_num, float_def_num, float_def_num, "-");
        build_internal_func(float_def_num, float_def_num, float_def_num, "*");
        build_internal_func(float_def_num, float_def_num, float_def_num, "/");
        build_internal_func(float_def_num, float_def_num, float_def_num, "**");
        build_internal_func(float_def_num, float_def_num, bool_def_num, "==");
        build_internal_func(float_def_num, float_def_num, bool_def_num, "!=");
        build_internal_func(float_def_num, float_def_num, bool_def_num, ">");
        build_internal_func(float_def_num, float_def_num, bool_def_num, "<");
        build_internal_func(float_def_num, float_def_num, bool_def_num, ">=");
        build_internal_func(float_def_num, float_def_num, bool_def_num, "<=");

        build_internal_func(double_def_num, double_def_num, double_def_num, "=");
        build_internal_func(double_def_num, double_def_num, double_def_num, "+");
        build_internal_func(double_def_num, double_def_num, double_def_num, "-");
        build_internal_func(double_def_num, double_def_num, double_def_num, "*");
        build_internal_func(double_def_num, double_def_num, double_def_num, "/");
        build_internal_func(double_def_num, double_def_num, double_def_num, "**");
        build_internal_func(double_def_num, double_def_num, bool_def_num, "==");
        build_internal_func(double_def_num, double_def_num, bool_def_num, "!=");
        build_internal_func(double_def_num, double_def_num, bool_def_num, ">");
        build_internal_func(double_def_num, double_def_num, bool_def_num, "<");
        build_internal_func(double_def_num, double_def_num, bool_def_num, ">=");
        build_internal_func(double_def_num, double_def_num, bool_def_num, "<=");

        build_internal_func(char_def_num, char_def_num, bool_def_num, "==");
        build_internal_func(char_def_num, char_def_num, bool_def_num, "!=");

        build_internal_func(string_def_num, string_def_num, bool_def_num, "==");
        build_internal_func(string_def_num, string_def_num, bool_def_num, "!=");
        build_internal_func(string_def_num, string_def_num, bool_def_num, ">");
        build_internal_func(string_def_num, string_def_num, bool_def_num, "<");
        build_internal_func(string_def_num, string_def_num, bool_def_num, ">=");
        build_internal_func(string_def_num, string_def_num, bool_def_num, "<=");

        build_internal_func(string_def_num, string_def_num, string_def_num, "=");
        build_internal_func(string_def_num, string_def_num, string_def_num, "+");
        build_internal_func(string_def_num, char_def_num, string_def_num, "+");

        build_internal_func(char_def_num, char_def_num, char_def_num, "=");
        build_internal_func(char_def_num, string_def_num, string_def_num, "+");

    }

    Program() {
        global = new Scope();

        //Default types
        add_default_types();
        add_default_funcs();

        libs.push_back("aquarium");
    }
};



#endif /* PROGRAM_HPP_ */
