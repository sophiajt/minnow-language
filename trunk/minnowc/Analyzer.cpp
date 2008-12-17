// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include "Common.hpp"
#include "Analyzer.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

Scope *Analyzer::find_or_create_namespace(Program *program, Token *ns) {
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
        throw Compiler_Exception("Use '.' for declaring a deep namespace", ns->start_pos);
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

Scope *Analyzer::find_namespace(Program *program, Token *ns, bool throw_exceptions) {
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
                throw Compiler_Exception("Namespace not found", ns->start_pos);
            }
            else {
                return NULL;
            }
        }
    }
    //If we get here, we should be at a '.' for a deep namespace
    if (ns->contents != ".") {
        if (throw_exceptions) {
            throw Compiler_Exception("Use '.' for specifying a deep namespace", ns->start_pos);
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
                throw Compiler_Exception("Namespace not found", ns->start_pos);
            }
            else {
                return NULL;
            }
        }
    }
}
unsigned int Analyzer::find_type(Program *program, Token *ns, Scope *scope) {
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
        throw Compiler_Exception("Can not find type '" + ns->contents + "'", ns->start_pos);
    }
    else {
        if (ns->type == Token_Type::ARRAY_CALL) {
            if (ns->children.size() != 2) {
                throw Compiler_Exception("Type incomplete", ns->start_pos);
            }
            else if (ns->children[0]->contents != "Array") {
                throw Compiler_Exception("Unsupported container type", ns->children[0]->start_pos);
            }
            else {
                std::ostringstream containername;
                find_type(program, ns->children[1], scope);

                containername << "Con___" << ns->children[1]->definition_number;
                if (program->global->local_types.find(containername.str()) != program->global->local_types.end()) {
                    ns->definition_number = program->global->local_types[containername.str()];
                    ns->type_def_num = ns->definition_number;
                    return ns->definition_number;
                }
                else {
                    //It doesn't exist yet, so let's create it
                    Type_Def *container = new Type_Def();
                    container->container = Container_Type::ARRAY;
                    container->contained_type_def_num = ns->children[1]->type_def_num;
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
        }
        else if (ns->contents == ".") {
            scope = find_namespace(program, ns->children[0], true);
            if (ns->contents != ".") {
                //todo: put this some place useful
                throw Compiler_Exception("Use '.' for specifying a namespaced type", ns->start_pos);
            }
            else {
                if (scope->local_types.find(ns->children[1]->contents) != scope->local_types.end()) {
                    ns->definition_number = scope->local_types[ns->children[1]->contents];
                    ns->type_def_num = ns->definition_number;
                    return ns->definition_number;
                }
                else {
                    throw Compiler_Exception("Can not find type '" + ns->children[1]->contents + "'", ns->start_pos);
                }
            }
        }
        else {
            throw Compiler_Exception("Can not find type", ns->start_pos);
        }
    }
    //return ret_val;
}

void Analyzer::find_constructor(Program *program, Token *ns, Scope *scope) {
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
            throw Compiler_Exception("Can not find constructor for '" + ns->children[0]->contents + "'", ns->start_pos);
        }
    }

    else {

        if (ns->type == Token_Type::ARRAY_CALL) {
            if (ns->children.size() != 2) {
                throw Compiler_Exception("Type incomplete", ns->start_pos);
            }
            else if (ns->children[0]->contents != "Array") {
                throw Compiler_Exception("Unsupported container type", ns->children[0]->start_pos);
            }
            else {
                std::ostringstream containername;
                find_type(program, ns->children[1], scope);

                containername << "Con___" << ns->children[1]->definition_number;
                if (program->global->local_types.find(containername.str()) != program->global->local_types.end()) {
                    ns->definition_number = program->global->local_types[containername.str()];
                    ns->type_def_num = ns->definition_number;
                    //return ns->definition_number;
                }
                else {
                    //It doesn't exist yet, so let's create it
                    Type_Def *container = new Type_Def();
                    container->container = Container_Type::ARRAY;
                    container->contained_type_def_num = ns->children[1]->type_def_num;
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
                    throw Compiler_Exception("Can not find constructor for '" + ns->children[1]->children[0]->contents + "'", ns->start_pos);
                }
            }
            else {
                throw Compiler_Exception("Missing constructor call", ns->start_pos);
            }
            //}
        }
        else {
            throw Compiler_Exception("Constructor not given", ns->start_pos);
        }
    }

    //return ret_val;
}

unsigned int Analyzer::build_var_def(Program *program, Token *token, Scope *scope) {
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

void Analyzer::build_function_args(Program *program, Token *token, Scope *scope, Function_Def *fd) {
    unsigned int var_def_num;

    if (token->contents == ",") {
        build_function_args(program, token->children[0], scope, fd);
        build_function_args(program, token->children[1], scope, fd);
    }
    else if (token->contents == ":") {
        //push the furthest lhs first
        if (token->children[1]->contents == "var") {
            //Workaround because functions are not allowed to have var types (yet?)
            throw Compiler_Exception("Implied types not allowed in function header", token->children[1]->start_pos);
        }
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

            throw Compiler_Exception(msg.str(), token->children[0]->start_pos);
        }
        scope->local_vars[token->children[0]->contents] = var_def_num;
    }
    else {
        for (unsigned int i = 0; i < token->children.size(); ++i) {
            build_function_args(program, token->children[i], scope, fd);
        }
    }
}

std::string Analyzer::build_function_arg_types(Program *program, Token *token, Scope *scope) {
    std::ostringstream output;
    if (token->contents == ",") {
        output << build_function_arg_types(program, token->children[0], scope);
        output << "__";
        output << build_function_arg_types(program, token->children[1], scope);
    }
    else {
        output << token->type_def_num;
    }
    return output.str();
}

std::string Analyzer::build_function_name(Program *program, Token *token, Scope *scope) {
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

std::string Analyzer::build_function_def(Program *program, Token *token, Scope *scope, Function_Def *fd) {
    std::ostringstream full_name;
    unsigned int return_type = program->global->local_types["void"];

    if ((token->type == Token_Type::ACTION_DEF) && (token->children[1]->contents == ":")) {
        throw Compiler_Exception("Actions do not have return types", token->start_pos);
    }

    if (token->children[1]->contents == ":") {
        //We have a return type
        if (token->children[1]->children[1]->contents == "var") {
            //Workaround because functions are not allowed to have var types (yet?)
            throw Compiler_Exception("Implied types are not allowed as function return types", token->children[1]->start_pos);
        }

        return_type = find_type(program, token->children[1]->children[1], scope);
        if (token->children[1]->children[0]->type == Token_Type::FUN_CALL) {
            build_function_args(program, token->children[1]->children[0], scope, fd);
        }
    }
    else if (token->children[1]->type == Token_Type::FUN_CALL) {
        build_function_args(program, token->children[1], scope, fd);
    }

    fd->return_type_def_num = return_type;

    full_name << token->contents;
    for (unsigned int i = 0; i < fd->arg_def_nums.size(); ++i) {
        full_name << "__" << program->vars[fd->arg_def_nums[i]]->type_def_num;
    }

    return full_name.str();
}

//todo: extend strays pass to catch stray tokens
void Analyzer::analyze_strays(Token *token) {
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
                                token->children[i]->start_pos);
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

void Analyzer::analyze_type_blocks(Program *program, Token *token, Scope **scope) {
    Scope *prev = *scope;
    bool block_dive = false;

    switch (token->type) {
        case (Token_Type::NAMESPACE) : {
            *scope = find_or_create_namespace(program, token->children[1]);
            (*scope)->owner = token;
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
                throw Compiler_Exception(msg.str(), token->start_pos);
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

void Analyzer::analyze_fun_blocks(Program *program, Token *token, Scope **scope) {
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

        case (Token_Type::ACTOR_DEF) :
        case (Token_Type::ISOLATED_ACTOR_DEF) :
        case (Token_Type::FEATURE_DEF) : {
            block_dive = true;
            *scope = token->scope;
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

            Function_Def *fd = new Function_Def();
            fd->token = token;
            new_scope->owner = token;
            token->scope = new_scope;
            if (token->type == Token_Type::ACTION_DEF) {
                fd->is_port_of_exit = true;
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
                msg << "Duplicate definitions for function/action (see also: line: " << dupe->token->start_pos.line
                    << " of " << dupe->token->start_pos.filename << ")";
                throw Compiler_Exception(msg.str(), token->start_pos);
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

void Analyzer::add_implied_constructors(Program *program) {

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
        }
    }
}

void Analyzer::analyze_var_type_and_scope(Program *program, Token *token, Scope *scope) {
    if ((token->type == Token_Type::SYMBOL) && (token->contents == ":")) {
        //We found a variable decl
        if (token->children.size() != 2) {
            throw Compiler_Exception("Internal parsing error", token->start_pos);
        }
        else if (token->children[0]->type != Token_Type::ID) {
            throw Compiler_Exception("Expected variable name", token->children[0]->start_pos);
        }
        else {
            unsigned int var_def_num = build_var_def(program, token, scope);
            token->definition_number = var_def_num;

            if (scope->local_vars.find(token->children[0]->contents) != scope->local_vars.end()) {
                Token *dupe = program->vars[scope->local_vars[token->children[0]->contents]]->token;
                std::ostringstream msg;
                msg << "Redefined variable '" << token->children[0]->contents << "' (see also line " << dupe->start_pos.line
                    << " of " << dupe->start_pos.filename << ")";

                throw Compiler_Exception(msg.str(), token->children[0]->start_pos);
            }
            else {
                //std::cout << "Added: " << token->children[0]->contents << " to " << scope << std::endl;
                scope->local_vars[token->children[0]->contents] = program->vars.size() - 1;
                token->children[0]->type = Token_Type::VAR_CALL;
            }
        }
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
            unsigned int var_type_def_num = program->global->local_types["var"];
            for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(), end = scope->local_vars.end();
                iter != end; ++iter) {

                Var_Def *vd = program->vars[iter->second];

                if (vd->type_def_num == var_type_def_num) {
                    throw Compiler_Exception("Implied type not allowed in actor/feature definition", vd->token->start_pos);
                }
            }
        }

        return;
    }
    else if (token->type == Token_Type::EXTERN_FUN_DEF) {
        return;
    }

    if (token->type == Token_Type::RETURN_CALL) {
        //todo What is this here for?
        if (token->children.size() > 1) {
            analyze_var_type_and_scope(program, token->children[1], scope);
        }
    }
    else {
        for (unsigned int i = 0; i < token->children.size(); ++i) {
            analyze_var_type_and_scope(program, token->children[i], scope);
        }
    }

}

void Analyzer::check_fun_call(Program *program, Token *token, Scope *fun_scope, Scope *parm_scope) {
    for (unsigned int i = 1; i < token->children.size(); ++i) {
        analyze_token_types(program, token->children[i], parm_scope);
    }

    std::string fullname = build_function_name(program, token, fun_scope);

    Scope *scope = fun_scope;
    while (scope != NULL) {
        if (scope->local_funs.find(fullname) != scope->local_funs.end()) {
            token->definition_number = scope->local_funs[fullname];
            token->type_def_num = program->funs[token->definition_number]->return_type_def_num;

            return;
        }
        scope = scope->parent;
    }
    throw Compiler_Exception("Can not find usage of '" + fullname.substr(0, fullname.find("__")) + "'", token->start_pos);

}

void Analyzer::analyze_token_types(Program *program, Token *token, Scope *scope) {
    if  (token->scope != NULL) {
        scope = token->scope;
    }

    if ((token->type == Token_Type::FUN_DEF) || (token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::ACTOR_DEF) ||
            (token->type == Token_Type::FEATURE_DEF) || (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {

        analyze_token_types(program, token->children[2], scope);
        return;
    }
    else if (token->type == Token_Type::EXTERN_FUN_DEF) {
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
                            throw Compiler_Exception("Actor methods can not be accessed directly, use actions instead", token->children[1]->start_pos);
                        }

                        token->contents = token->children[1]->contents;
                        token->type = Token_Type::METHOD_CALL;
                        token->type_def_num = token->children[1]->type_def_num;
                    }
                    Function_Def *fd = program->funs[token->children[1]->definition_number];
                    if ((fd->token->type != Token_Type::FUN_DEF) && (fd->token->type != Token_Type::METHOD_CALL) && (fd->token->type != Token_Type::EXTERN_FUN_DEF)) {
                        throw Compiler_Exception("Expected method", token->children[1]->start_pos);
                    }

                    return;
                }
                else {
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

                    analyze_token_types(program, token->children[0], scope);
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
                /*
                else if (token->children[1]->type == Token_Type::FUN_CALL) {
                    analyze_token_types(program, token->children[0], scope);
                    Type_Def *dot_type = program->types[token->children[0]->type_def_num];
                    Scope *dot_scope = dot_type->token->scope;

                    if ((dot_type->token->type == Token_Type::ACTOR_DEF) && (token->children[0]->type != Token_Type::THIS)) {
                        throw Compiler_Exception("Actor methods can not be accessed directly, use actions instead", token->children[1]->start_pos);
                    }

                    token->contents = token->children[1]->contents;
                    token->type = Token_Type::METHOD_CALL;
                }
                */
                else if (token->children[1]->type == Token_Type::VAR_CALL) {
                    analyze_token_types(program, token->children[0], scope);
                    Type_Def *dot_type = program->types[token->children[0]->type_def_num];
                    //Scope *dot_scope = dot_type->token->scope;

                    if (((dot_type->token->type == Token_Type::ACTOR_DEF) || (dot_type->token->type == Token_Type::ACTOR_DEF)) && (token->children[0]->type != Token_Type::THIS)) {
                        throw Compiler_Exception("Actor attributes can not be accessed directly, use actions instead", token->children[1]->start_pos);
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
                throw Compiler_Exception("Expected action call", token->children[1]->start_pos);
            }
            else if (fd->token->type != Token_Type::ACTION_DEF) {
                throw Compiler_Exception("Expected action not method call", token->children[1]->start_pos);
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
                throw Compiler_Exception("If block expects boolean expression", token->start_pos);
            }
        }
        else if (token->type == Token_Type::ELSEIF_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }

            if (token->children[1]->type_def_num != (signed)program->global->local_types["bool"]) {
                throw Compiler_Exception("Elseif block expects boolean expression", token->start_pos);
            }
        }
        else if (token->type == Token_Type::ELSE_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }
        }
        else if (token->type == Token_Type::WHILE_BLOCK) {
            for (unsigned int i = 1; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }

            if (token->children[1]->type_def_num != (signed)program->global->local_types["bool"]) {
                throw Compiler_Exception("While block expects boolean expression", token->start_pos);
            }
        }
        else if (token->type == Token_Type::FUN_CALL) {
            check_fun_call(program, token, scope, scope);
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
        else {
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                analyze_token_types(program, token->children[i], scope);
            }
        }
    }
    if (token->type == Token_Type::ID) {

        while (scope != NULL) {
            /*
            std::cout << "SCOPE: " << scope << " looking for: " << token->contents << std::endl;
            for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(), end = scope->local_vars.end(); iter != end; ++iter) {
                std::cout << " var: " << iter->first << " def: " << iter->second << std::endl;
            }
            */

            if (scope->local_vars.find(token->contents) != scope->local_vars.end()) {
                token->definition_number = scope->local_vars[token->contents];
                token->type_def_num = program->vars[token->definition_number]->type_def_num;
                token->type = Token_Type::VAR_CALL;
                return;
            }
            scope = scope->parent;
        }
        throw Compiler_Exception("Variable '" + token->contents + "' not found", token->start_pos);
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
        throw Compiler_Exception("'this' could not be matched to an actor or feature", token->start_pos);
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
                    throw Compiler_Exception("Mismatched types in equation", token->start_pos);
                }
            }
            else {
                //std::cout << "ERROR: " << token->children[0]->type_def_num << " " << token->children[1]->type_def_num << std::endl;

                throw Compiler_Exception("Mismatched types in equation", token->start_pos);
            }
        }
        else if (token->contents == "<+") {
            //Melding operator
            if ((token->children[0]->type_def_num < (signed)program->global->local_types["object"]) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ACTOR_DEF) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ISOLATED_ACTOR_DEF)){
                throw Compiler_Exception("Object expected on left hand side of meld operator", token->children[0]->start_pos);
            }
            if ((token->children[1]->type_def_num < (signed)program->global->local_types["object"]) ||
                    (program->types[token->children[1]->type_def_num]->token->type == Token_Type::ACTOR_DEF) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
                throw Compiler_Exception("Object or feature expected on right hand side of meld operator", token->children[1]->start_pos);
            }
            token->type_def_num = token->children[0]->type_def_num;
            return;
        }
        else if (token->contents == "+>") {
            //Extraction operator
            if ((token->children[0]->type_def_num < (signed)program->global->local_types["object"]) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ACTOR_DEF) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ISOLATED_ACTOR_DEF)){
                throw Compiler_Exception("Object expected on left hand side of extraction operator", token->children[0]->start_pos);
            }
            if ((token->children[1]->type_def_num < (signed)program->global->local_types["object"]) ||
                    (program->types[token->children[1]->type_def_num]->token->type == Token_Type::ACTOR_DEF) ||
                    (program->types[token->children[0]->type_def_num]->token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
                throw Compiler_Exception("Object or feature expected on right hand side of extraction operator", token->children[1]->start_pos);
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
                    token->type_def_num = program->funs[scope->local_funs[fullname]]->return_type_def_num;
                    return;
                }
                scope = scope->parent;
            }
        }
        throw Compiler_Exception("Can not understand usage of '" + token->contents + "'", token->start_pos);
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
    else if (token->type == Token_Type::ARRAY_CALL) {
        token->type_def_num = program->types[token->children[0]->type_def_num]->contained_type_def_num;
    }
}

void Analyzer::analyze_implied_this(Program *program, Token *token, Scope *scope) {
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

                    Token *new_fun_call = new Token(Token_Type::FUN_CALL);
                    *new_fun_call = *token;
                    Token *this_ptr = new Token(Token_Type::THIS);

                    token->contents = new_fun_call->contents;
                    if (scope->owner->type == Token_Type::FEATURE_DEF) {
                        token->type = Token_Type::METHOD_CALL;
                    }
                    else if ((scope->owner->type == Token_Type::ACTOR_DEF)
                            || (scope->owner->type == Token_Type::ISOLATED_ACTOR_DEF)) {
                        token->type = Token_Type::ACTION_CALL;
                        token->type_def_num = program->global->local_types["error"];
                    }

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
            throw Compiler_Exception("Internal error involving implied 'this'", token->start_pos);
        }
    }
    else if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        for (unsigned int i = 2; i < token->children.size(); ++i) {
            analyze_implied_this(program, token->children[i], scope);
        }
    }
    else if ((token->type == Token_Type::METHOD_CALL) || (token->type == Token_Type::ACTION_CALL) || (token->contents == ".")) {
        analyze_implied_this(program, token->children[0], scope);
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

Token *Analyzer::extract_function(Program *program, Token *token, Scope *var_scope) {
    if (token->children.size() > 0) {
        unsigned int i = 0;
        while (i < token->children.size()) {
            if ((token->children[i]->type == Token_Type::FUN_CALL)
                    && (token->type != Token_Type::BLOCK) && (token->contents != "=")
                    && (token->type != Token_Type::ACTION_CALL) && (token->type != Token_Type::METHOD_CALL)) {
                Var_Def *vd = new Var_Def();
                unsigned int type_def_num = token->children[i]->type_def_num;

                if (type_def_num == program->global->local_types["void"]) {
                    throw Compiler_Exception("Unexpected void return type", token->children[i]->start_pos);
                }

                vd->token = token->children[i];
                vd->type_def_num = type_def_num;
                vd->usage_start = token->children[i]->start_pos;

                program->vars.push_back(vd);

                unsigned int definition_number = program->vars.size() - 1;

                std::ostringstream varname;
                varname << "tmp__" << definition_number;

                //build a replacement variable reference to replace where the function had been
                Token *var_ref = new Token(Token_Type::VAR_CALL);
                var_ref->contents = varname.str();
                var_ref->start_pos = vd->token->start_pos;
                var_ref->end_pos = vd->token->end_pos;
                var_ref->definition_number = definition_number;
                var_ref->type_def_num = type_def_num;

                var_scope->local_vars[varname.str()] = definition_number;
                token->children[i] = var_ref;

                //return the old tree that held the function call so we can move it up in the block
                return vd->token;
            }
            else if ((token->children[i]->type == Token_Type::NEW_ALLOC)
                    && (token->type != Token_Type::BLOCK) && (token->contents != "=")) {
                Var_Def *vd = new Var_Def();
                unsigned int type_def_num = token->children[i]->type_def_num;

                if (type_def_num == program->global->local_types["void"]) {
                    throw Compiler_Exception("Unexpected void return type", token->children[i]->start_pos);
                }

                vd->token = token->children[i];
                vd->type_def_num = type_def_num;
                vd->usage_start = token->children[i]->start_pos;

                program->vars.push_back(vd);

                unsigned int definition_number = program->vars.size() - 1;

                std::ostringstream varname;
                varname << "tmp__" << definition_number;

                //build a replacement variable reference to replace where the function had been
                Token *var_ref = new Token(Token_Type::VAR_CALL);
                var_ref->contents = varname.str();
                var_ref->start_pos = vd->token->start_pos;
                var_ref->end_pos = vd->token->end_pos;
                var_ref->definition_number = definition_number;
                var_ref->type_def_num = type_def_num;

                var_scope->local_vars[varname.str()] = definition_number;
                token->children[i] = var_ref;

                //return the old tree that held the function call so we can move it up in the block
                return vd->token;
            }

            else if ((token->children[i]->type == Token_Type::SPAWN_ACTOR)
                    && (token->type != Token_Type::BLOCK) && (token->contents != "=")) {
                Var_Def *vd = new Var_Def();
                unsigned int type_def_num = token->children[i]->type_def_num;

                if (type_def_num == program->global->local_types["void"]) {
                    throw Compiler_Exception("Unexpected void return type", token->children[i]->start_pos);
                }

                vd->token = token->children[i];
                vd->type_def_num = type_def_num;
                vd->usage_start = token->children[i]->start_pos;

                program->vars.push_back(vd);

                unsigned int definition_number = program->vars.size() - 1;

                std::ostringstream varname;
                varname << "tmp__" << definition_number;

                //build a replacement variable reference to replace where the function had been
                Token *var_ref = new Token(Token_Type::VAR_CALL);
                var_ref->contents = varname.str();
                var_ref->start_pos = vd->token->start_pos;
                var_ref->end_pos = vd->token->end_pos;
                var_ref->definition_number = definition_number;
                var_ref->type_def_num = type_def_num;

                var_scope->local_vars[varname.str()] = definition_number;
                token->children[i] = var_ref;

                //return the old tree that held the function call so we can move it up in the block
                return vd->token;
            }
            else if ((token->children[i]->type == Token_Type::CONCATENATE)
                    && (token->type != Token_Type::BLOCK) && (token->contents != "=")) {
                Var_Def *vd = new Var_Def();
                unsigned int type_def_num = token->children[i]->type_def_num;

                if (type_def_num == program->global->local_types["void"]) {
                    throw Compiler_Exception("Unexpected void return type", token->children[i]->start_pos);
                }

                vd->token = token->children[i];
                vd->type_def_num = type_def_num;
                vd->usage_start = token->children[i]->start_pos;
                vd->usage_end = token->children[i]->end_pos;

                program->vars.push_back(vd);

                unsigned int definition_number = program->vars.size() - 1;

                std::ostringstream varname;
                varname << "tmp__" << definition_number;

                //build a replacement variable reference to replace where the function had been
                Token *var_ref = new Token(Token_Type::VAR_CALL);
                var_ref->contents = varname.str();
                var_ref->start_pos = vd->token->start_pos;
                var_ref->end_pos = vd->token->end_pos;
                var_ref->definition_number = definition_number;
                var_ref->type_def_num = type_def_num;

                var_scope->local_vars[varname.str()] = definition_number;
                token->children[i] = var_ref;

                //return the old tree that held the function call so we can move it up in the block
                return vd->token;
            }
            else if ((token->children[i]->type == Token_Type::REFERENCE_FEATURE)
                    && (token->type != Token_Type::BLOCK) && (token->contents != "=")) {
                Var_Def *vd = new Var_Def();
                unsigned int type_def_num = token->children[i]->type_def_num;

                if (type_def_num == program->global->local_types["void"]) {
                    throw Compiler_Exception("Unexpected void return type", token->children[i]->start_pos);
                }

                vd->token = token->children[i];
                vd->type_def_num = type_def_num;

                program->vars.push_back(vd);

                unsigned int definition_number = program->vars.size() - 1;

                std::ostringstream varname;
                varname << "tmp__" << definition_number;

                //build a replacement variable reference to replace where the function had been
                Token *var_ref = new Token(Token_Type::VAR_CALL);
                var_ref->contents = varname.str();
                var_ref->start_pos = vd->token->start_pos;
                var_ref->end_pos = vd->token->end_pos;
                var_ref->definition_number = definition_number;
                var_ref->type_def_num = type_def_num;

                var_scope->local_vars[varname.str()] = definition_number;
                token->children[i] = var_ref;

                //return the old tree that held the function call so we can move it up in the block
                return vd->token;
            }
            else if (token->children[i]->type == Token_Type::RETURN_CALL) {
                Token *child = token->children[i];

                if (child->children.size() > 1) {
                    if (child->children[1]->type != Token_Type::VAR_CALL) {
                        Var_Def *vd = new Var_Def();
                        unsigned int type_def_num = child->children[1]->type_def_num;
                        vd->token = child->children[1];
                        vd->type_def_num = type_def_num;

                        program->vars.push_back(vd);

                        unsigned int definition_number = program->vars.size() - 1;

                        std::ostringstream varname;
                        varname << "tmp__" << definition_number;

                        //build a replacement variable reference to replace where the function had been
                        Token *var_ref = new Token(Token_Type::VAR_CALL);
                        var_ref->contents = varname.str();
                        var_ref->start_pos = vd->token->start_pos;
                        var_ref->end_pos = vd->token->end_pos;
                        var_ref->definition_number = definition_number;
                        var_ref->type_def_num = type_def_num;

                        var_scope->local_vars[varname.str()] = definition_number;

                        Token *equation = new Token(Token_Type::SYMBOL);
                        equation->contents = "=";
                        equation->type_def_num = vd->type_def_num;

                        Token *var_ref2 = new Token(Token_Type::VAR_CALL);
                        var_ref2->contents = varname.str();
                        var_ref2->definition_number = definition_number;
                        var_ref2->type_def_num = vd->type_def_num;

                        equation->children.push_back(var_ref2);
                        equation->children.push_back(child->children[1]);

                        child->children[1] = var_ref;
                        token->children.insert(token->children.begin() + i, equation);
                        ++i;
                    }
                }
            }
            else {
                Token *results = extract_function(program, token->children[i], var_scope);
                if (results != NULL)  {
                    if (token->type == Token_Type::BLOCK) {
                        //We've backed out to the point we can now insert the new equation node before the
                        //function reference
                        Var_Def *vd = program->vars[program->vars.size() - 1];
                        Token *equation = new Token(Token_Type::SYMBOL);
                        equation->contents = "=";
                        equation->type_def_num = results->type_def_num;

                        Token *var_ref = new Token(Token_Type::VAR_DECL);
                        var_ref->definition_number = program->vars.size() - 1;
                        var_ref->type_def_num = results->type_def_num;

                        equation->children.push_back(var_ref);
                        equation->children.push_back(results);
                        vd->usage_end = results->end_pos;

                        token->children.insert(token->children.begin() + i, equation);

                        //skip over the increment so that we can dig deeper into what we just unpacked
                        continue;
                    }
                    else {
                        return results;
                    }
                }
            }
            ++i;
        }
    }
    return NULL;
}

void Analyzer::analyze_embedded_functions(Program *program, Token *token) {
    if ((token->type == Token_Type::FUN_DEF) || (token->type == Token_Type::ACTION_DEF)) {
        extract_function(program, token->children[2], token->children[2]->scope);
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        analyze_embedded_functions(program, token->children[i]);
    }
}

void Analyzer::analyze_return_calls(Program *program, Token *token, unsigned int allowed_return_type) {
    if ((token->type == Token_Type::FUN_DEF) || (token->type == Token_Type::ACTION_DEF)) {
        allowed_return_type = program->funs[token->definition_number]->return_type_def_num;
    }
    else if (token->type == Token_Type::RETURN_CALL) {
        if (token->children.size() == 1) {
            if (allowed_return_type != program->global->local_types["void"]) {
                throw Compiler_Exception("Void return in non-void function", token->start_pos);
            }
        }
        else {
            if (allowed_return_type != (unsigned)token->type_def_num) {
                throw Compiler_Exception("Return type does not match function return type", token->children[1]->start_pos);
            }
        }
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        analyze_return_calls(program, token->children[i], allowed_return_type);
    }
}

bool Analyzer::contains_var(Token *token, unsigned int var_def_num) {
    if ((token->type == Token_Type::VAR_CALL) && (token->definition_number == (signed)var_def_num)) {
        return true;
    }
    for (unsigned int i = 0; i < token->children.size(); ++i) {
        if (contains_var(token->children[i], var_def_num)) {
            return true;
        }
    }
    return false;
}

void Analyzer::find_var_endpoints(Program *program, Token *token, unsigned int var_def_num) {
    if (token->type == Token_Type::VAR_DECL) {
        if (token->definition_number == (signed)var_def_num) {
            Var_Def *vd = program->vars[var_def_num];
            vd->usage_end = token->end_pos;
        }
    }
    else if (token->type == Token_Type::WHILE_BLOCK) {
        if (contains_var(token, var_def_num)) {
            Var_Def *vd = program->vars[var_def_num];
            vd->usage_end = token->end_pos;
        }
    }
    /*
    else if (token->type == Token_Type::IF_BLOCK) {
        //todo: tracing usage will give more accurate results than doing it this way

        //First, look for else, if there's an else, it's the last point in an if block, otherwise, use whole if (until we get better tracing)
        find_var_endpoints(program, token->children[i], var_def_num);
    }
    */
    else if ((token->type == Token_Type::VAR_CALL) && (token->definition_number == (signed)var_def_num)) {
        Var_Def *vd = program->vars[var_def_num];
        vd->usage_end = token->end_pos;
    }
    else {
        for (unsigned int i = 0; i < token->children.size(); ++i) {
            find_var_endpoints(program, token->children[i], var_def_num);
        }
    }
}

void Analyzer::analyze_var_visibility(Program *program, Token *token) {
    if (token->scope != NULL) {
        if ((token->type == Token_Type::ACTOR_DEF) || (token->type == Token_Type::FEATURE_DEF) ||
                (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
            Scope *scope = token->scope;
            for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(), end = scope->local_vars.end(); iter != end; ++iter) {
                unsigned int var_def_num = iter->second;
                Var_Def *vd = program->vars[var_def_num];
                vd->is_property = true;
                vd->usage_start = token->start_pos;
                vd->usage_end = token->end_pos;
            }
            /*
            for (unsigned int i = 0; i < token->children[2]->children.size(); ++i) {
                if (token->type != Token_Type::VAR_DECL) {
                    analyze_var_visibility(program, token->children[2]->children[i]);
                }
            }
            */
            analyze_var_visibility(program, token->children[2]);
            return;
        }
        else if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
            Scope *scope = token->scope;
            for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(), end = scope->local_vars.end(); iter != end; ++iter) {
                unsigned int var_def_num = iter->second;
                Var_Def *vd = program->vars[var_def_num];
                vd->is_property = false;
                vd->usage_start = token->start_pos;

                if (token->type == Token_Type::ACTION_DEF) {
                    vd->usage_end = vd->token->end_pos;
                    find_var_endpoints(program, token->children[2], var_def_num);
                }
                else {
                    vd->usage_end = token->end_pos;
                }
            }
            /*
            for (unsigned int i = 0; i < token->children[2]->children.size(); ++i) {
                analyze_var_visibility(program, token->children[2]);
            }
            */
            analyze_var_visibility(program, token->children[2]);
            return;
        }
        else {
            Scope *scope = token->scope;
            for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(), end = scope->local_vars.end(); iter != end; ++iter) {
                unsigned int var_def_num = iter->second;
                find_var_endpoints(program, token, var_def_num);
            }
        }
    }
    for (unsigned int i = 0; i < token->children.size(); ++i) {
        analyze_var_visibility(program, token->children[i]);
    }
}

std::vector<int> Analyzer::build_push_pop_list(Program *program, Scope *scope, Position &tok_start, Position &tok_end) {
    std::vector<int> ret_val;

    while (scope != NULL) {
        for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                end = scope->local_vars.end(); iter != end; ++iter) {

            Var_Def *vd = program->vars[iter->second];
            if ((vd->usage_start <= tok_end) && (vd->usage_end >= tok_start) && (vd->is_property == false)) {
                ret_val.push_back(iter->second);
            }
        }
        scope = scope->parent;
    }

    return ret_val;
}

Token *Analyzer::create_temp_replacement(Program *program, Token *token, Scope *var_scope,
        unsigned int type_def_num, bool is_dependent) {
    Var_Def *vd = new Var_Def();

    if (type_def_num == program->global->local_types["void"]) {
        throw Compiler_Exception("Unexpected void return type", token->start_pos);
    }

    vd->token = new Token(token->type);
    *(vd->token) = *token;
    vd->type_def_num = type_def_num;
    vd->usage_start = token->start_pos;
    vd->is_dependent = is_dependent;

    program->vars.push_back(vd);

    unsigned int definition_number = program->vars.size() - 1;

    std::ostringstream varname;
    varname << "tmp__" << definition_number;

    //build a replacement variable reference to replace where the function had been
    Token *var_ref = new Token(Token_Type::VAR_CALL);
    var_ref->contents = varname.str();
    var_ref->start_pos = vd->token->start_pos;
    var_ref->end_pos = vd->token->end_pos;
    var_ref->definition_number = definition_number;
    var_ref->type_def_num = type_def_num;

    var_scope->local_vars[varname.str()] = definition_number;

    return var_ref;
}

Token *Analyzer::analyze_ports_of_entry(Program *program, Token *token, Scope *scope, bool is_lhs) {
    if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        //scope = token->scope;
        scope = token->children[2]->scope;

        analyze_ports_of_entry(program, token->children[2], scope, false);
    }
    else if ((token->type == Token_Type::FEATURE_DEF) || (token->type == Token_Type::ACTOR_DEF) ||
            (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
        analyze_ports_of_entry(program, token->children[2], scope, false);
    }
    else if (token->type == Token_Type::EXTERN_FUN_DEF) {
        //do nothing
    }
    else {
        if (token->type == Token_Type::BLOCK) {
            unsigned int i = 0;
            while (i < token->children.size()) {
                Token *ret_val = analyze_ports_of_entry(program, token->children[i], scope, is_lhs);
                if (ret_val != NULL) {
                    Var_Def *vd = program->vars[program->vars.size() - 1];
                    Token *equation = new Token(Token_Type::SYMBOL);
                    equation->contents = "=";
                    equation->type_def_num = ret_val->type_def_num;

                    Token *var_ref = new Token(Token_Type::VAR_DECL);
                    var_ref->definition_number = program->vars.size() - 1;
                    var_ref->type_def_num = ret_val->type_def_num;

                    equation->children.push_back(var_ref);
                    equation->children.push_back(ret_val);
                    vd->usage_end = ret_val->end_pos;

                    token->children.insert(token->children.begin() + i, equation);
                }
                ++i;
            }
            return NULL;
        }
        else {
            if (token->contents == "=") {
                Token *child = analyze_ports_of_entry(program, token->children[0], scope, true);
                if (child != NULL) {
                    return child;
                }

                child = analyze_ports_of_entry(program, token->children[1], scope, false);
                if (child != NULL) {
                    return child;
                }

                return NULL;
            }
            else if (token->contents == ":") {
                //ignore type stuff
                return NULL;
            }
            else if (token->type == Token_Type::ARRAY_CALL) {
                for (unsigned int j = 0; j < token->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[j], scope, is_lhs);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (is_lhs == false) {
                    if (token->type_def_num != (int)program->global->local_types["void"]) {
                        Token *replacement = create_temp_replacement(program, token, scope, token->type_def_num, false);
                        *token = *replacement;
                        return program->vars[token->definition_number]->token;
                    }
                    else {
                        return NULL;
                    }
                }
                else {
                    return NULL;
                }
            }
            else if (token->type == Token_Type::FUN_CALL) {
                for (unsigned int j = 0; j < token->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[j], scope, is_lhs);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (token->type_def_num != (int)program->global->local_types["void"]) {
                    if (token->definition_number == -1) {
                        throw Compiler_Exception("Internal definition number error", token->start_pos);
                    }
                    Function_Def *fd = program->funs[token->definition_number];
                    bool is_dependent;
                    if (fd->is_port_of_entry) {
                        is_dependent = true;
                    }
                    else {
                        is_dependent = false;
                    }

                    Token *replacement = create_temp_replacement(program, token, scope, token->type_def_num, is_dependent);
                    *token = *replacement;
                    return program->vars[token->definition_number]->token;
                }
                else {
                    return NULL;
                }
            }
            else if (token->type == Token_Type::METHOD_CALL) {
                Token *child = analyze_ports_of_entry(program, token->children[0], scope, is_lhs);
                if (child != NULL) {
                    return child;
                }

                for (unsigned int j = 0; j < token->children[1]->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[1]->children[j], scope, is_lhs);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (token->type_def_num != (int)program->global->local_types["void"]) {
                    if (token->children[1]->definition_number == -1) {
                        throw Compiler_Exception("Internal definition number error", token->start_pos);
                    }
                    Function_Def *fd = program->funs[token->children[1]->definition_number];
                    bool is_dependent;
                    if (fd->is_port_of_entry) {
                        is_dependent = true;
                    }
                    else {
                        is_dependent = false;
                    }
                    Token *replacement = create_temp_replacement(program, token, scope, token->type_def_num, is_dependent);
                    *token = *replacement;
                    return program->vars[token->definition_number]->token;
                }
                else {
                    return NULL;
                }
            }
            else if (token->type == Token_Type::REFERENCE_FEATURE) {
                for (unsigned int j = 0; j < token->children[0]->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[0]->children[j], scope, is_lhs);
                    if (child != NULL) {
                        return child;
                    }
                }
                for (unsigned int j = 0; j < token->children[1]->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[1]->children[j], scope, is_lhs);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (token->type_def_num != (int)program->global->local_types["void"]) {
                    Token *replacement = create_temp_replacement(program, token, scope, token->type_def_num, false);
                    *token = *replacement;
                    return program->vars[token->definition_number]->token;
                }
                else {
                    return NULL;
                }
            }
            else if (token->type == Token_Type::NEW_ALLOC) {
                if (token->children[1]->type != Token_Type::ARRAY_CALL) {
                    for (unsigned int j = 0; j < token->children[1]->children.size(); ++j) {
                        Token *child = analyze_ports_of_entry(program, token->children[1]->children[j], scope, is_lhs);
                        if (child != NULL) {
                            return child;
                        }
                    }
                }
                Token *replacement = create_temp_replacement(program, token, scope, token->type_def_num, true);
                *token = *replacement;
                return program->vars[token->definition_number]->token;
            }
            else if (token->type == Token_Type::SPAWN_ACTOR) {
                for (unsigned int j = 0; j < token->children[1]->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[1]->children[j], scope, is_lhs);
                    if (child != NULL) {
                        return child;
                    }
                }

                Token *replacement = create_temp_replacement(program, token, scope, token->type_def_num, true);
                *token = *replacement;
                return program->vars[token->definition_number]->token;
            }
            else if (token->type == Token_Type::CONCATENATE) {
                for (unsigned int j = 0; j < token->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[j], scope, is_lhs);
                    if (child != NULL) {
                        return child;
                    }
                }

                Token *replacement = create_temp_replacement(program, token, scope, token->type_def_num, true);
                *token = *replacement;
                return program->vars[token->definition_number]->token;
            }
            else {
                for (unsigned int j = 0; j < token->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[j], scope, is_lhs);
                    if (child != NULL) {
                        return child;
                    }
                }
                return NULL;
            }
        }
    }
    return NULL;
}

void Analyzer::analyze_freeze_resume(Program *program, Token *token, Scope *scope) {

    if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        //scope = token->scope;
        scope = token->children[2]->scope;

        analyze_freeze_resume(program, token->children[2], scope);
    }
    else if ((token->type == Token_Type::FEATURE_DEF) || (token->type == Token_Type::ACTOR_DEF) ||
            (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
        analyze_freeze_resume(program, token->children[2], scope);
    }
    else {
        for (unsigned int i = 0; i < token->children.size(); ++i) {
            if (token->children[i]->type != Token_Type::CONTINUATION_SITE) {
                analyze_freeze_resume(program, token->children[i], scope);
            }
        }

        if (token->type == Token_Type::BLOCK) {
            unsigned int i = 0;
            while (i < token->children.size()) {
                Token *child = token->children[i];

                if (child->type == Token_Type::FUN_CALL) {
                    Function_Def *fd = program->funs[child->definition_number];
                    if ((fd->is_internal == false) && (fd->external_name == "")) {
                        Token *continuation = new Token(Token_Type::CONTINUATION_SITE);
                        continuation->children.push_back(child);
                        continuation->start_pos = child->start_pos;
                        continuation->end_pos = child->end_pos;
                        std::vector<int> push_pop = build_push_pop_list(program, scope, child->start_pos, child->end_pos);
                        /*
                        if (push_pop.size() > 0) {
                            program->var_sites.push_back(push_pop);
                            continuation->definition_number = program->var_sites.size() - 1;
                        }
                        else {
                            continuation->definition_number = -1;
                        }
                        */
                        Scope *prev = scope;
                        while (scope != NULL) {
                            if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTION_DEF) ||
                                    (scope->owner->type == Token_Type::FUN_DEF))) {

                                Function_Def *owner = program->funs[scope->owner->definition_number];
                                program->var_sites.push_back(push_pop);
                                owner->continuation_sites.push_back(program->var_sites.size() - 1);
                                continuation->definition_number = program->var_sites.size() - 1;
                                break;
                            }
                            scope = scope->parent;
                        }
                        scope = prev;
                        token->children[i] = continuation;
                    }
                }
                else if  ((child->children.size() > 1) &&
                        ((child->type == Token_Type::SYMBOL) || (child->type == Token_Type::METHOD_CALL)) &&
                        (child->children[1]->type == Token_Type::FUN_CALL)) {

                    Function_Def *fd = program->funs[child->children[1]->definition_number];
                    if (fd->is_internal == false) {
                        Token *continuation = new Token(Token_Type::CONTINUATION_SITE);
                        continuation->children.push_back(child);
                        continuation->start_pos = child->start_pos;
                        continuation->end_pos = child->end_pos;
                        std::vector<int> push_pop = build_push_pop_list(program, scope, child->start_pos, child->end_pos);
                        /*
                        if (push_pop.size() > 0) {
                            program->var_sites.push_back(push_pop);
                            continuation->definition_number = program->var_sites.size() - 1;
                        }
                        else {
                            continuation->definition_number = -1;
                        }
                        */
                        Scope *prev = scope;
                        while (scope != NULL) {
                            if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTION_DEF) ||
                                    (scope->owner->type == Token_Type::FUN_DEF))) {

                                Function_Def *owner = program->funs[scope->owner->definition_number];
                                program->var_sites.push_back(push_pop);
                                owner->continuation_sites.push_back(program->var_sites.size() - 1);
                                continuation->definition_number = program->var_sites.size() - 1;
                                break;
                            }
                            scope = scope->parent;
                        }
                        scope = prev;

                        token->children[i] = continuation;
                    }
                }
                ++i;
            }
        }

        if (token->type == Token_Type::WHILE_BLOCK) {
            bool found_continuation = false;
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                for (unsigned int j = 0; j < token->children[i]->children.size(); ++j) {
                    if (token->children[i]->children[j]->type == Token_Type::CONTINUATION_SITE) {
                        found_continuation = true;
                    }
                }
            }
            if (!found_continuation) {
                Token *continuation = new Token(Token_Type::CONTINUATION_SITE);
                std::vector<int> push_pop = build_push_pop_list(program, scope, token->start_pos, token->end_pos);
                continuation->start_pos = token->start_pos;
                continuation->end_pos = token->end_pos;

                /*
                if (push_pop.size() > 0) {
                    program->var_sites.push_back(push_pop);
                    continuation->definition_number = program->var_sites.size() - 1;
                }
                else {
                    continuation->definition_number = -1;
                }
                */
                Scope *prev = scope;
                while (scope != NULL) {
                    if ((scope->owner != NULL) && ((scope->owner->type == Token_Type::ACTION_DEF) ||
                            (scope->owner->type == Token_Type::FUN_DEF))) {

                        Function_Def *owner = program->funs[scope->owner->definition_number];
                        program->var_sites.push_back(push_pop);
                        owner->continuation_sites.push_back(program->var_sites.size() - 1);
                        continuation->definition_number = program->var_sites.size() - 1;
                        break;
                    }
                    scope = scope->parent;
                }
                scope = prev;


                token->children[1]->children.insert(token->children[1]->children.begin(), continuation);
            }
        }
    }
}

std::vector<int> Analyzer::build_delete_list(Program *program, Scope *scope, Position &position) {
    std::vector<int> ret_val;

    while (scope != NULL) {
        for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                end = scope->local_vars.end(); iter != end; ++iter) {

            Var_Def *vd = program->vars[iter->second];

            if ((vd->usage_end <= position) && (vd->is_property == false)
                    && (vd->is_dependent == true) && (vd->is_removed == false) &&
                    (is_complex_var(program, iter->second))) {

                vd->is_removed = true;
                ret_val.push_back(iter->second);
            }
        }
        scope = scope->parent;
    }

    return ret_val;
}

std::vector<int> Analyzer::build_delete_remaining_list(Program *program, Scope *scope) {
    std::vector<int> ret_val;

    while (scope != NULL) {
        for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                end = scope->local_vars.end(); iter != end; ++iter) {

            Var_Def *vd = program->vars[iter->second];

            if ((vd->is_property == false) && (vd->is_dependent == true) && (vd->is_removed == false) &&
                    (is_complex_var(program, iter->second))) {

                vd->is_removed = true;
                ret_val.push_back(iter->second);
            }
        }
        scope = scope->parent;
    }

    return ret_val;
}


void Analyzer::analyze_copy_delete(Program *program, Token *token, Scope *scope) {

    if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        //scope = token->scope;
        scope = token->children[2]->scope;

        analyze_copy_delete(program, token->children[2], scope);
        if (token->children[2]->children.size() > 0) {
            if (token->children[2]->children[token->children[2]->children.size() - 1]->type != Token_Type::RETURN_CALL) {
                std::vector<int> delete_site = build_delete_list(program, scope, token->end_pos);

                if (delete_site.size() > 0) {
                    Token *deletion = new Token(Token_Type::DELETION_SITE);

                    program->var_sites.push_back(delete_site);
                    deletion->definition_number = program->var_sites.size() - 1;

                    token->children[2]->children.push_back(deletion);
                }

            }
        }
    }
    else if ((token->type == Token_Type::FEATURE_DEF) || (token->type == Token_Type::ACTOR_DEF) ||
            (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
        analyze_copy_delete(program, token->children[2], scope);
    }
    else {
        if (token->type == Token_Type::BLOCK) {
            unsigned int i = 0;
            while (i < token->children.size()) {
                Token *child = token->children[i];

                //While handles analysis differently, so we check for that here
                if (child->type != Token_Type::WHILE_BLOCK) {
                    analyze_copy_delete(program, token->children[i], scope);
                }

                if (child->type == Token_Type::CONTINUATION_SITE) {
                    std::vector<int> delete_site = build_delete_list(program, scope, child->start_pos);
                    if (delete_site.size() > 0) {
                        Token *deletion = new Token(Token_Type::DELETION_SITE);
                        program->var_sites.push_back(delete_site);
                        deletion->definition_number = program->var_sites.size() - 1;

                        child->children.push_back(deletion);
                    }
                    ++i;
                }
                else if (child->type == Token_Type::WHILE_BLOCK) {
                    std::vector<int> delete_site = build_delete_list(program, scope, child->start_pos);
                    analyze_copy_delete(program, token->children[i], scope);
                    if (delete_site.size() > 0) {
                        Token *deletion = new Token(Token_Type::DELETION_SITE);
                        program->var_sites.push_back(delete_site);
                        deletion->definition_number = program->var_sites.size() - 1;

                        token->children.insert(token->children.begin() + i, deletion);
                        i += 2;
                    }
                    else {
                        ++i;
                    }
                }
                else if (child->type == Token_Type::RETURN_CALL) {
                    if (child->children.size() > 1) {
                        if (child->children[1]->type == Token_Type::VAR_CALL) {
                            Var_Def *vd = program->vars[child->children[1]->definition_number];
                            if (vd->is_dependent == true) {
                                vd->is_removed = true;
                                std::vector<int> delete_site = build_delete_remaining_list(program, scope);
                                if (delete_site.size() > 0) {
                                    Token *deletion = new Token(Token_Type::DELETION_SITE);
                                    program->var_sites.push_back(delete_site);
                                    deletion->definition_number = program->var_sites.size() - 1;

                                    token->children.insert(token->children.begin() + i, deletion);
                                    i += 2;
                                }
                                else {
                                    ++i;
                                }
                                vd->is_removed = false;
                            }
                            else {
                                if (is_complex_type(program, child->children[1]->type_def_num)) {
                                    Token *copy_t = new Token(Token_Type::COPY);
                                    copy_t->start_pos = child->children[1]->start_pos;
                                    copy_t->end_pos = child->children[1]->end_pos;
                                    copy_t->type_def_num = child->children[1]->type_def_num;
                                    copy_t->children.push_back(child->children[1]);

                                    child->children[1] = copy_t;
                                }
                                std::vector<int> delete_site = build_delete_remaining_list(program, scope);
                                if (delete_site.size() > 0) {
                                    Token *deletion = new Token(Token_Type::DELETION_SITE);
                                    program->var_sites.push_back(delete_site);
                                    deletion->definition_number = program->var_sites.size() - 1;

                                    token->children.insert(token->children.begin() + i, deletion);
                                    i += 2;
                                }
                                else {
                                    ++i;
                                }
                            }
                        }
                        else {
                            ++i;
                        }
                    }
                    else {
                        std::vector<int> delete_site = build_delete_remaining_list(program, scope);
                        if (delete_site.size() > 0) {
                            Token *deletion = new Token(Token_Type::DELETION_SITE);
                            program->var_sites.push_back(delete_site);
                            deletion->definition_number = program->var_sites.size() - 1;

                            token->children.insert(token->children.begin() + i, deletion);
                            i += 2;
                        }
                        else {
                            ++i;
                        }
                    }
                }
                else if ((child->type == Token_Type::ACTION_CALL) || (child->type == Token_Type::METHOD_CALL)
                        || (child->type == Token_Type::FUN_CALL)) {

                    Function_Def *fd;
                    if ((child->type == Token_Type::ACTION_CALL) || (child->type == Token_Type::METHOD_CALL)) {
                        fd = program->funs[child->children[1]->definition_number];
                    }
                    else if (child->type == Token_Type::FUN_CALL) {
                        fd = program->funs[child->definition_number];
                    }

                    if (fd->is_port_of_exit) {
                        if (child->children[1]->children.size() > 1) {
                            Token *parm = child->children[1]->children[1];
                            if (parm->contents == ",") {
                                while (parm->contents == ",") {
                                    Token *rhs = parm->children[1];
                                    Token *lhs = parm->children[0];

                                    if (rhs->contents == ".") {
                                        Token *copy_t = new Token(Token_Type::COPY);
                                        copy_t->start_pos = rhs->start_pos;
                                        copy_t->end_pos = rhs->end_pos;
                                        copy_t->type_def_num = rhs->type_def_num;
                                        copy_t->children.push_back(rhs);

                                        parm->children[1] = copy_t;
                                    }
                                    else if (rhs->type == Token_Type::VAR_CALL) {
                                        Var_Def *vd = program->vars[rhs->definition_number];
                                        if (((vd->is_removed == false) || (vd->is_dependent == false))
                                                && (is_complex_var(program, rhs->definition_number)))  {
                                            if ((vd->usage_end == rhs->end_pos) && (vd->is_dependent == true)) {
                                                vd->is_removed = true;
                                            }
                                            else {
                                                Token *copy_t = new Token(Token_Type::COPY);
                                                copy_t->start_pos = rhs->start_pos;
                                                copy_t->end_pos = rhs->end_pos;
                                                copy_t->type_def_num = rhs->type_def_num;
                                                copy_t->children.push_back(rhs);

                                                parm->children[1] = copy_t;
                                            }
                                        }
                                    }

                                    if (lhs->contents == ".") {
                                        Token *copy_t = new Token(Token_Type::COPY);
                                        copy_t->start_pos = lhs->start_pos;
                                        copy_t->end_pos = lhs->end_pos;
                                        copy_t->type_def_num = lhs->type_def_num;
                                        copy_t->children.push_back(lhs);

                                        parm->children[0] = copy_t;
                                    }
                                    else if (lhs->type == Token_Type::VAR_CALL) {
                                        Var_Def *vd = program->vars[lhs->definition_number];
                                        if (((vd->is_removed == false) || (vd->is_dependent == false))
                                                && (is_complex_var(program, lhs->definition_number)))  {

                                            if ((vd->usage_end == lhs->end_pos) && (vd->is_dependent == true)) {
                                                vd->is_removed = true;
                                            }
                                            else {
                                                Token *copy_t = new Token(Token_Type::COPY);
                                                copy_t->start_pos = lhs->start_pos;
                                                copy_t->end_pos = lhs->end_pos;
                                                copy_t->type_def_num = lhs->type_def_num;
                                                copy_t->children.push_back(lhs);

                                                parm->children[0] = copy_t;
                                            }
                                        }
                                    }
                                    parm = parm->children[0];
                                }
                            }
                            else {
                                if (parm->contents == ".") {
                                    if (is_complex_type(program, parm->type_def_num)) {
                                        Token *copy_t = new Token(Token_Type::COPY);
                                        copy_t->start_pos = parm->start_pos;
                                        copy_t->end_pos = parm->end_pos;
                                        copy_t->type_def_num = parm->type_def_num;
                                        copy_t->children.push_back(parm);

                                        child->children[1]->children[1] = copy_t;
                                    }
                                }
                                else if (parm->type == Token_Type::VAR_CALL) {
                                    Var_Def *vd = program->vars[parm->definition_number];
                                    if (((vd->is_removed == false) || (vd->is_dependent == false))
                                            && (is_complex_var(program, parm->definition_number)))  {

                                        if ((vd->usage_end == parm->end_pos) && (vd->is_dependent == true)) {
                                            vd->is_removed = true;
                                        }
                                        else {
                                            Token *copy_t = new Token(Token_Type::COPY);
                                            copy_t->start_pos = parm->start_pos;
                                            copy_t->end_pos = parm->end_pos;
                                            copy_t->type_def_num = parm->type_def_num;
                                            copy_t->children.push_back(parm);

                                            child->children[1]->children[1] = copy_t;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    ++i;
                }
                else if (child->contents == "<+") {
                    while (child->contents == "<+") {
                        if (child->children[1]->type == Token_Type::VAR_CALL) {
                            Var_Def *vd = program->vars[child->children[1]->definition_number];
                            if (vd->is_dependent == true) {
                                vd->is_removed = true;
                            }
                            else {
                                Token *copy_t = new Token(Token_Type::COPY);
                                copy_t->start_pos = child->children[1]->start_pos;
                                copy_t->end_pos = child->children[1]->end_pos;
                                copy_t->type_def_num = child->children[1]->type_def_num;
                                copy_t->children.push_back(child->children[1]);

                                child->children[1] = copy_t;
                            }
                        }
                        child = child->children[0];
                    }
                    ++i;
                }
                else if (child->contents == "+>") {
                    if ((child->children[1]->type == Token_Type::VAR_CALL) || (child->children[1]->type == Token_Type::VAR_DECL)) {
                        Var_Def *vd = program->vars[child->children[1]->definition_number];

                        if ((vd->is_removed == false) && (vd->is_dependent == true)) {
                            Token *delete_t = new Token(Token_Type::DELETE);
                            delete_t->children.push_back(child->children[1]);
                            token->children.insert(token->children.begin() + i, delete_t);

                            i += 2;
                        }
                        else {
                            ++i;
                        }

                        vd->is_removed = false;
                    }
                    else {
                        ++i;
                    }
                }
                else if (child->contents == "=") {
                    //Don't allow any deletion of references
                    if ((child->children[0]->type == Token_Type::VAR_CALL) || (child->children[0]->type == Token_Type::VAR_DECL)) {
                        Var_Def *vd = program->vars[child->children[0]->definition_number];

                        if ((vd->is_removed == false) && (vd->is_dependent == true) &&
                                (is_complex_var(program, child->children[0]->definition_number))) {

                            Token *delete_t = new Token(Token_Type::DELETE);
                            delete_t->children.push_back(child->children[0]);
                            token->children.insert(token->children.begin() + i, delete_t);

                            i += 2;
                        }
                        else {
                            ++i;
                        }

                        if (child->children[1]->type == Token_Type::REFERENCE_FEATURE) {
                            //vd->is_removed = true;
                            vd->is_dependent = false;
                        }
                        else if (child->children[1]->type == Token_Type::COPY) {
                            vd->is_dependent = true;
                        }
                    }
                    else {
                        ++i;
                    }

                    //Copy any rhs that is not ours
                    if ((child->children[1]->type == Token_Type::VAR_DECL) || (child->children[1]->type == Token_Type::VAR_CALL)) {
                        Var_Def *vd = program->vars[child->children[1]->definition_number];
                        if (((vd->is_removed == false) || (vd->is_dependent == false))
                                && (is_complex_var(program, child->children[1]->definition_number))) {

                            if ((vd->usage_end == child->children[1]->end_pos) && (vd->is_dependent == true)) {
                                vd->is_removed = true;
                            }
                            else {
                                Token *copy_t = new Token(Token_Type::COPY);
                                copy_t->start_pos = child->children[1]->start_pos;
                                copy_t->end_pos = child->children[1]->end_pos;
                                copy_t->type_def_num = child->children[1]->type_def_num;
                                copy_t->children.push_back(child->children[1]);

                                child->children[1] = copy_t;
                            }
                        }
                    }
                }
                else {
                    ++i;
                }
            }
        }
        else {
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                analyze_copy_delete(program, token->children[i], scope);
            }
        }
    }
}


