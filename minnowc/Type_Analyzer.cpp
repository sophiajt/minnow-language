// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <algorithm>
#include <iostream>

#include "Common.hpp"
#include "Var_Scope_Analyzer.hpp"
#include "Type_Analyzer.hpp"

bool is_complex_type(Program *program, unsigned int type_def_num) {
    Type_Def *td = program->types[type_def_num];

    if (((type_def_num >= program->global->local_types["object"]) ||
            (type_def_num == program->global->local_types["string"])) &&
        (td->token->type != Token_Type::ACTOR_DEF) &&
        (td->token->type != Token_Type::ISOLATED_ACTOR_DEF) &&
        (td->token->type != Token_Type::ENUM_DEF)) {
        return true;
    }
    else {
        return false;
    }

}
bool is_complex_var(Program *program, unsigned int definition_number) {
    Var_Def *vd = program->vars[definition_number];

    return is_complex_type(program, vd->type_def_num);
}


Scope *Type_Analyzer::find_or_create_namespace(Program *program, Token *ns) {
    Scope *ret_val;
    //First, recurse left
    if (ns->children.size() > 0) {
        ret_val = find_or_create_namespace(program, ns->children[0]);
    }
    else {
        //We hit the bottom
        if (ns->contents == "global") {
            return program->global;
        }
        else if (program->global->namespaces.find(ns->contents) != program->global->namespaces.end()) {
            return program->global->namespaces[ns->contents];
        }
        else {
            Scope *new_scope = new Scope();
            program->global->namespaces[ns->contents] = new_scope;
            new_scope->parent = program->global;
            new_scope->owner = ns;
            return new_scope;
        }
    }
    //If we get here, we should be at a '.' for a deep namespace
    if (ns->contents != ".") {
        throw Compiler_Exception("Use '.' for declaring a deep namespace", ns->start_pos, ns->end_pos);
    }
    else {
        if (ret_val->namespaces.find(ns->children[1]->contents) != ret_val->namespaces.end()) {
            return ret_val->namespaces[ns->children[1]->contents];
        }
        else {
            Scope *new_scope = new Scope();
            ret_val->namespaces[ns->children[1]->contents] = new_scope;
            new_scope->parent = ret_val;
            new_scope->owner = ns;
            return new_scope;
        }
    }
}

Scope *Type_Analyzer::find_namespace(Program *program, Token *ns, bool throw_exceptions) {
    Scope *ret_val;
    //First, recurse left
    if (ns->children.size() > 0) {
        ret_val = find_namespace(program, ns->children[0], throw_exceptions);
        if (ret_val == NULL) {
            return NULL;
        }
    }
    else {
        //We hit the bottom
        if (ns->contents == "global") {
            return program->global;
        }
        else if (program->global->namespaces.find(ns->contents) != program->global->namespaces.end()) {
            return program->global->namespaces[ns->contents];
        }
        else {
            if (throw_exceptions) {
                throw Compiler_Exception("Namespace not found", ns->start_pos, ns->end_pos);
            }
            else {
                return NULL;
            }
        }
    }
    //If we get here, we should be at a '.' for a deep namespace
    if (ns->contents != ".") {
        if (throw_exceptions) {
            throw Compiler_Exception("Use '.' for specifying a deep namespace", ns->start_pos, ns->end_pos);
        }
        else {
            return NULL;
        }
    }
    else {
        if (ret_val->namespaces.find(ns->children[1]->contents) != ret_val->namespaces.end()) {
            return ret_val->namespaces[ns->children[1]->contents];
        }
        else {
            if (throw_exceptions) {
                throw Compiler_Exception("Namespace not found", ns->start_pos, ns->end_pos);
            }
            else {
                return NULL;
            }
        }
    }
}
unsigned int Type_Analyzer::find_type(Program *program, Token *ns, Scope *scope) {
    //unsigned int ret_val;

    if (ns->children.size() == 0) {
        //If there are no children, we have to find the type up the scope tree
        while (scope != NULL) {
            if (scope->local_types.find(ns->contents) != scope->local_types.end()) {
                ns->definition_number = scope->local_types[ns->contents];
                ns->type_def_num = ns->definition_number;
                return ns->definition_number;
            }
            scope = scope->parent;
        }
        throw Compiler_Exception("Can not find type '" + ns->contents + "'", ns->start_pos, ns->end_pos);
    }
    else {
        if (ns->type == Token_Type::ARRAY_CALL) {
            if (ns->children.size() != 2) {
                throw Compiler_Exception("Type incomplete", ns->start_pos, ns->end_pos);
            }
            else if ((ns->children[0]->contents != "Array") && (ns->children[0]->contents != "Dict")) {
                throw Compiler_Exception("Unsupported container type", ns->children[0]->start_pos, ns->children[0]->end_pos);
            }
            else {
                std::ostringstream containername;

                if (ns->children[0]->contents == "Array") {
                    find_type(program, ns->children[1], scope);

                    containername << "Array___" << ns->children[1]->definition_number;
                    if (program->global->local_types.find(containername.str()) != program->global->local_types.end()) {
                        ns->definition_number = program->global->local_types[containername.str()];
                        ns->type_def_num = ns->definition_number;
                        return ns->definition_number;
                    }
                    else {
                        //It doesn't exist yet, so let's create it
                        Type_Def *container = new Type_Def();
                        container->container = Container_Type::ARRAY;
                        container->contained_type_def_nums.push_back(ns->children[1]->type_def_num);
                        container->token = new Token(Token_Type::EMPTY); //todo: set this to something reasonable
                        container->token->scope = new Scope();
                        container->token->scope->owner = container->token;
                        program->types.push_back(container);

                        unsigned int def_num = program->types.size() - 1;

                        program->global->local_types[containername.str()] = def_num;
                        ns->definition_number = def_num;
                        ns->type_def_num = def_num;

                        program->build_internal_array_methods(container, def_num);

                        return ns->definition_number;
                    }
                }
                else if (ns->children[0]->contents == "Dict") {
                    find_type(program, ns->children[1], scope);

                    containername << "Dict___" << ns->children[1]->definition_number;
                    if (program->global->local_types.find(containername.str()) != program->global->local_types.end()) {
                        ns->definition_number = program->global->local_types[containername.str()];
                        ns->type_def_num = ns->definition_number;
                        return ns->definition_number;
                    }
                    else {
                        //It doesn't exist yet, so let's create it
                        Type_Def *container = new Type_Def();
                        container->container = Container_Type::DICT;
                        container->contained_type_def_nums.push_back(ns->children[1]->type_def_num);
                        container->token = new Token(Token_Type::EMPTY); //todo: set this to something reasonable
                        container->token->scope = new Scope();
                        container->token->scope->owner = container->token;
                        program->types.push_back(container);

                        unsigned int def_num = program->types.size() - 1;

                        program->global->local_types[containername.str()] = def_num;
                        ns->definition_number = def_num;
                        ns->type_def_num = def_num;

                        program->build_internal_dict_methods(container, def_num);

                        return ns->definition_number;
                    }
                }
                else {
                    throw Compiler_Exception("Internal container type error", ns->children[0]->start_pos, ns->children[1]->end_pos);
                }
            }
        }
        else if (ns->type == Token_Type::ARRAY_INIT) {
            std::ostringstream containername;
            int contained_type;

            contained_type = ns->children[0]->type_def_num;

            containername << "Array___" << contained_type;
            if (program->global->local_types.find(containername.str()) != program->global->local_types.end()) {
                ns->definition_number = program->global->local_types[containername.str()];
                ns->type_def_num = ns->definition_number;
                return ns->definition_number;
            }
            else {
                //It doesn't exist yet, so let's create it
                Type_Def *container = new Type_Def();
                container->container = Container_Type::ARRAY;
                container->contained_type_def_nums.push_back(contained_type);
                container->token = new Token(Token_Type::EMPTY); //todo: set this to something reasonable
                container->token->scope = new Scope();
                container->token->scope->owner = container->token;
                program->types.push_back(container);

                unsigned int def_num = program->types.size() - 1;

                program->global->local_types[containername.str()] = def_num;
                ns->definition_number = def_num;
                ns->type_def_num = def_num;

                program->build_internal_array_methods(container, def_num);

                return ns->definition_number;
            }
        }
        else if (ns->contents == ".") {
            scope = find_namespace(program, ns->children[0], true);
            if (ns->contents != ".") {
                //todo: put this some place useful
                throw Compiler_Exception("Use '.' for specifying a namespaced type", ns->start_pos, ns->end_pos);
            }
            else {
                if (scope->local_types.find(ns->children[1]->contents) != scope->local_types.end()) {
                    ns->definition_number = scope->local_types[ns->children[1]->contents];
                    ns->type_def_num = ns->definition_number;
                    return ns->definition_number;
                }
                else {
                    throw Compiler_Exception("Can not find type '" + ns->children[1]->contents + "'", ns->start_pos, ns->end_pos);
                }
            }
        }
        else if (ns->contents == "->") {
            std::vector <unsigned int> ref_types;

            while (ns->contents == "->") {
                ref_types.push_back(find_type(program, ns->children[1], scope));
                ns = ns->children[0];
            }
            ref_types.push_back(find_type(program, ns, scope));

            std::ostringstream functorname;

            functorname << "Fun___" << ref_types[0];

            for (unsigned int i = 1; i < ref_types.size(); ++i) {
                functorname << "__" << ref_types[i];
            }

            if (program->global->local_types.find(functorname.str()) != program->global->local_types.end()) {
                ns->definition_number = program->global->local_types[functorname.str()];
                ns->type_def_num = ns->definition_number;
                return ns->definition_number;
            }
            else {
                //It doesn't exist yet, so let's create it
                Type_Def *functor = new Type_Def();
                functor->container = Container_Type::FUNCTOR;
                for (unsigned int i = 0; i < ref_types.size(); ++i) {
                    functor->contained_type_def_nums.push_back(ref_types[i]);
                }
                functor->token = new Token(Token_Type::EMPTY); //todo: set this to something reasonable
                functor->token->scope = new Scope();
                functor->token->scope->owner = functor->token;
                program->types.push_back(functor);

                unsigned int def_num = program->types.size() - 1;

                program->global->local_types[functorname.str()] = def_num;
                ns->definition_number = def_num;
                ns->type_def_num = def_num;

                //program->build_internal_array_methods(functor, def_num);

                return ns->definition_number;
            }

        }
        else {
            throw Compiler_Exception("Can not find type", ns->start_pos, ns->end_pos);
        }
    }
    //return ret_val;
}

void Type_Analyzer::find_constructor(Program *program, Token *ns, Scope *scope) {
    //unsigned int ret_val;

    if (ns->type == Token_Type::FUN_CALL) {
        //If there are no children, we have to find the type up the scope tree

        Type_Def *td;

        try {
            find_type(program, ns->children[0], scope);
            td = program->types[ns->children[0]->type_def_num];

            check_fun_call(program, ns, td->token->scope, scope);
            ns->type_def_num = ns->children[0]->type_def_num;
            ns->type = Token_Type::CONSTRUCTOR_CALL;
        }
        catch (Compiler_Exception &ce){
            throw Compiler_Exception("Can not find constructor for '" + ns->children[0]->contents + "'", ns->start_pos, ns->end_pos);
        }
    }

    else {

        if (ns->type == Token_Type::ARRAY_CALL) {
            if (ns->children.size() != 2) {
                throw Compiler_Exception("Type incomplete", ns->start_pos, ns->end_pos);
            }
            else if ((ns->children[0]->contents != "Array") && (ns->children[0]->contents != "Dict")) {
                throw Compiler_Exception("Unsupported container type", ns->children[0]->start_pos, ns->children[0]->end_pos);
            }
            else {
                std::ostringstream containername;

                if (ns->children[0]->contents == "Array") {

                    find_type(program, ns->children[1], scope);

                    containername << "Array___" << ns->children[1]->definition_number;
                    if (program->global->local_types.find(containername.str()) != program->global->local_types.end()) {
                        ns->definition_number = program->global->local_types[containername.str()];
                        ns->type_def_num = ns->definition_number;
                        //return ns->definition_number;
                    }
                    else {
                        //It doesn't exist yet, so let's create it
                        Type_Def *container = new Type_Def();
                        container->container = Container_Type::ARRAY;
                        container->contained_type_def_nums.push_back(ns->children[1]->type_def_num);
                        container->token = new Token(Token_Type::EMPTY); //todo: set this to something reasonable
                        container->token->scope = new Scope();
                        program->types.push_back(container);

                        unsigned int def_num = program->types.size() - 1;

                        program->global->local_types[containername.str()] = def_num;
                        ns->definition_number = def_num;
                        ns->type_def_num = def_num;

                        program->build_internal_array_methods(container, def_num);

                        //return ns->definition_number;
                    }
                }
                else if (ns->children[0]->contents == "Dict") {

                    find_type(program, ns->children[1], scope);

                    containername << "Dict___" << ns->children[1]->definition_number;
                    if (program->global->local_types.find(containername.str()) != program->global->local_types.end()) {
                        ns->definition_number = program->global->local_types[containername.str()];
                        ns->type_def_num = ns->definition_number;
                        //return ns->definition_number;
                    }
                    else {
                        //It doesn't exist yet, so let's create it
                        Type_Def *container = new Type_Def();
                        container->container = Container_Type::DICT;
                        container->contained_type_def_nums.push_back(ns->children[1]->type_def_num);
                        container->token = new Token(Token_Type::EMPTY); //todo: set this to something reasonable
                        container->token->scope = new Scope();
                        program->types.push_back(container);

                        unsigned int def_num = program->types.size() - 1;

                        program->global->local_types[containername.str()] = def_num;
                        ns->definition_number = def_num;
                        ns->type_def_num = def_num;

                        program->build_internal_dict_methods(container, def_num);

                        //return ns->definition_number;
                    }
                }

            }
        }

        else if (ns->contents == ".") {
            Scope *type_scope = find_namespace(program, ns->children[0], true);
            /*
            if (ns->contents != ".") {
                //todo: put this some place useful
                throw Compiler_Exception("Use '.' for specifying a namespaced type", ns->start_pos);
            }
            else {
            */
            Type_Def *td;

            if (ns->children[1]->children.size() > 0) {
                try {
                    find_type(program, ns->children[1]->children[0], type_scope);
                    td = program->types[ns->children[1]->children[0]->type_def_num];

                    check_fun_call(program, ns->children[1], td->token->scope, scope);
                    //ns->type_def_num = ns->children[1]->type_def_num;
                    ns->definition_number = ns->children[1]->definition_number;
                    ns->type_def_num = ns->children[1]->children[0]->type_def_num;
                    //ns->definition_number = ns->children[1]->children[0]->definition_number;
                    ns->children[1]->type = Token_Type::CONSTRUCTOR_CALL;
                }
                catch (Compiler_Exception &ce){
                    throw Compiler_Exception("Can not find constructor for '" + ns->children[1]->children[0]->contents + "'",
                            ns->start_pos, ns->end_pos);
                }
            }
            else {
                throw Compiler_Exception("Missing constructor call", ns->start_pos, ns->end_pos);
            }
            //}
        }
        else {
            throw Compiler_Exception("Constructor not given", ns->start_pos, ns->end_pos);
        }
    }

    //return ret_val;
}

unsigned int Type_Analyzer::build_var_def(Program *program, Token *token, Scope *scope) {
    unsigned int type_def_num;

    type_def_num = find_type(program, token->children[1], scope);

    Var_Def *vd = new Var_Def();
    vd->token = token;
    vd->type_def_num = type_def_num;
    vd->usage_start = token->start_pos;
    vd->usage_end = token->end_pos;
    program->vars.push_back(vd);

    unsigned int var_def_num = program->vars.size() - 1;

    token->definition_number = var_def_num;

    return var_def_num;
}

void Type_Analyzer::build_function_args(Program *program, Token *token, Scope *scope, Function_Def *fd) {
    unsigned int var_def_num;

    if (token->type == Token_Type::QUICK_VAR_DECL) {
        throw Compiler_Exception("Quick variable declarations like $var are not allowed as parameters", token->start_pos, token->end_pos);
    }
    else if (token->contents == ",") {
        build_function_args(program, token->children[0], scope, fd);
        build_function_args(program, token->children[1], scope, fd);
    }
    /*
    else if (token->type == Token_Type::ID) {
        int implied_type = program->global->local_types["var"];
        Var_Def *vd = new Var_Def();
        vd->token = token;
        vd->type_def_num = implied_type;
        vd->usage_start = token->start_pos;
        vd->usage_end = token->end_pos;
        program->vars.push_back(vd);

        unsigned int var_def_num = program->vars.size() - 1;

        token->definition_number = var_def_num;
        //If this is an arg into an actor, make sure we know we're responsible for it
        {
            Var_Def *vd = program->vars[var_def_num];
            if (fd->token->type == Token_Type::ACTION_DEF) {
                vd->is_dependent = true;
            }
            else {
                vd->is_dependent = false;
            }
        }

        fd->arg_def_nums.push_back(var_def_num);
        if (scope->local_vars.find(token->contents) != scope->local_vars.end()) {
            Var_Def *redux = program->vars[scope->local_vars[token->contents]];
            std::ostringstream msg;
            msg << "Redefined variable '" << token->contents << "' (see also line "
                << redux->token->start_pos.line << " of " << redux->token->start_pos.filename << ")";

            throw Compiler_Exception(msg.str(), token->start_pos, token->end_pos);
        }
        scope->local_vars[token->contents] = var_def_num;
    }
    */
    else if (token->contents == ":") {
        //push the furthest lhs first
        /*
        if (token->children[1]->contents == "var") {
            //Workaround because functions are not allowed to have var types (yet?)
            throw Compiler_Exception("Implied types not allowed in function header", token->children[1]->start_pos,
                    token->children[1]->end_pos);
        }
        */
        var_def_num = build_var_def(program, token, scope);
        //If this is an arg into an actor, make sure we know we're responsible for it
        {
            Var_Def *vd = program->vars[var_def_num];
            if (fd->token->type == Token_Type::ACTION_DEF) {
                vd->is_dependent = true;
            }
            else {
                vd->is_dependent = false;
            }
        }

        fd->arg_def_nums.push_back(var_def_num);
        if (scope->local_vars.find(token->children[0]->contents) != scope->local_vars.end()) {
            Var_Def *redux = program->vars[scope->local_vars[token->children[0]->contents]];
            std::ostringstream msg;
            msg << "Redefined variable '" << token->children[0]->contents << "' (see also line "
                << redux->token->start_pos.line << " of " << redux->token->start_pos.filename << ")";

            throw Compiler_Exception(msg.str(), token->children[0]->start_pos, token->children[0]->end_pos);
        }
        scope->local_vars[token->children[0]->contents] = var_def_num;
    }
    else {
        for (unsigned int i = 0; i < token->children.size(); ++i) {
            build_function_args(program, token->children[i], scope, fd);
        }
    }
}

std::string Type_Analyzer::build_function_arg_types(Program *program, Token *token, Scope *scope) {
    std::ostringstream output;
    int void_type = program->global->local_types["void"];
    int error_type = program->global->local_types["error"];
    int var_type = program->global->local_types["var"];


    if (token->contents == ",") {
        if ((token->children[0]->type_def_num != void_type) && (token->children[0]->type_def_num != error_type) &&
                (token->children[0]->type_def_num != var_type)) {
            output << build_function_arg_types(program, token->children[0], scope);
        }
        else {
            if (token->children[0]->type_def_num == var_type) {
                throw Compiler_Exception("Functions can not take uninitialized var arguments", token->children[0]->start_pos, token->children[0]->end_pos);
            }
            else {
                throw Compiler_Exception("Functions can not take void arguments", token->children[0]->start_pos, token->children[0]->end_pos);
            }
        }
        output << "__";
        if ((token->children[1]->type_def_num != void_type) && (token->children[1]->type_def_num != error_type) &&
                (token->children[1]->type_def_num != var_type)) {
            output << build_function_arg_types(program, token->children[1], scope);
        }
        else {
            if (token->children[1]->type_def_num == var_type) {
                throw Compiler_Exception("Functions can not take uninitialized var arguments", token->children[1]->start_pos, token->children[1]->end_pos);
            }
            else {
                throw Compiler_Exception("Functions can not take void arguments", token->children[1]->start_pos, token->children[1]->end_pos);
            }
        }
    }
    else {
        if ((token->type_def_num != void_type) && (token->type_def_num != error_type) && (token->type_def_num != var_type)) {
            output << token->type_def_num;
        }
        else {
            if (token->type_def_num == var_type) {
                throw Compiler_Exception("Functions can not take uninitialized var arguments", token->start_pos, token->end_pos);
            }
            else {
                throw Compiler_Exception("Functions can not take void arguments", token->start_pos, token->end_pos);
            }
        }
    }
    return output.str();
}

std::string Type_Analyzer::build_function_name(Program *program, Token *token, Scope *scope) {
    std::ostringstream output;
    std::string args;
    output << token->children[0]->contents;
    if (token->children.size() > 1) {
        args = build_function_arg_types(program, token->children[1], scope);
    }
    if (args.length() > 0) {
        output << "__" << args;
    }
    return output.str();
}

std::string Type_Analyzer::build_function_def(Program *program, Token *token, Scope *scope, Function_Def *fd) {
    std::ostringstream full_name;
    unsigned int return_type = program->global->local_types["void"];

    if ((token->type == Token_Type::ACTION_DEF) && (token->children[1]->contents == ":")) {
        throw Compiler_Exception("Actions do not have return types", token->start_pos, token->end_pos);
    }

    if (token->children[1]->contents == ":") {
        //We have a return type

        if (token->children[1]->children[1]->contents == "var") {
            //Workaround because functions are not allowed to have var types (yet?)
            throw Compiler_Exception("Implied types are not allowed as return types",
                    token->children[1]->children[1]->start_pos, token->children[1]->children[1]->end_pos);
        }

        return_type = find_type(program, token->children[1]->children[1], scope);
        if (token->children[1]->children[0]->type == Token_Type::FUN_CALL) {
            build_function_args(program, token->children[1]->children[0], scope, fd);
        }
    }
    else if (token->children[1]->type == Token_Type::FUN_CALL) {
        build_function_args(program, token->children[1], scope, fd);
    }

    if (token->type == Token_Type::ACTION_DEF) {
        if (fd->arg_def_nums.size() > 8) {
            throw Compiler_Exception("Actions may not exceed 8 arguments", token->children[1]->start_pos, token->children[1]->end_pos);
        }
    }

    fd->return_type_def_num = return_type;

    full_name << token->contents;
    for (unsigned int i = 0; i < fd->arg_def_nums.size(); ++i) {
        full_name << "__" << program->vars[fd->arg_def_nums[i]]->type_def_num;
    }

    return full_name.str();
}

//todo: extend strays pass to catch stray tokens
void Type_Analyzer::analyze_strays(Token *token) {
    switch (token->type) {
        case (Token_Type::BLOCK) : {
            unsigned int i = 0;
            bool saw_non_eol = false;
            while (i < token->children.size()) {
                if (token->children[i]->type == Token_Type::EOL) {
                    saw_non_eol = false;
                    token->children.erase(token->children.begin() + i);
                }
                else {
                    if (saw_non_eol) {
                        throw Compiler_Exception("Two statements together without a separator.  Use return or semicolon",
                                token->children[i]->start_pos, token->children[i]->end_pos);
                    }
                    else {
                        saw_non_eol = true;
                    }

                    ++i;
                }
            }
        }
        break;
        default : {

        }
    }
    for (unsigned int i = 0; i < token->children.size(); ++i) {
        analyze_strays(token->children[i]);
    }
}

void Type_Analyzer::analyze_type_blocks(Program *program, Token *token, Scope **scope) {
    Scope *prev = *scope;
    bool block_dive = false;

    switch (token->type) {
        case (Token_Type::NAMESPACE) : {
            *scope = find_or_create_namespace(program, token->children[1]);
            (*scope)->owner = token;
        }
        break;

        case (Token_Type::ENUM_DEF) : {
            if ((*scope)->local_types.find(token->children[1]->contents) != (*scope)->local_types.end()) {
                Type_Def *dupe = program->types[(*scope)->local_types[token->children[1]->contents]];
                std::ostringstream msg;
                msg << "Duplicate definitions for type (see also: line: " << dupe->token->start_pos.line
                    << " of " << dupe->token->start_pos.filename << ")";
                throw Compiler_Exception(msg.str(), token->children[1]->start_pos, token->children[1]->end_pos);
            }
            else {
                Type_Def *td = new Type_Def();
                td->token = token;
                token->scope = *scope;

                program->types.push_back(td);
                token->definition_number = program->types.size() - 1;
                (*scope)->local_types[td->token->children[1]->contents] = program->types.size() - 1;
                program->build_internal_func(token->definition_number, token->definition_number, program->global->local_types["bool"], "==");

                Function_Def *print_fd = new Function_Def();
                print_fd->token = new Token(Token_Type::FUN_DEF);
                print_fd->is_internal = true;
                print_fd->arg_def_nums.push_back(token->definition_number);
                print_fd->return_type_def_num = program->global->local_types["void"];
                std::ostringstream print_name;
                print_name << "print__" << token->definition_number;
                program->funs.push_back(print_fd);
                program->global->local_funs[print_name.str()] = program->funs.size() - 1;
            }
        }
        break;
        case (Token_Type::ACTOR_DEF) :
        case (Token_Type::ISOLATED_ACTOR_DEF) :
        case (Token_Type::FEATURE_DEF) : {
            block_dive = true;
            Scope *new_scope = new Scope();
            new_scope->parent = *scope;

            if ((*scope)->local_types.find(token->contents) != (*scope)->local_types.end()) {
                Type_Def *dupe = program->types[(*scope)->local_types[token->contents]];
                std::ostringstream msg;
                msg << "Duplicate definitions for type (see also: line: " << dupe->token->start_pos.line
                    << " of " << dupe->token->start_pos.filename << ")";
                throw Compiler_Exception(msg.str(), token->children[1]->start_pos, token->children[1]->end_pos);
            }
            else {
                Type_Def *td = new Type_Def();
                td->token = token;
                new_scope->owner = token; //todo: This may not be the best way to do a reverse lookup
                token->scope = new_scope;
                token->children[2]->scope = new_scope;

                if (token->type == Token_Type::ISOLATED_ACTOR_DEF) {
                    td->is_isolated_actor = true;
                }

                program->types.push_back(td);
                token->definition_number = program->types.size() - 1;
                (*scope)->local_types[td->token->contents] = program->types.size() - 1;

                //Add a check for null method for each type
                Function_Def *null_check = new Function_Def(true);
                null_check->return_type_def_num = program->global->local_types["bool"];
                null_check->is_port_of_exit = false;
                null_check->token = new Token(Token_Type::FUN_DEF);

                program->funs.push_back(null_check);
                new_scope->local_funs["is_null"] = program->funs.size() - 1;

                //Also add "exists", the compliment of is_null, and easier to type
                Function_Def *exists_check = new Function_Def(true);
                exists_check->return_type_def_num = program->global->local_types["bool"];
                exists_check->is_port_of_exit = false;
                exists_check->token = new Token(Token_Type::FUN_DEF);

                program->funs.push_back(exists_check);
                new_scope->local_funs["exists"] = program->funs.size() - 1;

            }
            *scope = new_scope;
        }
        break;
        default: {

        }
    }
    for (unsigned int i = 0; i < token->children.size(); ++i) {
        analyze_type_blocks(program, token->children[i], scope);
    }

    if (block_dive) {
        *scope = prev;
    }
}

void Type_Analyzer::analyze_fun_blocks(Program *program, Token *token, Scope **scope) {
    Scope *prev = *scope;
    bool block_dive = false;

    switch (token->type) {
        case (Token_Type::NAMESPACE) : {
            *scope = find_or_create_namespace(program, token->children[1]);
        }
        break;
        case (Token_Type::IF_BLOCK) :
        case (Token_Type::ELSEIF_BLOCK) :
        case (Token_Type::ELSE_BLOCK) :
        case (Token_Type::WHILE_BLOCK) : {
            block_dive = true;
            Scope *new_scope = new Scope();
            new_scope->parent = *scope;
            new_scope->owner = token;
            token->scope = new_scope;

            *scope = new_scope;
        }
        break;

        case (Token_Type::ENUM_DEF) :
            break;
        case (Token_Type::ACTOR_DEF) :
        case (Token_Type::ISOLATED_ACTOR_DEF) :
        case (Token_Type::FEATURE_DEF) : {
            block_dive = true;
            *scope = token->scope;
        }
        break;
        case (Token_Type::LIBRARY_EXTERN_BLOCK) : {
            if (std::find(program->libs.begin(), program->libs.end(), token->children[1]->contents) == program->libs.end()) {
                program->libs.push_back(token->children[1]->contents);
            }
        }
        break;
        case (Token_Type::FUN_DEF) :
        case (Token_Type::EXTERN_FUN_DEF) :
        case (Token_Type::ACTION_DEF) : {
            block_dive = true;
            Scope *new_scope = new Scope();
            new_scope->parent = *scope;

            Scope *new_scope_block = new Scope();
            new_scope_block->parent = new_scope;
            new_scope_block->owner = token;

            Function_Def *fd = new Function_Def();
            new_scope->owner = token;
            fd->token = token;
            token->scope = new_scope;
            if (token->type == Token_Type::ACTION_DEF) {
                fd->is_port_of_exit = true;
                if (token->contents == "main") {
                    fd->is_used = true;
                }
            }

            if (fd->is_constructor == true) {
                fd->is_used = true;
            }

            if ((token->contents == "&&") || (token->contents == "||")) {
                throw Compiler_Exception("Operators '&&' and '||' can not be overloaded", token->children[1]->start_pos,
                        token->children[1]->end_pos);
            }

            //Build function def using the prototype
            std::string full_fn_name = build_function_def(program, token, new_scope, fd);
            if (token->type == Token_Type::EXTERN_FUN_DEF) {
                fd->external_name = token->contents;
            }
            else {
                token->children[2]->scope = new_scope_block;
            }

            if ((*scope)->local_funs.find(full_fn_name) != (*scope)->local_funs.end()) {
                Function_Def *dupe = program->funs[(*scope)->local_funs[full_fn_name]];
                std::ostringstream msg;
                if (dupe->token != NULL) {
                    msg << "Duplicate definitions for function/action (see also: line: " << dupe->token->start_pos.line
                        << " of " << dupe->token->start_pos.filename << ")";
                }
                else {
                    msg << "Duplicate definitions for built-in \"" << token->contents << "\"";
                }
                throw Compiler_Exception(msg.str(), token->children[1]->start_pos, token->children[1]->end_pos);
            }
            else {
                program->funs.push_back(fd);
                (*scope)->local_funs[full_fn_name] = program->funs.size() - 1;
                token->definition_number = program->funs.size() - 1;
            }
            *scope = new_scope_block;
        }
        break;

        default: {

        }
    }
    for (unsigned int i = 0; i < token->children.size(); ++i) {
        analyze_fun_blocks(program, token->children[i], scope);
    }

    if (block_dive) {
        *scope = prev;
    }
}

void Type_Analyzer::add_implied_constructors(Program *program) {

    for (unsigned int i = program->global->local_types["object"] + 1; i < program->types.size(); ++i) {
        Type_Def *td = program->types[i];
        if ((td->token->type == Token_Type::FEATURE_DEF) || (td->token->type == Token_Type::ACTOR_DEF) || (td->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
            //Check for any constructor
            bool found_constructor = false;
            for (std::map<std::string, unsigned int>::iterator iter = td->token->scope->local_funs.begin(),
                    end = td->token->scope->local_funs.end(); iter != end; ++iter) {

                std::ostringstream match;
                match << td->token->contents << "__";

                if ((iter->first == td->token->contents) || (iter->first.find(match.str()) == 0)) {
                    Function_Def *fd = program->funs[iter->second];
                    fd->is_constructor = true;
                    found_constructor = true;
                }
            }
            if (!found_constructor) {
                Function_Def *new_c = new Function_Def();
                new_c->token = new Token(Token_Type::FUN_DEF);
                new_c->token->contents = td->token->contents;
                new_c->token->scope = new Scope();
                new_c->token->scope->parent = td->token->scope;
                new_c->token->scope->owner = new_c->token;
                new_c->return_type_def_num = program->global->local_types["void"];
                new_c->is_internal = true;
                new_c->is_constructor = true;

                program->funs.push_back(new_c);

                td->token->scope->local_funs[td->token->contents] = program->funs.size() - 1;
            }

            //Check for delete method
            if (td->token->scope->local_funs.find("delete") == td->token->scope->local_funs.end()) {
                Function_Def *delete_me = new Function_Def(true);
                delete_me->return_type_def_num = program->global->local_types["void"];
                delete_me->is_port_of_exit = false;
                delete_me->is_internal = true;
                delete_me->token = new Token(Token_Type::FUN_DEF);

                program->funs.push_back(delete_me);
                td->token->scope->local_funs["delete"] = program->funs.size() - 1;
            }
        }
        if (td->token->type == Token_Type::FEATURE_DEF) {

            //Check for copy call
            std::ostringstream match;
            match << "copy__" << i;

            if (td->token->scope->local_funs.find(match.str()) == td->token->scope->local_funs.end()) {
                Function_Def *copy_me = new Function_Def(true);
                copy_me->return_type_def_num = program->global->local_types["void"];
                copy_me->arg_def_nums.push_back(i);
                copy_me->is_port_of_exit = false;
                copy_me->is_internal = true;
                copy_me->is_port_of_entry = true;
                copy_me->token = new Token(Token_Type::FUN_DEF);

                program->funs.push_back(copy_me);
                td->token->scope->local_funs[match.str()] = program->funs.size() - 1;
            }
        }
    }
}

void Type_Analyzer::analyze_var_type_and_scope(Program *program, Token *token, Scope *scope) {
    if ((token->type == Token_Type::SYMBOL) && (token->contents == ":")) {
        //We found a variable decl
        if (token->children.size() != 2) {
            throw Compiler_Exception("Internal parsing error", token->start_pos, token->end_pos);
        }
        else if (token->children[0]->type != Token_Type::ID) {
            throw Compiler_Exception("Expected variable name", token->children[0]->start_pos, token->children[0]->end_pos);
        }
        else {
            unsigned int var_def_num = build_var_def(program, token, scope);
            token->definition_number = var_def_num;

            if (scope->local_vars.find(token->children[0]->contents) != scope->local_vars.end()) {
                Token *dupe = program->vars[scope->local_vars[token->children[0]->contents]]->token;
                std::ostringstream msg;
                msg << "Redefined variable '" << token->children[0]->contents << "' (see also line " << dupe->start_pos.line
                    << " of " << dupe->start_pos.filename << ")";

                throw Compiler_Exception(msg.str(), token->children[0]->start_pos, token->children[0]->end_pos);
            }
            else {
                //std::cout << "Added: " << token->children[0]->contents << " to " << scope << std::endl;
                scope->local_vars[token->children[0]->contents] = program->vars.size() - 1;
                token->children[0]->type = Token_Type::VAR_CALL;
            }
        }
    }
    else if (token->type == Token_Type::QUICK_VAR_DECL) {
        int implied_type = program->global->local_types["var"];

        Var_Def *vd = new Var_Def();
        vd->token = token;
        vd->type_def_num = implied_type;
        vd->usage_start = token->start_pos;
        vd->usage_end = token->end_pos;
        program->vars.push_back(vd);

        unsigned int var_def_num = program->vars.size() - 1;

        token->definition_number = var_def_num;
        token->type_def_num = implied_type;
        std::string var_name = token->contents.substr(1, token->contents.length()-1);

        if (scope->local_vars.find(var_name) != scope->local_vars.end()) {
            Token *dupe = program->vars[scope->local_vars[var_name]]->token;
            std::ostringstream msg;
            msg << "Redefined variable '" << token->contents << "' (see also line " << dupe->start_pos.line
                << " of " << dupe->start_pos.filename << ")";

            throw Compiler_Exception(msg.str(), token->start_pos, token->end_pos);
        }
        else {
            //std::cout << "Added: " << token->children[0]->contents << " to " << scope << std::endl;
            scope->local_vars[var_name] = program->vars.size() - 1;
            token->type = Token_Type::VAR_DECL;
        }
    }

    if (token->type == Token_Type::FOR_BLOCK) {
        //for blocks need their own vars for constraint management
        std::ostringstream name;
        name << "for__" << for_inner_var_id++;

        Var_Def *vd = new Var_Def();
        vd->token = token;
        vd->type_def_num = program->global->local_types["int"];
        vd->usage_start = token->start_pos;
        vd->usage_end = token->end_pos;
        program->vars.push_back(vd);

        unsigned int var_def_num = program->vars.size() - 1;
        token->definition_number = var_def_num;
        scope->local_vars[name.str()] = var_def_num;

        vd = new Var_Def();
        vd->token = token;
        vd->type_def_num = program->global->local_types["int"];
        vd->usage_start = token->start_pos;
        vd->usage_end = token->end_pos;
        program->vars.push_back(vd);

        var_def_num = program->vars.size() - 1;
        std::ostringstream name2;
        name2 << "for__" << for_inner_var_id++;
        scope->local_vars[name2.str()] = var_def_num;

    }

    if ((token->type == Token_Type::FUN_DEF) || (token->type == Token_Type::ACTION_DEF) ||
            (token->type == Token_Type::ACTOR_DEF) || (token->type == Token_Type::FEATURE_DEF) ||
            (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {

        //only allow the variable scope to change at specific times
        scope = token->children[2]->scope;

        analyze_var_type_and_scope(program, token->children[2], scope);

        if ((token->type == Token_Type::ACTOR_DEF) || (token->type == Token_Type::ISOLATED_ACTOR_DEF) ||
                (token->type == Token_Type::FEATURE_DEF)) {
            //Check to make sure they don't have any var type variables as properties
            int var_type_def_num = program->global->local_types["var"];
            for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(), end = scope->local_vars.end();
                iter != end; ++iter) {

                Var_Def *vd = program->vars[iter->second];

                if (vd->type_def_num == var_type_def_num) {
                    throw Compiler_Exception("Implied type not allowed in actor/feature definition",
                            vd->token->start_pos, vd->token->end_pos);
                }
            }
        }

        return;
    }
    else if ((token->type == Token_Type::EXTERN_FUN_DEF) || (token->type == Token_Type::ENUM_DEF)) {
        return;
    }

    if (token->type == Token_Type::RETURN_CALL) {
        //todo What is this here for?
        if (token->children.size() > 1) {
            analyze_var_type_and_scope(program, token->children[1], scope);
        }
    }
    else {
        //for (unsigned int i = 0; i < token->children.size(); ++i) {
        unsigned int i = 0;
        while (i < token->children.size()) {
            analyze_var_type_and_scope(program, token->children[i], scope);
            ++i;
        }
    }

}

void Type_Analyzer::analyze_stacked_fun_call(Program *program, Token *token, Scope *scope) {
    unsigned int i = 0;
    while (i < token->children.size()) {
        Token *child = token->children[i];
        if (child->contents == "<<") {
            //unstack the stacked function call

            //first, find the function call, which should be the left-most token
            Token *fun_token = child;
            std::vector<Token*> arg_stack;
            while (fun_token->contents == "<<") {
                arg_stack.push_back(new Token(*(fun_token->children[1])));
                fun_token = fun_token->children[0];
            }

            while (arg_stack.size()) {
                Token *fun_call = new Token(Token_Type::FUN_CALL, arg_stack.back()->start_pos, arg_stack.back()->end_pos);
                if (fun_token->contents==".") {
                    fun_call->children.push_back(new Token(*(fun_token->children[1])));
                    fun_call->children.push_back(arg_stack.back());

                    Token *dot_call = new Token(Token_Type::SYMBOL, arg_stack.back()->start_pos, arg_stack.back()->end_pos);
                    dot_call->contents = ".";
                    dot_call->children.push_back(new Token(*(fun_token->children[0])));
                    dot_call->children.push_back(fun_call);
                    fun_call = dot_call;
                }
                else {
                    fun_call->children.push_back(new Token(*fun_token));
                    fun_call->children.push_back(arg_stack.back());
                }
                arg_stack.pop_back();
                token->children.insert(token->children.begin() + i, fun_call);
                ++i;
            }
            token->children.erase(token->children.begin() + i);

            //todo: this probably leaks children
            delete(fun_token);
            //++i;
        }
        else {
            analyze_stacked_fun_call(program, child, scope);
            ++i;
        }
    }
}

int look_for_fun_call(Program *program, std::string fullname, Scope *fun_scope) {
    Scope *scope = fun_scope;
    while (scope != NULL) {
        if (scope->local_funs.find(fullname) != scope->local_funs.end()) {
            return scope->local_funs[fullname];
        }
        scope = scope->parent;
    }
    return -1;
}

//This isn't used yet.
int clone_function_with_typeset(Program *program, Function_Def *fd, Scope *scope, std::vector<unsigned int> &types) {
    //first, clone the function
    Function_Def *clone_fd = new Function_Def(*fd);
    clone_fd->token = new Token(*(fd->token));

    //todo: this may not be sufficient
    clone_fd->token->children[2]->scope = clone_fd->token->scope;

    //then, clone the variables... all of them (just in case)
    //the function parms:
    /*
    for (unsigned int i = 0; i < fd->arg_def_nums.size(); ++i) {
        Var_Def *vd = program->vars[]
    }
    */

    return 0;
}

int build_or_find_fun_variant(Program *program, Token *token, Scope *fun_scope) {
    //make a list of functions named the same, with the same number of parameters:

    std::vector<unsigned int> call_parms;
    Token *child = token->children[1];
    while (child->contents == ",") {
        call_parms.insert(call_parms.begin(), child->children[1]->type_def_num);
        child = child->children[0];
    }
    call_parms.insert(call_parms.begin(), child->type_def_num);

    std::vector<Function_Def *> close_matches;
    std::vector<Scope *> corresponding_scopes;
    Scope *scope = fun_scope;

    while (scope != NULL) {
        for (std::map<std::string, unsigned int>::iterator iter = scope->local_funs.begin(), end = scope->local_funs.end(); iter != end; ++iter) {
            Function_Def *fd = program->funs[iter->second];
            if (fd->token != NULL) {
                if ((fd->token->contents == token->children[0]->contents) && (fd->arg_def_nums.size() == call_parms.size())) {
                    close_matches.push_back(fd);
                    corresponding_scopes.push_back(scope);
                }
            }
        }
        scope = scope->parent;
    }

    if (close_matches.size() == 0) {
        throw Compiler_Exception("No functions named '" + token->children[0]->contents + "' available", token->start_pos, token->end_pos);
    }
    else {
        //now we should have an array we can match against
        unsigned int implied_type = program->global->local_types["var"];

        for (long long int l = 1, end = (1 << call_parms.size()); l != end; ++l) {
            for (unsigned int match = 0; match < close_matches.size(); ++match) {
                bool found_match = true;
                Function_Def *fd_match = close_matches[match];
                for (unsigned int arg = 0; arg < call_parms.size(); ++arg) {
                    Var_Def *var_arg = program->vars[fd_match->arg_def_nums[arg]];

                    if ( (l | (1 << arg)) != 0) {
                        if (var_arg->type_def_num != (int)implied_type) {
                            found_match = false;
                            break;
                        }
                    }
                    else {
                        if (var_arg->type_def_num != (int)call_parms[arg]) {
                            found_match = false;
                            break;
                        }
                    }
                }

                if (found_match) {
                    std::cout << "Found match! " << close_matches[match] << std::endl;
                }
            }
        }
    }

    return -1;
}



void Type_Analyzer::check_fun_call(Program *program, Token *token, Scope *fun_scope, Scope *parm_scope) {
    for (unsigned int i = 1; i < token->children.size(); ++i) {
        analyze_token_types(program, token->children[i], parm_scope);
    }

    std::string fullname = build_function_name(program, token, fun_scope);

    int def_num = look_for_fun_call(program, fullname, fun_scope);

    if (def_num != -1) {
        token->definition_number = def_num;
        token->type_def_num = program->funs[token->definition_number]->return_type_def_num;

        Function_Def *fd = program->funs[def_num];
        fd->is_used = true;
        return;
    }

    /*
    def_num = build_or_find_fun_variant(program, token, fun_scope);

    if (def_num != -1) {
        token->definition_number = def_num;
        token->type_def_num = program->funs[token->definition_number]->return_type_def_num;

        Function_Def *fd = program->funs[def_num];
        fd->is_used = true;
        return;
    }
    */

    throw Compiler_Exception("Can not find usage of '" + fullname.substr(0, fullname.find("__")) + "'",
            token->start_pos, token->end_pos);

}

void Type_Analyzer::analyze_token_types(Program *program, Token *token, Scope *scope) {
    if  (token->scope != NULL) {
        scope = token->scope;
    }

    if ((token->type == Token_Type::FUN_DEF) || (token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::ACTOR_DEF) ||
            (token->type == Token_Type::FEATURE_DEF) || (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {

        analyze_token_types(program, token->children[2], scope);
        return;
    }
    else if ((token->type == Token_Type::EXTERN_FUN_DEF) || (token->type == Token_Type::ENUM_DEF)) {
        return;
    }

    if (token->children.size() > 0) {
        if (token->contents == ".") {
            if (token->children[1]->contents != ".") {
                if (token->children[1]->type == Token_Type::FUN_CALL) {
                    Scope *ns_scope = find_namespace(program, token->children[0], false);

                    if (ns_scope != NULL) {
                        check_fun_call(program, token->children[1], ns_scope, scope);
                        *token = *token->children[1];
                        return;
                    }
                    else {
                        analyze_token_types(program, token->children[0], scope);
                        Type_Def *dot_type = program->types[token->children[0]->type_def_num];
                        Scope *dot_scope = dot_type->token->scope;
                        if (dot_scope != NULL) {
                            check_fun_call(program, token->children[1], dot_scope, scope);
                        }
                        else {
                            check_fun_call(program, token->children[1], scope, scope);
                        }

                        if (((dot_type->token->type == Token_Type::ACTOR_DEF) || (dot_type->token->type == Token_Type::ISOLATED_ACTOR_DEF))
                                && (token->children[0]->type != Token_Type::THIS)) {
                            throw Compiler_Exception("Actor methods can not be accessed directly, use actions instead",
                                    token->children[1]->start_pos, token->children[1]->end_pos);
                        }

                        token->contents = token->children[1]->contents;
                        token->type = Token_Type::METHOD_CALL;
                        token->type_def_num = token->children[1]->type_def_num;
                    }
                    Function_Def *fd = program->funs[token->children[1]->definition_number];
                    if ((fd->token->type != Token_Type::FUN_DEF) && (fd->token->type != Token_Type::METHOD_CALL) && (fd->token->type != Token_Type::EXTERN_FUN_DEF)) {
                        throw Compiler_Exception("Expected method", token->children[1]->start_pos,
                                token->children[1]->end_pos);
                    }

                    return;
                }
                else if (token->children[1]->type == Token_Type::ARRAY_CALL) {
                    // this part of the tree is coming to us with the dot on top, but instead should have the array call on top
                    Token *prev_lhs = token->children[0];
                    Token *prev_rhs = token->children[1]->children[0];

                    Token *new_dot = new Token(Token_Type::SYMBOL);
                    new_dot->contents = ".";
                    new_dot->start_pos = prev_lhs->start_pos;
                    new_dot->end_pos = prev_rhs->end_pos;
                    new_dot->children.push_back(prev_lhs);
                    new_dot->children.push_back(prev_rhs);

                    *token = *token->children[1];
                    token->children[0] = new_dot;

                    //Since we're all fixed, reanalyze ourselves
                    analyze_token_types(program, token, scope);
                    return;
                }
                else {
                    Scope *ns_scope = find_namespace(program, token->children[0], false);

                    if (ns_scope != NULL) {
                        analyze_token_types(program, token->children[1], ns_scope);
                        *token = *token->children[1];
                        return;
                    }
                    analyze_token_types(program, token->children[0], scope);

                    if (token->children[0]->type == Token_Type::ENUM_CALL) {
                        //We're referencing the enum
                        Token *enum_lookup = program->types[token->children[0]->definition_number]->token;
                        for (unsigned int j = 2; j < enum_lookup->children.size(); ++j) {
                            if (enum_lookup->children[j]->contents == token->children[1]->contents) {
                                token->definition_number = j-2;
                                token->type_def_num = token->children[0]->definition_number;
                                token->type = Token_Type::REFERENCE_ENUM;
                                return;
                            }
                        }
                        throw Compiler_Exception("Unknown enumerated value", token->start_pos, token->end_pos);
                    }
                    /*
                    else {
                        std::cout << "TYPE: " << token->children[0]->type << std::endl;
                    }
                    */


                    Scope *orig = scope;
                    while (scope != NULL) {
                        //Feature referencing
                        if (scope->local_types.find(token->children[1]->contents) != scope->local_types.end()) {
                            analyze_token_types(program, token->children[0], orig);
                            token->definition_number = -1;
                            token->type_def_num = scope->local_types[token->children[1]->contents];
                            token->type = Token_Type::REFERENCE_FEATURE;
                            return;
                        }
                        scope = scope->parent;
                    }
                    scope = orig;

                    Type_Def *dot_type = program->types[token->children[0]->type_def_num];
                    Scope *dot_scope = dot_type->token->scope;

                    if (dot_scope != NULL) {
                        analyze_token_types(program, token->children[1], dot_scope);
                    }
                    else {
                        analyze_token_types(program, token->children[1], scope);
                    }
                }

                token->type_def_num = token->children[1]->type_def_num;
                if ((token->children[1]->type == Token_Type::ID) && (token->children[1]->definition_number == -1)) {
                    //This is a referenced feature, special case
                    token->type = Token_Type::REFERENCE_FEATURE;
                }
                else if (token->children[1]->type == Token_Type::VAR_CALL) {
                    //analyze_token_types(program, token->children[0], scope);
                    Type_Def *dot_type = program->types[token->children[0]->type_def_num];
                    //Scope *dot_scope = dot_type->token->scope;

                    if (((dot_type->token->type == Token_Type::ACTOR_DEF) || (dot_type->token->type == Token_Type::ACTOR_DEF)) && (token->children[0]->type != Token_Type::THIS)) {
                        throw Compiler_Exception("Actor attributes can not be accessed directly, use actions instead",
                                token->children[1]->start_pos, token->children[1]->end_pos);
                    }

                    token->type = Token_Type::ATTRIBUTE_CALL;
                }
            }
            else {
                analyze_token_types(program, token->children[0], scope);
                unsigned int type_def_num = find_type(program, token->children[1], scope);
                token->type = Token_Type::REFERENCE_FEATURE;
                token->type_def_num = type_def_num;
            }
            return;
        }
        else if (token->contents == "::") {
            analyze_token_types(program, token->children[0], scope);
            Type_Def *dot_type = program->types[token->children[0]->type_def_num];
            Scope *dot_scope = dot_type->token->scope;
            //analyze_token_types(program, token->children[1], dot_scope);
            check_fun_call(program, token->children[1], dot_scope, scope);

            Function_Def *fd = program->funs[token->children[1]->definition_number];

            if (token->children[1]->type != Token_Type::FUN_CALL) {
                throw Compiler_Exception("Expected action call", token->children[1]->start_pos, token->children[1]->end_pos);
            }
            else if (fd->token->type != Token_Type::ACTION_DEF) {
                throw Compiler_Exception("Expected action not method call", token->children[1]->start_pos,
                        token->children[1]->end_pos);
            }
            else {
                token->type = Token_Type::ACTION_CALL;
                token->contents = token->children[1]->contents;
                token->type_def_num = program->global->local_types["error"];
            }

            token->type_def_num = program->global->local_types["error"];
            return;
        }
        else if (token->contents == ",") {
            analyze_token_types(program, token->children[0], scope);
            analyze_token_types(program, token->children[1], scope);
            return;
        }
        else if (token->contents == "+") {
            int string_id = program->global->local_types["string"];
            analyze_token_types(program, token->children[0], scope);
            analyze_token_types(program, token->children[1], scope);

            if ((token->children[0]->type_def_num == string_id) || (token->children[1]->type_def_num == string_id)) {
                token->type = Token_Type::CONCATENATE;
                token->type_def_num = string_id;
                return;
            }
        }
        else if (token->contents == "++") {
            analyze_token_types(program, token->children[0], scope);
            analyze_token_types(program, token->children[1], scope);

            token->type = Token_Type::CONCATENATE_ARRAY;

            if (token->children[0]->type_def_num != token->children[1]->type_def_num) {
                throw Compiler_Exception("Mismatched arrays in concatenation", token->start_pos, token->end_pos);
            }
            if (token->children[0]->type_def_num >= 0) {
                Type_Def *td = program->types[token->children[0]->type_def_num];
                if (td->container != Container_Type::ARRAY) {
                    throw Compiler_Exception("Array concatenation operator used on non-array", token->start_pos, token->end_pos);
                }
            }
            token->type_def_num = token->children[0]->type_def_num;
            return;
        }
        else if (token->contents == ":") {
            //analyze_token_types(token->children[0], program, scope);
            token->type_def_num = program->vars[token->definition_number]->type_def_num;
            token->type = Token_Type::VAR_DECL;
            token->children[0]->type_def_num = token->type_def_num;
            token->children[0]->definition_number = token->definition_number;
            return;
        }
        else if (token->type == Token_Type::NEW_ALLOC) {
            //analyze_token_types(program, token->children[1], scope);
            find_constructor(program, token->children[1], scope);
            token->type_def_num = token->children[1]->type_def_num;
            return;
        }
        else if (token->type == Token_Type::SPAWN_ACTOR) {
            //analyze_token_types(program, token->children[1], scope);
            find_constructor(program, token->children[1], scope);
            token->type_def_num = token->children[1]->type_def_num;
            return;
        }
        else if (token->type == Token_Type::NAMESPACE) {
            return;
        }
        else if (token->type == Token_Type::LIBRARY_EXTERN_BLOCK) {
            analyze_token_types(program, token->children[2], scope);
            return;
        }
        else if (token->type == Token_Type::RETURN_CALL) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }

            if (token->children.size() == 1) {
                token->type_def_num = program->global->local_types["void"];
            }
            else {
                token->type_def_num = token->children[1]->type_def_num;
            }

            return;
        }
        else if (token->type == Token_Type::USE_CALL) {
            return;
        }
        else if (token->type == Token_Type::IF_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }

            if (token->children[1]->type_def_num != (signed)program->global->local_types["bool"]) {
                throw Compiler_Exception("If block expects boolean expression", token->start_pos, token->end_pos);
            }
        }
        else if (token->type == Token_Type::ELSEIF_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }

            if (token->children[1]->type_def_num != (signed)program->global->local_types["bool"]) {
                throw Compiler_Exception("Elseif block expects boolean expression", token->start_pos, token->end_pos);
            }
        }
        else if (token->type == Token_Type::ELSE_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }
        }
        else if (token->type == Token_Type::TRY_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }
        }
        else if (token->type == Token_Type::CATCH_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }
        }
        else if (token->type == Token_Type::WHILE_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }

            if (token->children[1]->type_def_num != (signed)program->global->local_types["bool"]) {
                throw Compiler_Exception("While block expects boolean expression", token->start_pos, token->end_pos);
            }
        }
        else if (token->type == Token_Type::FOR_BLOCK) {
            int int_type_def_num = program->global->local_types["int"];
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }
            if (token->children[1]->type_def_num != int_type_def_num) {
                throw Compiler_Exception("For loops only accept integer increments", token->children[1]->start_pos,
                        token->children[1]->end_pos);
            }
            if (token->children[2]->type_def_num != int_type_def_num) {
                throw Compiler_Exception("For loops only accept integer increments", token->children[2]->start_pos,
                        token->children[2]->end_pos);
            }
            if (token->children[1]->contents != "=") {
                throw Compiler_Exception("Expected equation in first part of for loop", token->children[1]->start_pos,
                        token->children[1]->end_pos);
            }
            /*
            //This shouldn't matter since it's being extracted, right?
            if ((token->children[2]->type == Token_Type::FUN_CALL) || (token->children[2]->type == Token_Type::METHOD_CALL)) {
                throw Compiler_Exception("For loops allow only simple values for end conditions", token->children[2]->start_pos,
                        token->children[2]->end_pos);
            }
            */
        }
        else if (token->type == Token_Type::FUN_CALL) {
            check_fun_call(program, token, scope, scope);
        }
        else if (token->type == Token_Type::ARRAY_CALL) {
            if ((token->children.size() != 2) || (token->children[0] == NULL) || (token->children[1] == NULL)) {
                throw Compiler_Exception("Mismatched types for array call", token->start_pos, token->end_pos);
            }

            analyze_token_types(program, token->children[0], scope);
            analyze_token_types(program, token->children[1], scope);

            if (!is_complex_type(program, token->children[0]->type_def_num)) {
                throw Compiler_Exception("Mismatched types for array call", token->start_pos, token->end_pos);
            }


            token->type_def_num = program->types[token->children[0]->type_def_num]->contained_type_def_nums[0];
            Type_Def *td = program->types[token->children[0]->type_def_num];
            if (td->container == Container_Type::ARRAY) {
                if (token->children[1]->type_def_num != (int)program->global->local_types["int"]) {
                    throw Compiler_Exception("Array calls expect 'int' as index", token->children[1]->start_pos, token->children[1]->end_pos);
                }
            }
            else if (td->container == Container_Type::DICT) {
                if (token->children[1]->type_def_num != (int)program->global->local_types["string"]) {
                    throw Compiler_Exception("Dictionary calls expect 'string' as key", token->children[1]->start_pos, token->children[1]->end_pos);
                }
            }
            else {
                throw Compiler_Exception("Unsupported type for array reference", token->children[0]->start_pos, token->children[0]->end_pos);
            }
            return;
        }
        else if (token->type == Token_Type::ARRAY_INIT) {
            int guarded_type_def = -1;
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
                if (i == 0) {
                    guarded_type_def = token->children[i]->type_def_num;
                }
                else {
                    if (token->children[i]->type_def_num != guarded_type_def) {
                        throw Compiler_Exception("Array initialization types don't match", token->children[i]->start_pos,
                                token->children[i]->end_pos);
                    }
                }
            }
            //todo: Add check to make sure array is all the same type
            token->type_def_num = find_type(program, token, scope);
            return;
        }
        else if (token->type == Token_Type::BLOCK) {
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }

            if (token->children.size() > 0) {
                token->type_def_num = token->children[token->children.size() - 1]->type_def_num;
            }
            return;
        }
        else if ((token->contents == "+=") || (token->contents == "-=")) {
            //do nothing
        }
        else {
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }
        }
    }
    if (token->type == Token_Type::ID) {
        Scope *prev_scope = scope;
        while (scope != NULL) {
            if (scope->local_types.find(token->contents) != scope->local_types.end()) {
                Type_Def *td = program->types[scope->local_types[token->contents]];
                if (td->token->type == Token_Type::ENUM_DEF) {
                    //We used definition number because we do not want the parser
                    //to pass using an enum without a lookup
                    token->definition_number = scope->local_types[token->contents];
                    token->type = Token_Type::ENUM_CALL;
                    return;
                }
            }
            scope = scope->parent;
        }

        scope = prev_scope;

        while (scope != NULL) {

            if (scope->local_vars.find(token->contents) != scope->local_vars.end()) {
                token->definition_number = scope->local_vars[token->contents];
                token->type_def_num = program->vars[token->definition_number]->type_def_num;
                token->type = Token_Type::VAR_CALL;
                return;
            }
            scope = scope->parent;
        }
        throw Compiler_Exception("Variable '" + token->contents + "' not found", token->start_pos, token->end_pos);
    }
    else if (token->type == Token_Type::EXCEPTION) {
        token->type_def_num = program->global->local_types["object"];
        return;
    }
    else if (token->type == Token_Type::THIS) {
        while (scope != NULL) {
            if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTOR_DEF)
                    || (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF) || (scope->owner->type == Token_Type::FEATURE_DEF))) {
                token->type_def_num = scope->owner->definition_number;
                return;
            }
            scope = scope->parent;
        }
        throw Compiler_Exception("'this' could not be matched to an actor or feature", token->start_pos, token->end_pos);
    }
    else if (token->type == Token_Type::FLOAT) {
        token->type_def_num = program->global->local_types["double"];
    }
    else if (token->type == Token_Type::INT) {
        token->type_def_num = program->global->local_types["int"];
    }
    else if (token->type == Token_Type::BOOL) {
        token->type_def_num = program->global->local_types["bool"];
    }
    else if (token->type == Token_Type::SYMBOL) {
        if (token->contents == "=") {
            //todo: This isn't exactly optimal
            if (token->children[0]->type_def_num == (signed)program->global->local_types["var"]) {
                token->children[0]->type_def_num = token->children[1]->type_def_num;

                if (token->children[0]->definition_number != -1) {
                    //Set the actual type of the implied type based on the witness of the rhs
                    Var_Def *vd = program->vars[token->children[0]->definition_number];
                    vd->type_def_num = token->children[1]->type_def_num;
                }

                token->type_def_num = token->children[0]->type_def_num;
                return;
            }
            else if (token->children[0]->type_def_num == token->children[1]->type_def_num) {
                token->type_def_num = token->children[0]->type_def_num;
                return;
            }
            else if (token->children[0]->type_def_num == (signed)program->global->local_types["object"]) {
                Type_Def *td = program->types[token->children[1]->type_def_num];
                if (td->token->type == Token_Type::FEATURE_DEF) {
                    token->type_def_num = token->children[0]->type_def_num;
                    return;
                }
                else {
                    throw Compiler_Exception("Mismatched types in equation", token->start_pos, token->end_pos);
                }
            }
            else {
                //std::cout << "ERROR: " << token->children[0]->type_def_num << " " << token->children[1]->type_def_num << std::endl;
                throw Compiler_Exception("Mismatched types in equation", token->start_pos, token->end_pos);
            }
        }
        else if (token->contents == "<+") {
            //Melding operator
            if (token->children[0]->type_def_num == (signed)program->global->local_types["var"]) {
                token->children[0]->type_def_num = (signed)program->global->local_types["object"];

                if (token->children[0]->definition_number != -1) {
                    //Set the actual type of the implied type based on the witness of the rhs
                    Var_Def *vd = program->vars[token->children[0]->definition_number];
                    vd->type_def_num = (signed)program->global->local_types["object"];
                }

                token->type_def_num = token->children[0]->type_def_num;
            }

            if ((token->children[0]->type_def_num < (signed)program->global->local_types["object"]) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ACTOR_DEF) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ISOLATED_ACTOR_DEF)){
                throw Compiler_Exception("Object expected on left hand side of meld operator", token->children[0]->start_pos,
                        token->children[0]->end_pos);
            }
            if ((token->children[1]->type_def_num < (signed)program->global->local_types["object"]) ||
                    (program->types[token->children[1]->type_def_num]->token->type == Token_Type::ACTOR_DEF) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
                throw Compiler_Exception("Object or feature expected on right hand side of meld operator", token->children[1]->start_pos,
                        token->children[1]->end_pos);
            }

            token->type_def_num = token->children[0]->type_def_num;
            return;
        }
        else if (token->contents == "+>") {
            //Extraction operator
            if ((token->children[0]->type_def_num < (signed)program->global->local_types["object"]) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ACTOR_DEF) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ISOLATED_ACTOR_DEF)){
                throw Compiler_Exception("Object expected on left hand side of extraction operator",
                        token->children[0]->start_pos, token->children[0]->end_pos);
            }
            if ((token->children[1]->type_def_num < (signed)program->global->local_types["object"]) ||
                    (program->types[token->children[1]->type_def_num]->token->type == Token_Type::ACTOR_DEF) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
                throw Compiler_Exception("Object or feature expected on right hand side of extraction operator",
                        token->children[1]->start_pos, token->children[1]->end_pos);
            }
            token->type_def_num = token->children[1]->type_def_num;
            return;
        }
        else if (token->contents == "+=") {
            Token *new_sign = new Token(Token_Type::SYMBOL);
            new_sign->contents = "+";
            new_sign->start_pos = token->start_pos;
            new_sign->end_pos = token->end_pos;

            Token *new_rhs = new Token(token->children[0]->type);
            *new_rhs = *(token->children[0]);
            new_sign->children.push_back(new_rhs);
            new_sign->children.push_back(token->children[1]);
            token->children[1] = new_sign;
            token->contents = "=";
            analyze_token_types(program, token, scope);
            return;
        }
        else if (token->contents == "-=") {
            Token *new_sign = new Token(Token_Type::SYMBOL);
            new_sign->contents = "-";
            new_sign->start_pos = token->start_pos;
            new_sign->end_pos = token->end_pos;

            Token *new_rhs = new Token(token->children[0]->type);
            *new_rhs = *(token->children[0]);
            new_sign->children.push_back(new_rhs);
            new_sign->children.push_back(token->children[1]);
            token->children[1] = new_sign;
            token->contents = "=";
            analyze_token_types(program, token, scope);
            return;
        }
        else {
            std::ostringstream name;
            name << token->contents << "__" << token->children[0]->type_def_num << "__" << token->children[1]->type_def_num;

            while (scope != NULL) {
                std::string fullname = name.str();
                if (scope->local_funs.find(fullname) != scope->local_funs.end()) {
                    Function_Def *fd = program->funs[scope->local_funs[fullname]];
                    fd->is_used = true;
                    token->type_def_num = program->funs[scope->local_funs[fullname]]->return_type_def_num;
                    token->definition_number = scope->local_funs[fullname];
                    return;
                }
                scope = scope->parent;
            }
        }

        int implied_type = program->global->local_types["var"];
        if (token->children[0]->type_def_num == implied_type) {
            throw Compiler_Exception("Implied type variable used without being set to a value", token->children[0]->start_pos, token->children[0]->end_pos);
        }
        if (token->children[1]->type_def_num == implied_type) {
            throw Compiler_Exception("Implied type variable used without being set to a value", token->children[1]->start_pos, token->children[1]->end_pos);
        }

        throw Compiler_Exception("Can not understand usage of '" + token->contents + "'", token->start_pos,
                token->end_pos);
    }
    else if (token->type == Token_Type::SINGLE_QUOTED_STRING) {
        token->type_def_num = program->global->local_types["char"];
    }
    else if (token->type == Token_Type::QUOTED_STRING) {
        token->type_def_num = program->global->local_types["string"];
    }
    else if (token->type == Token_Type::QUOTED_STRING_CONST) {
        token->type_def_num = program->global->local_types["string"];
    }
}

void Type_Analyzer::analyze_implied_this(Program *program, Token *token, Scope *scope) {
    if (token->scope != NULL) {
        scope = token->scope;
    }

    if (token->type == Token_Type::FUN_CALL) {
        for (unsigned int i = 1; i < token->children.size(); ++i) {
            analyze_implied_this(program, token->children[i], scope);
        }

        std::string fullname = build_function_name(program, token, scope);

        Scope *prev = scope;
        while (scope != NULL) {
            if (scope->local_funs.find(fullname) != scope->local_funs.end()) {
                if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTOR_DEF) ||
                        (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF) ||
                        (scope->owner->type == Token_Type::FEATURE_DEF))) {

                    Function_Def *fd = program->funs[scope->local_funs[fullname]];

                    Token *new_fun_call = new Token(Token_Type::FUN_CALL);
                    *new_fun_call = *token;
                    Token *this_ptr = new Token(Token_Type::THIS);

                    token->contents = new_fun_call->contents;
                    if (fd->token->type == Token_Type::ACTION_DEF) {
                        token->type = Token_Type::ACTION_CALL;
                        token->type_def_num = program->global->local_types["error"];
                    }
                    else {
                        token->type = Token_Type::METHOD_CALL;
                    }
                    /*
                    if (scope->owner->type == Token_Type::FEATURE_DEF) {
                        token->type = Token_Type::METHOD_CALL;
                    }
                    else if ((scope->owner->type == Token_Type::ACTOR_DEF)
                            || (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF)) {
                        token->type = Token_Type::ACTION_CALL;
                        token->type_def_num = program->global->local_types["error"];
                    }
                    */

                    token->children.clear();
                    token->children.push_back(this_ptr);
                    token->children.push_back(new_fun_call);
                }


            }
            scope = scope->parent;
        }
        scope = prev;
        //Removed because this does not work with namespaced functions
        //throw Compiler_Exception("Internal error involving implied 'this'", token->start_pos);

        return;
    }
    else if ((token->type == Token_Type::VAR_CALL) && (token->contents != "")) { // "" would mean an internal temp
        if (token->definition_number != -1) {

            while (scope != NULL) {
                if (scope->local_vars.find(token->contents) != scope->local_vars.end()) {
                    if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTOR_DEF) || (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF) ||
                            (scope->owner->type == Token_Type::FEATURE_DEF))) {

                        Token *new_attr_call = new Token(Token_Type::VAR_CALL);
                        *new_attr_call = *token;
                        Token *this_ptr = new Token(Token_Type::THIS);

                        token->contents = new_attr_call->contents;
                        token->type = Token_Type::ATTRIBUTE_CALL;

                        token->children.clear();
                        token->children.push_back(this_ptr);
                        token->children.push_back(new_attr_call);
                    }

                    return;
                }
                scope = scope->parent;
            }
            throw Compiler_Exception("Internal error involving implied 'this'", token->start_pos, token->end_pos);
        }
    }
    else if (token->type == Token_Type::ATTRIBUTE_CALL) {
        //only allow it to go left, since we know the rhs is correct already
        analyze_implied_this(program, token->children[0], scope);
    }
    else if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        for (unsigned int i = 2; i < token->children.size(); ++i) {
            analyze_implied_this(program, token->children[i], scope);
        }
    }
    else if ((token->type == Token_Type::METHOD_CALL) || (token->type == Token_Type::ACTION_CALL) || (token->contents == ".")) {
        analyze_implied_this(program, token->children[0], scope);

        for (unsigned int i = 1; i < token->children[1]->children.size(); ++i) {
            analyze_implied_this(program, token->children[1]->children[i], scope);
        }
    }
    else if ((token->type == Token_Type::EXTERN_FUN_DEF)) {
        //do nothing
    }
    else {
        for (unsigned int i = 0; i < token->children.size(); ++i) {
            analyze_implied_this(program, token->children[i], scope);
        }
    }
}

void Type_Analyzer::analyze_return_calls(Program *program, Token *token, unsigned int allowed_return_type) {
    if ((token->type == Token_Type::FUN_DEF) || (token->type == Token_Type::ACTION_DEF)) {
        allowed_return_type = program->funs[token->definition_number]->return_type_def_num;
    }
    else if (token->type == Token_Type::RETURN_CALL) {
        if (token->children.size() == 1) {
            if (allowed_return_type != program->global->local_types["void"]) {
                throw Compiler_Exception("Void return in non-void function", token->start_pos, token->end_pos);
            }
        }
        else {
            if (allowed_return_type != (unsigned)token->type_def_num) {
                throw Compiler_Exception("Return type does not match function return type", token->children[1]->start_pos,
                        token->children[1]->end_pos);
            }
        }
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        analyze_return_calls(program, token->children[i], allowed_return_type);
    }
}

bool Type_Analyzer::analyze_required_return_calls(Program *program, Token *token) {
    if (token->type == Token_Type::FUN_DEF) {
        if (program->funs[token->definition_number]->return_type_def_num != program->global->local_types["void"]) {
            bool found_return = false;
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                if (analyze_required_return_calls(program, token->children[i])) {
                    found_return = true;
                }
            }
            if (!found_return) {
                throw Compiler_Exception("Return call missing in function with return type", token->children[1]->children[1]->start_pos,
                        token->children[1]->children[1]->end_pos);
            }
        }
        return false;
    }
    else if (token->type == Token_Type::RETURN_CALL) {
        return true;
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        if (analyze_required_return_calls(program, token->children[i])) {
            return true;
        }
    }
    return false;
}


