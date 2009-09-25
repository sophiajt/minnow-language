// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include "Common.hpp"
#include "Var_Scope_Analyzer.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

bool Var_Scope_Analyzer::contains_var(Token *token, unsigned int var_def_num) {
    if ( ((token->type == Token_Type::VAR_DECL) || (token->type == Token_Type::VAR_CALL)) &&
            (token->definition_number == (signed)var_def_num)) {
        return true;
    }
    for (unsigned int i = 0; i < token->children.size(); ++i) {
        if (contains_var(token->children[i], var_def_num)) {
            return true;
        }
    }
    return false;
}

void attach_extent(Var_Def *vd, Extent *extent) {
    if (vd->extent == NULL) {
        vd->extent = extent;
    }
    else {
        Extent *pos = vd->extent;

        while (pos->next != NULL) {
            pos = pos->next;
        }
        pos->next = extent;
        extent->prev = pos;
    }
}

void Var_Scope_Analyzer::analyze_usage_extents(Program *program, Token *token, Token *bounds, Scope *scope) {
    Extent *extent;

    if (token->scope != NULL) {
        scope = token->scope;
    }


    if ((token->type == Token_Type::FUN_CALL) || (token->type == Token_Type::METHOD_CALL) || (token->type == Token_Type::ACTION_CALL)) {
        bounds = token;

        for (unsigned int i = 0; i < token->children.size(); ++i) {
            analyze_usage_extents(program, token->children[i], bounds, scope);
        }

    }
    else if (token->type == Token_Type::VAR_DECL) {
        if (token->definition_number != -1) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->start_pos;
                extent->end_pos = token->end_pos;
            }
            extent->type = Extent_Type::DECLARE;
            attach_extent(program->vars[token->definition_number], extent);
        }
    }
    else if (token->type == Token_Type::VAR_CALL) {
        if (token->definition_number != -1) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->start_pos;
                extent->end_pos = token->end_pos;
            }
            extent->type = Extent_Type::READ;
            attach_extent(program->vars[token->definition_number], extent);
        }
    }
    else if (token->contents == "=") {
        if (token->children[0]->type == Token_Type::VAR_DECL) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->children[0]->start_pos;
                extent->end_pos = token->children[0]->end_pos;
            }
            extent->type = Extent_Type::DECLARE;
            attach_extent(program->vars[token->children[0]->definition_number], extent);

            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->children[0]->start_pos;
                extent->end_pos = token->children[0]->end_pos;
            }
            extent->type = Extent_Type::WRITE;
            attach_extent(program->vars[token->children[0]->definition_number], extent);
        }
        if (token->children[0]->type == Token_Type::VAR_CALL) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->children[0]->start_pos;
                extent->end_pos = token->children[0]->end_pos;
            }
            extent->type = Extent_Type::WRITE;
            attach_extent(program->vars[token->children[0]->definition_number], extent);
        }
        for (unsigned int i = 0; i < token->children[0]->children.size(); ++i) {
            analyze_usage_extents(program, token->children[0]->children[i], bounds, scope);
        }
        analyze_usage_extents(program, token->children[1], bounds, scope);
    }
    else if (token->type == Token_Type::WHILE_BLOCK) {
        Scope *s;
        s = scope;

        for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->start_pos;
                extent->end_pos = token->start_pos;
            }
            extent->type = Extent_Type::LOOP_START;
            attach_extent(program->vars[iter->second], extent);
        }
        do {
            s = s->parent;
            for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
                extent = new Extent();
                if (bounds != NULL) {
                    extent->start_pos = bounds->start_pos;
                    extent->end_pos = bounds->end_pos;
                }
                else {
                    extent->start_pos = token->start_pos;
                    extent->end_pos = token->start_pos;
                }
                extent->type = Extent_Type::LOOP_START;
                attach_extent(program->vars[iter->second], extent);
            }

        } while ((s->owner->type != Token_Type::ACTION_DEF) && (s->owner->type != Token_Type::FUN_DEF));


        for (unsigned int i = 0; i < token->children.size(); ++i) {
            analyze_usage_extents(program, token->children[i], bounds, scope);
        }

        s = scope;

        for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->end_pos;
                extent->end_pos = token->end_pos;
            }
            extent->type = Extent_Type::LOOP_JOIN;
            attach_extent(program->vars[iter->second], extent);
        }
        do {
            s = s->parent;
            for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
                extent = new Extent();
                if (bounds != NULL) {
                    extent->start_pos = bounds->start_pos;
                    extent->end_pos = bounds->end_pos;
                }
                else {
                    extent->start_pos = token->end_pos;
                    extent->end_pos = token->end_pos;
                }
                extent->type = Extent_Type::LOOP_JOIN;
                attach_extent(program->vars[iter->second], extent);
            }

        } while ((s->owner->type != Token_Type::ACTION_DEF) && (s->owner->type != Token_Type::FUN_DEF));
    }
    else if (token->type == Token_Type::IF_BLOCK) {
        Scope *s;
        s = scope;

        for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->start_pos;
                extent->end_pos = token->start_pos;
            }
            extent->type = Extent_Type::IF_START;
            attach_extent(program->vars[iter->second], extent);
        }
        do {
            s = s->parent;
            for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
                extent = new Extent();
                if (bounds != NULL) {
                    extent->start_pos = bounds->start_pos;
                    extent->end_pos = bounds->end_pos;
                }
                else {
                    extent->start_pos = token->start_pos;
                    extent->end_pos = token->start_pos;
                }
                extent->type = Extent_Type::IF_START;
                attach_extent(program->vars[iter->second], extent);
            }

        } while ((s->owner->type != Token_Type::ACTION_DEF) && (s->owner->type != Token_Type::FUN_DEF));


        for (unsigned int i = 0; i < token->children.size(); ++i) {
            analyze_usage_extents(program, token->children[i], bounds, scope);
        }

        s = scope;

        for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->end_pos;
                extent->end_pos = token->end_pos;
            }
            extent->type = Extent_Type::IF_JOIN;
            attach_extent(program->vars[iter->second], extent);
        }
        do {
            s = s->parent;
            for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
                extent = new Extent();
                if (bounds != NULL) {
                    extent->start_pos = bounds->start_pos;
                    extent->end_pos = bounds->end_pos;
                }
                else {
                    extent->start_pos = token->end_pos;
                    extent->end_pos = token->end_pos;
                }
                extent->type = Extent_Type::IF_JOIN;
                attach_extent(program->vars[iter->second], extent);
            }

        } while ((s->owner->type != Token_Type::ACTION_DEF) && (s->owner->type != Token_Type::FUN_DEF));
    }
    else if (token->type == Token_Type::ELSEIF_BLOCK) {
        Scope *s;
        s = scope;

        for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->start_pos;
                extent->end_pos = token->start_pos;
            }
            extent->type = Extent_Type::ELSEIF_START;
            attach_extent(program->vars[iter->second], extent);
        }
        do {
            s = s->parent;
            for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
                extent = new Extent();
                if (bounds != NULL) {
                    extent->start_pos = bounds->start_pos;
                    extent->end_pos = bounds->end_pos;
                }
                else {
                    extent->start_pos = token->start_pos;
                    extent->end_pos = token->start_pos;
                }
                extent->type = Extent_Type::ELSEIF_START;
                attach_extent(program->vars[iter->second], extent);
            }

        } while ((s->owner->type != Token_Type::ACTION_DEF) && (s->owner->type != Token_Type::FUN_DEF));


        for (unsigned int i = 0; i < token->children.size(); ++i) {
            analyze_usage_extents(program, token->children[i], bounds, scope);
        }
    }
    else if (token->type == Token_Type::ELSE_BLOCK) {
        Scope *s;
        s = scope;

        for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
            extent = new Extent();
            if (bounds != NULL) {
                extent->start_pos = bounds->start_pos;
                extent->end_pos = bounds->end_pos;
            }
            else {
                extent->start_pos = token->start_pos;
                extent->end_pos = token->start_pos;
            }
            extent->type = Extent_Type::ELSE_START;
            attach_extent(program->vars[iter->second], extent);
        }
        do {
            s = s->parent;
            for (std::map<std::string, unsigned int>::iterator iter = s->local_vars.begin(), end = s->local_vars.end(); iter != end; ++iter) {
                extent = new Extent();
                if (bounds != NULL) {
                    extent->start_pos = bounds->start_pos;
                    extent->end_pos = bounds->end_pos;
                }
                else {
                    extent->start_pos = token->start_pos;
                    extent->end_pos = token->start_pos;
                }
                extent->type = Extent_Type::ELSE_START;
                attach_extent(program->vars[iter->second], extent);
            }

        } while ((s->owner->type != Token_Type::ACTION_DEF) && (s->owner->type != Token_Type::FUN_DEF));


        for (unsigned int i = 0; i < token->children.size(); ++i) {
            analyze_usage_extents(program, token->children[i], bounds, scope);
        }
    }
    else if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        for (unsigned int i = 0; i < token->children[2]->children.size(); ++i) {
            analyze_usage_extents(program, token->children[2]->children[i], bounds, scope);
        }
    }

    else {
        for (unsigned int i = 0; i < token->children.size(); ++i) {
            analyze_usage_extents(program, token->children[i], bounds, scope);
        }
    }
}

void Var_Scope_Analyzer::analyze_usage_extent_for_var(Program *program, Extent *extent) {
    if (extent->type == Extent_Type::READ) {
        extent->site_type = Extent_Site_Type::READ;
    }
    else if (extent->type == Extent_Type::WRITE) {
        extent->site_type = Extent_Site_Type::WRITE;
    }
}

void Var_Scope_Analyzer::analyze_usage_extent_colors(Program *program) {
    for (unsigned int i = 0; i < program->vars.size(); ++i) {

    }
}

void Var_Scope_Analyzer::find_var_endpoints(Program *program, Token *token, Token *bounds, unsigned int var_def_num) {
    if (token->type == Token_Type::VAR_DECL) {
        if (token->definition_number == (signed)var_def_num) {
            Var_Def *vd = program->vars[var_def_num];
            if (bounds != NULL) {
                vd->usage_end = bounds->end_pos;
            }
            else {

                vd->usage_end = token->end_pos;
            }
        }
    }
    else if ((token->type == Token_Type::WHILE_BLOCK) || (token->type == Token_Type::FOR_BLOCK)) {
        if (contains_var(token, var_def_num)) {
            Var_Def *vd = program->vars[var_def_num];
            if (token->end_pos > vd->usage_end) {
                vd->usage_end = token->end_pos;
            }
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

        if (bounds != NULL) {
            vd->usage_end = bounds->end_pos;
        }
        else {

            vd->usage_end = token->end_pos;
        }
    }
    else {
        if (bounds == NULL) {
            switch (token->type) {
                case (Token_Type::ACTION_CALL) :
                case (Token_Type::ARRAY_CALL) :
                case (Token_Type::CONCATENATE) :
                case (Token_Type::CONCATENATE_ARRAY) :
                case (Token_Type::CONSTRUCTOR_CALL) :
                case (Token_Type::FUN_CALL) :
                case (Token_Type::METHOD_CALL) :
                case (Token_Type::NEW_ALLOC) :
                case (Token_Type::RETURN_CALL) :
                case (Token_Type::SYMBOL) :
                    bounds = token;
                break;
                default:
                    break;
            }
        }
        for (unsigned int i = 0; i < token->children.size(); ++i) {
            find_var_endpoints(program, token->children[i], bounds, var_def_num);
        }
    }
}

void Var_Scope_Analyzer::analyze_var_visibility(Program *program, Token *token) {
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
                    find_var_endpoints(program, token->children[2], NULL, var_def_num);
                }
                else {
                    vd->is_dependent = false;
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
                find_var_endpoints(program, token, NULL, var_def_num);
            }
        }
    }
    for (unsigned int i = 0; i < token->children.size(); ++i) {
        analyze_var_visibility(program, token->children[i]);
    }
}

std::vector<int> Var_Scope_Analyzer::build_push_pop_list(Program *program, Scope *scope, Position &tok_start, Position &tok_end) {
    std::vector<int> ret_val;

    while (scope != NULL) {
        for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(),
                end = scope->local_vars.end(); iter != end; ++iter) {

            Var_Def *vd = program->vars[iter->second];

            /*
            std::cout << "Checking " << iter->second << " against: " << tok_start.line << " " << tok_start.col << " to " <<
                tok_end.line << " " << tok_end.col << " vs " << vd->usage_start.line << " " << vd->usage_start.col << " to " <<
                vd->usage_end.line << " " << vd->usage_end.col << std::endl;
            */
            if ((vd->usage_start <= tok_end) && (vd->usage_end >= tok_start) && (vd->is_property == false)) {
                //std::cout << "Pushing" << std::endl;
                ret_val.push_back(iter->second);
            }
        }
        scope = scope->parent;
    }

    return ret_val;
}

Token *Var_Scope_Analyzer::create_temp_replacement(Program *program, Token *token, Token *bounds, Scope *var_scope,
        unsigned int type_def_num, bool is_dependent) {
    Var_Def *vd = new Var_Def();

    if (type_def_num == program->global->local_types["void"]) {
        throw Compiler_Exception("Unexpected void return type", token->start_pos, token->end_pos);
    }

    vd->token = new Token(token->type);
    *(vd->token) = *token;
    vd->type_def_num = type_def_num;
    vd->is_dependent = is_dependent;
    vd->usage_start = bounds->start_pos;
    vd->usage_end = bounds->end_pos;

    program->vars.push_back(vd);

    unsigned int definition_number = program->vars.size() - 1;

    std::ostringstream varname;
    varname << "tmp__" << definition_number;

    //build a replacement variable reference to replace where the function had been
    Token *var_ref = new Token(Token_Type::VAR_CALL);
    var_ref->contents = varname.str();
    var_ref->definition_number = definition_number;
    var_ref->type_def_num = type_def_num;
    var_ref->start_pos = token->start_pos;
    var_ref->end_pos = token->end_pos;

    var_scope->local_vars[varname.str()] = definition_number;
    /*
    std::cout << "Creating: " << definition_number << " " << bounds->start_pos.line << " " << bounds->start_pos.col
        << " to " << bounds->end_pos.line << " " << bounds->end_pos.col << std::endl;
    */
    return var_ref;
}

Token *Var_Scope_Analyzer::analyze_ports_of_entry(Program *program, Token *token, Token *bounds, Scope *scope, bool is_lhs, bool is_block_bound) {
    if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        //scope = token->scope;
        scope = token->children[2]->scope;

        analyze_ports_of_entry(program, token->children[2], bounds, scope, false, false);
    }
    /*
    else if ((token->type == Token_Type::FEATURE_DEF) || (token->type == Token_Type::ACTOR_DEF) ||
            (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
        analyze_ports_of_entry(program, token->children[2], bounds, scope, false, false);
    }
    */
    else if (token->type == Token_Type::EXTERN_FUN_DEF) {
        //do nothing
    }
    else {
        if (token->type == Token_Type::BLOCK) {
            unsigned int i = 0;

            while (i < token->children.size()) {
                Token *ret_val;

                if (!is_block_bound) {
                    if (token->children[i]->type == Token_Type::WHILE_BLOCK) {
                        bounds = token->children[i];
                        ret_val = analyze_ports_of_entry(program, token->children[i], bounds, scope, is_lhs, true);
                    }
                }
                if (is_block_bound) {
                    ret_val = analyze_ports_of_entry(program, token->children[i], bounds, scope, is_lhs, is_block_bound);
                }
                else {
                    ret_val = analyze_ports_of_entry(program, token->children[i], token->children[i], scope, is_lhs, is_block_bound);
                }

                if (ret_val != NULL) {
                    Var_Def *vd = program->vars[program->vars.size() - 1];

                    Token *equation = new Token(Token_Type::SYMBOL);
                    equation->contents = "=";
                    equation->type_def_num = ret_val->type_def_num;
                    equation->start_pos = ret_val->start_pos;
                    equation->end_pos = ret_val->end_pos;

                    Token *var_ref = new Token(Token_Type::VAR_DECL);
                    var_ref->definition_number = program->vars.size() - 1;
                    var_ref->type_def_num = ret_val->type_def_num;
                    var_ref->start_pos = ret_val->start_pos;
                    var_ref->end_pos = ret_val->end_pos;

                    equation->children.push_back(var_ref);
                    equation->children.push_back(ret_val);
                    vd->usage_end = token->children[i]->end_pos;
                    vd->is_temporary = true;

                    token->children.insert(token->children.begin() + i, equation);
                }
                ++i;
            }
            return NULL;
        }
        else {
            if (token->contents == "<+") {
                Token *child = analyze_ports_of_entry(program, token->children[0], bounds, scope, is_lhs, is_block_bound);
                if (child != NULL) {
                    return child;
                }

                child = analyze_ports_of_entry(program, token->children[1], bounds, scope, is_lhs, is_block_bound);
                if (child != NULL) {
                    Var_Def *vd = program->vars[program->vars.size() - 1];
                    vd->is_dependent = false;
                    return child;
                }
                else if ((token->children[1]->type == Token_Type::VAR_CALL)|| (token->children[1]->type == Token_Type::VAR_DECL)) {
                    Var_Def *vd = program->vars[token->children[1]->definition_number];
                    vd->is_dependent = false;
                }

                Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, false);
                *token = *replacement;
                return program->vars[token->definition_number]->token;
            }
            if (token->contents == "=") {
                Token *child = analyze_ports_of_entry(program, token->children[0], bounds, scope, true, is_block_bound);
                if (child != NULL) {
                    return child;
                }

                child = analyze_ports_of_entry(program, token->children[1], bounds, scope, false, is_block_bound);
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
                    Token *child = analyze_ports_of_entry(program, token->children[j], bounds, scope, is_lhs, is_block_bound);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (is_lhs == false) {
                    if (token->type_def_num != (int)program->global->local_types["void"]) {
                        Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, false);
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
            else if (token->type == Token_Type::ARRAY_INIT) {
                for (unsigned int j = 0; j < token->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[j], bounds, scope, is_lhs, is_block_bound);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (is_lhs == false) {
                    if (token->type_def_num != (int)program->global->local_types["void"]) {
                        Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, true);
                        *token = *replacement;
                        return program->vars[token->definition_number]->token;
                    }
                    else {
                        return NULL;
                    }
                }
                else {
                    throw Compiler_Exception("Array initialization can not be on the left hand side of an equation", token->start_pos, token->end_pos);
                }
            }
            else if (token->type == Token_Type::FUN_CALL) {
                for (unsigned int j = 0; j < token->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[j], bounds, scope, is_lhs, is_block_bound);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (token->type_def_num != (int)program->global->local_types["void"]) {
                    if (token->definition_number == -1) {
                        throw Compiler_Exception("Internal definition number error", token->start_pos, token->end_pos);
                    }
                    Function_Def *fd = program->funs[token->definition_number];

                    bool is_dependent;
                    if (fd->is_port_of_entry) {
                        is_dependent = true;
                    }
                    else {
                        is_dependent = false;
                    }

                    /*
                    if ((is_dependent) && (is_lhs == false)) {
                        is_dependent = false;
                    }
                    */

                    Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, is_dependent);
                    *token = *replacement;
                    return program->vars[token->definition_number]->token;
                }
                else {
                    return NULL;
                }
            }
            else if (token->type == Token_Type::METHOD_CALL) {
                Token *child = analyze_ports_of_entry(program, token->children[0], bounds, scope, is_lhs, is_block_bound);
                if (child != NULL) {
                    return child;
                }

                for (unsigned int j = 0; j < token->children[1]->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[1]->children[j], bounds, scope, is_lhs, is_block_bound);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (token->type_def_num != (int)program->global->local_types["void"]) {
                    if (token->children[1]->definition_number == -1) {
                        throw Compiler_Exception("Internal definition number error", token->start_pos, token->end_pos);
                    }
                    Function_Def *fd = program->funs[token->children[1]->definition_number];
                    bool is_dependent;
                    if (fd->is_port_of_entry) {
                        is_dependent = true;
                    }
                    else {
                        is_dependent = false;
                    }
                    Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, is_dependent);
                    *token = *replacement;
                    return program->vars[token->definition_number]->token;
                }
                else {
                    return NULL;
                }
            }
            else if (token->type == Token_Type::REFERENCE_FEATURE) {
                for (unsigned int j = 0; j < token->children[0]->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[0]->children[j], bounds, scope, is_lhs, is_block_bound);
                    if (child != NULL) {
                        return child;
                    }
                }
                for (unsigned int j = 0; j < token->children[1]->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[1]->children[j], bounds, scope, is_lhs, is_block_bound);
                    if (child != NULL) {
                        return child;
                    }
                }

                if (token->type_def_num != (int)program->global->local_types["void"]) {
                    Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, false);
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
                        Token *child = analyze_ports_of_entry(program, token->children[1]->children[j], bounds, scope, is_lhs, is_block_bound);
                        if (child != NULL) {
                            return child;
                        }
                    }
                }
                Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, true);
                *token = *replacement;
                return program->vars[token->definition_number]->token;
            }
            else if (token->type == Token_Type::SPAWN_ACTOR) {
                for (unsigned int j = 0; j < token->children[1]->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[1]->children[j], bounds, scope, is_lhs, is_block_bound);
                    if (child != NULL) {
                        return child;
                    }
                }

                Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, true);
                *token = *replacement;
                return program->vars[token->definition_number]->token;
            }
            else if ((token->type == Token_Type::CONCATENATE) || (token->type == Token_Type::CONCATENATE_ARRAY)) {
                for (unsigned int j = 0; j < token->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[j], bounds, scope, is_lhs, is_block_bound);
                    if (child != NULL) {
                        return child;
                    }
                }

                Token *replacement = create_temp_replacement(program, token, bounds, scope, token->type_def_num, true);
                *token = *replacement;
                return program->vars[token->definition_number]->token;
            }
            else {
                for (unsigned int j = 0; j < token->children.size(); ++j) {
                    Token *child = analyze_ports_of_entry(program, token->children[j], bounds, scope, is_lhs, is_block_bound);
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

void Var_Scope_Analyzer::analyze_freeze_resume(Program *program, Token *token, Scope *scope) {

    if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        //scope = token->scope;

        if (token->children.size() > 1) {
            scope = token->children[2]->scope;

            analyze_freeze_resume(program, token->children[2], scope);
        }
    }
    /*
    else if ((token->type == Token_Type::FEATURE_DEF) || (token->type == Token_Type::ACTOR_DEF) ||
            (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {

        if (token->children.size() > 1) {
            analyze_freeze_resume(program, token->children[2], scope);
        }
    }
    */
    else {
        if ((token->type == Token_Type::EXTERN_FUN_DEF) || (token->type == Token_Type::ACTION_CALL)) {

        }

        else if (token->type == Token_Type::CONCATENATE_ARRAY) {
            Token *continuation = new Token(Token_Type::CONTINUATION_SITE);

            Token *copy = new Token(Token_Type::BOOL);
            *copy = *token;
            continuation->children.push_back(copy);
            continuation->start_pos = token->start_pos;
            continuation->end_pos = token->end_pos;

            std::vector<int> push_pop = build_push_pop_list(program, scope, token->start_pos, token->end_pos);


            if (scope->owner->definition_number < 0) {
                throw Compiler_Exception("Internal error finding resume function", token->start_pos, token->end_pos);
            }


            Function_Def *owner = program->funs[scope->owner->definition_number];
            program->var_sites.push_back(push_pop);
            continuation->definition_number = owner->continuation_sites.size();
            owner->continuation_sites.push_back(program->var_sites.size() - 1);
            owner->continuation_sites.push_back(program->var_sites.size() - 1);
            //continuation->definition_number = program->var_sites.size() - 1;

            *token = *continuation;
        }
        else if ((token->children.size() > 1) && (token->children[1]->type == Token_Type::CONCATENATE_ARRAY)) {
            Token *continuation = new Token(Token_Type::CONTINUATION_SITE);

            Token *copy = new Token(Token_Type::BOOL);
            *copy = *token;
            continuation->children.push_back(copy);
            continuation->start_pos = token->start_pos;
            continuation->end_pos = token->end_pos;

            std::vector<int> push_pop = build_push_pop_list(program, scope, token->start_pos, token->end_pos);


            if (scope->owner->definition_number < 0) {
                throw Compiler_Exception("Internal error finding resume function", token->start_pos, token->end_pos);
            }

            Function_Def *owner = program->funs[scope->owner->definition_number];
            program->var_sites.push_back(push_pop);
            continuation->definition_number = owner->continuation_sites.size();
            owner->continuation_sites.push_back(program->var_sites.size() - 1);
            owner->continuation_sites.push_back(program->var_sites.size() - 1);
            //continuation->definition_number = program->var_sites.size() - 1;

            *token = *continuation;
        }

        else if ((token->type == Token_Type::FUN_CALL) || (token->type == Token_Type::METHOD_CALL) || (token->type == Token_Type::NEW_ALLOC) ||
                (token->type == Token_Type::SPAWN_ACTOR)) {
            Function_Def *fun_call;
            if (token->type == Token_Type::FUN_CALL) {
                fun_call = program->funs[token->definition_number];
            }
            else if (token->type == Token_Type::METHOD_CALL) {
                fun_call = program->funs[token->children[1]->definition_number];
                //fun_call = program->funs[token->definition_number];
            }
            else if (token->type == Token_Type::NEW_ALLOC) {
                fun_call = program->funs[token->children[1]->definition_number];
                //fun_call = program->funs[token->definition_number];
            }
            else if (token->type == Token_Type::SPAWN_ACTOR) {
                fun_call = program->funs[token->children[1]->definition_number];
                //fun_call = program->funs[token->definition_number];
            }
            else {
                throw Compiler_Exception("Internal error in freeze resume symbol analysis", token->start_pos, token->end_pos);
            }

            if ((fun_call->external_name == "") && (fun_call->is_internal == false)) {
                Token *continuation = new Token(Token_Type::CONTINUATION_SITE);

                Token *copy = new Token(Token_Type::BOOL);
                *copy = *token;
                continuation->children.push_back(copy);
                continuation->start_pos = token->start_pos;
                continuation->end_pos = token->end_pos;

                std::vector<int> push_pop = build_push_pop_list(program, scope, token->start_pos, token->end_pos);


                if (scope->owner->definition_number < 0) {
                    throw Compiler_Exception("Internal error finding resume function", token->start_pos, token->end_pos);
                }


                Function_Def *owner = program->funs[scope->owner->definition_number];
                program->var_sites.push_back(push_pop);
                continuation->definition_number = owner->continuation_sites.size();
                owner->continuation_sites.push_back(program->var_sites.size() - 1);
                owner->continuation_sites.push_back(program->var_sites.size() - 1);
                //continuation->definition_number = program->var_sites.size() - 1;

                *token = *continuation;
            }
        }
        else if ((token->children.size() > 1) &&
                ((token->type == Token_Type::SYMBOL)) &&
                ((token->children[1]->type == Token_Type::FUN_CALL) || (token->children[1]->type == Token_Type::METHOD_CALL) ||
                        (token->children[1]->type == Token_Type::NEW_ALLOC) || (token->children[1]->type == Token_Type::SPAWN_ACTOR)) ) {

            Function_Def *fun_call;
            if (token->children[1]->type == Token_Type::FUN_CALL) {
                fun_call = program->funs[token->children[1]->definition_number];
            }
            else if (token->children[1]->type == Token_Type::METHOD_CALL) {
                fun_call = program->funs[token->children[1]->children[1]->definition_number];
                //fun_call = program->funs[token->children[1]->definition_number];
            }
            else if (token->children[1]->type == Token_Type::NEW_ALLOC) {
                fun_call = program->funs[token->children[1]->children[1]->definition_number];
            }
            else if (token->children[1]->type == Token_Type::SPAWN_ACTOR) {
                fun_call = program->funs[token->children[1]->children[1]->definition_number];
            }
            else {
                throw Compiler_Exception("Internal error in freeze resume symbol analysis", token->start_pos, token->end_pos);
            }
            if ((fun_call->external_name == "") && (fun_call->is_internal == false)) {
                Token *continuation = new Token(Token_Type::CONTINUATION_SITE);

                Token *copy = new Token(Token_Type::BOOL);
                *copy = *token;
                continuation->children.push_back(copy);
                continuation->start_pos = token->children[1]->start_pos;
                continuation->end_pos = token->children[1]->end_pos;
                std::vector<int> push_pop = build_push_pop_list(program, scope, token->children[1]->start_pos, token->children[1]->end_pos);

                if (scope->owner->definition_number < 0) {
                    throw Compiler_Exception("Internal error finding resume function", token->start_pos, token->end_pos);
                }
                Function_Def *owner = program->funs[scope->owner->definition_number];
                program->var_sites.push_back(push_pop);
                continuation->definition_number = owner->continuation_sites.size();
                owner->continuation_sites.push_back(program->var_sites.size() - 1);
                owner->continuation_sites.push_back(program->var_sites.size() - 1);
                //continuation->definition_number = program->var_sites.size() - 1;

                *token = *continuation;
            }
        }

        else {
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                if (token->children[i]->type != Token_Type::CONTINUATION_SITE) {
                    analyze_freeze_resume(program, token->children[i], scope);
                }
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

                continuation->start_pos = token->start_pos;
                continuation->end_pos = token->end_pos;
                std::vector<int> push_pop = build_push_pop_list(program, scope, token->start_pos, token->end_pos);

                if (scope->owner->definition_number < 0) {
                    throw Compiler_Exception("Internal error finding resume function", token->start_pos, token->end_pos);
                }
                Function_Def *owner = program->funs[scope->owner->definition_number];
                program->var_sites.push_back(push_pop);
                continuation->definition_number = owner->continuation_sites.size();
                owner->continuation_sites.push_back(program->var_sites.size() - 1);
                //continuation->definition_number = program->var_sites.size() - 1;

                // *token = *continuation;

                //token->children[1]->children.insert(token->children[1]->children.begin(), continuation);
                token->children[2]->children.push_back(continuation);
            }

        }

        if (token->type == Token_Type::FOR_BLOCK) {
            Token *continuation = new Token(Token_Type::CONTINUATION_SITE);

            continuation->start_pos = token->start_pos;
            continuation->end_pos = token->end_pos;
            std::vector<int> push_pop = build_push_pop_list(program, scope, token->start_pos, token->end_pos);

            if (scope->owner->definition_number < 0) {
                throw Compiler_Exception("Internal error finding resume function", token->start_pos, token->end_pos);
            }
            Function_Def *owner = program->funs[scope->owner->definition_number];
            program->var_sites.push_back(push_pop);
            continuation->definition_number = owner->continuation_sites.size();
            owner->continuation_sites.push_back(program->var_sites.size() - 1);
            //continuation->definition_number = program->var_sites.size() - 1;

            // *token = *continuation;

            //token->children[1]->children.insert(token->children[1]->children.begin(), continuation);
            token->children.push_back(continuation);
        }
    }
}

std::vector<int> Var_Scope_Analyzer::build_delete_list(Program *program, Scope *scope, Position &position) {
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

void Var_Scope_Analyzer::examine_port_of_exit(Program *program, Token *token, Token *bounds) {
    Function_Def *fd;
    if ((token->type == Token_Type::ACTION_CALL) || (token->type == Token_Type::METHOD_CALL)) {
        fd = program->funs[token->children[1]->definition_number];
    }
    else if (token->type == Token_Type::FUN_CALL) {
        fd = program->funs[token->definition_number];
    }
    else {
        throw Compiler_Exception("Internal error in examine_port_of_exit", token->start_pos, token->end_pos);
    }

    std::vector<int> var_refs;

    if (fd->is_port_of_exit) {
        if (token->children[1]->children.size() > 1) {
            Token *parm = token->children[1]->children[1];
            if (parm->contents == ",") {
                while (parm->contents == ",") {
                    Token *rhs = parm->children[1];
                    Token *lhs = parm->children[0];

                    if ((rhs->contents == ".") && (is_complex_type(program, rhs->type_def_num))) {

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

                            if ((vd->usage_end == bounds->end_pos) && (vd->is_dependent == true) &&
                                    (find(var_refs.begin(), var_refs.end(), rhs->definition_number) == var_refs.end())) {
                                var_refs.push_back(rhs->definition_number);
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

                    if ((lhs->contents == ".") && (is_complex_type(program, lhs->type_def_num))) {
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

                            if ((vd->usage_end == bounds->end_pos) && (vd->is_dependent == true) &&
                                    (find(var_refs.begin(), var_refs.end(), lhs->definition_number) == var_refs.end())) {
                                var_refs.push_back(lhs->definition_number);
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

                        token->children[1]->children[1] = copy_t;
                    }
                }
                else if (parm->type == Token_Type::VAR_CALL) {
                    Var_Def *vd = program->vars[parm->definition_number];
                    if (((vd->is_removed == false) || (vd->is_dependent == false))
                            && (is_complex_var(program, parm->definition_number)))  {

                        if ((vd->usage_end == bounds->end_pos) && (vd->is_dependent == true) &&
                                (find(var_refs.begin(), var_refs.end(), parm->definition_number) == var_refs.end())) {

                            var_refs.push_back(parm->definition_number);
                            vd->is_removed = true;
                        }
                        else {
                            Token *copy_t = new Token(Token_Type::COPY);
                            copy_t->start_pos = parm->start_pos;
                            copy_t->end_pos = parm->end_pos;
                            copy_t->type_def_num = parm->type_def_num;
                            copy_t->children.push_back(parm);

                            token->children[1]->children[1] = copy_t;
                        }
                    }
                }
            }
        }
    }
}

void Var_Scope_Analyzer::examine_equation_for_copy_delete(Program *program, Token *block, Token *token, Token *bounds, Scope *scope, unsigned int &i) {
    //Don't allow any deletion of references
    Function_Def *fd = program->funs[scope->parent->owner->definition_number];

    //Copy any rhs that is not ours
    if ((token->children[1]->type == Token_Type::VAR_DECL) || (token->children[1]->type == Token_Type::VAR_CALL)) {
        Var_Def *vd = program->vars[token->children[1]->definition_number];

        if (((vd->is_removed == false) || (vd->is_dependent == false))
                && (is_complex_var(program, token->children[1]->definition_number))) {

            if ((vd->usage_end == token->children[1]->end_pos) && (vd->is_dependent == true)) {
                vd->is_removed = true;
            }
            else {
                Token *copy_t = new Token(Token_Type::COPY);
                copy_t->start_pos = token->children[1]->start_pos;
                copy_t->end_pos = token->children[1]->end_pos;
                copy_t->type_def_num = token->children[1]->type_def_num;
                copy_t->children.push_back(token->children[1]);

                Token *replace_t = create_temp_replacement(program, token->children[1], bounds, scope, token->children[1]->type_def_num, false);

                Token *eq_t = new Token(Token_Type::SYMBOL);
                eq_t->contents = "=";
                eq_t->children.push_back(replace_t);
                eq_t->children.push_back(copy_t);

                std::vector<int> continuation_site = build_push_pop_list(program, scope, token->start_pos, token->end_pos);
                program->var_sites.push_back(continuation_site);

                Token *cont_t = new Token(Token_Type::CONTINUATION_SITE);
                cont_t->children.push_back(eq_t);
                cont_t->definition_number = fd->continuation_sites.size();

                fd->continuation_sites.push_back(program->var_sites.size() - 1);
                fd->continuation_sites.push_back(program->var_sites.size() - 1);

                block->children.insert(block->children.begin() + i, cont_t);
                ++i;

                token->children[1] = replace_t;
                //token->children[1] = copy_t;
            }
        }
    }
    else if ((token->children[1]->type == Token_Type::FUN_CALL) || (token->children[1]->type == Token_Type::METHOD_CALL) ||
            (token->children[1]->type == Token_Type::ACTION_CALL)) {

        examine_port_of_exit(program, token->children[1], bounds);
    }
    else if (token->children[1]->type == Token_Type::ARRAY_INIT) {
        for (unsigned int j = 0; j < token->children[1]->children.size(); ++j) {
            Token *arg = token->children[1]->children[j];
            if (arg->type == Token_Type::VAR_CALL) {
                Var_Def *vd = program->vars[arg->definition_number];

                if (((vd->is_removed == false) || (vd->is_dependent == false))
                        && (is_complex_var(program, arg->definition_number))) {


                    if ((vd->usage_end == token->children[1]->end_pos) && (vd->is_dependent == true)) {
                        vd->is_removed = true;
                    }
                    else {
                        Token *copy_t = new Token(Token_Type::COPY);
                        copy_t->start_pos = token->children[1]->children[j]->start_pos;
                        copy_t->end_pos = token->children[1]->children[j]->end_pos;
                        copy_t->type_def_num = token->children[1]->children[j]->type_def_num;
                        copy_t->children.push_back(token->children[1]->children[j]);

                        Token *replace_t = create_temp_replacement(program, token->children[1]->children[j], bounds, scope, token->children[1]->children[j]->type_def_num, false);

                        Token *eq_t = new Token(Token_Type::SYMBOL);
                        eq_t->contents = "=";
                        eq_t->children.push_back(replace_t);
                        eq_t->children.push_back(copy_t);

                        std::vector<int> continuation_site = build_push_pop_list(program, scope, token->start_pos, token->end_pos);
                        program->var_sites.push_back(continuation_site);

                        Token *cont_t = new Token(Token_Type::CONTINUATION_SITE);
                        cont_t->children.push_back(eq_t);
                        cont_t->definition_number = fd->continuation_sites.size();

                        fd->continuation_sites.push_back(program->var_sites.size() - 1);
                        fd->continuation_sites.push_back(program->var_sites.size() - 1);

                        block->children.insert(block->children.begin() + i, cont_t);
                        ++i;

                        //token->children[1]->children[j] = copy_t;
                        token->children[1]->children[j] = replace_t;
                    }
                }
            }
        }
    }

    if ((token->children[0]->type == Token_Type::VAR_CALL) || (token->children[0]->type == Token_Type::VAR_DECL)) {
        Var_Def *vd = program->vars[token->children[0]->definition_number];

        /*(vd->is_removed == false) &&*/

        if ( (vd->is_dependent == true) &&
                (is_complex_var(program, token->children[0]->definition_number))) {

            Token *delete_t = new Token(Token_Type::DELETE);
            delete_t->children.push_back(token->children[0]);

            std::vector<int> continuation_site = build_push_pop_list(program, scope, token->start_pos, token->end_pos);
            program->var_sites.push_back(continuation_site);

            Token *cont_t = new Token(Token_Type::CONTINUATION_SITE);
            cont_t->children.push_back(delete_t);
            //cont_t->definition_number = program->var_sites.size() - 1;
            cont_t->definition_number = fd->continuation_sites.size();

            fd->continuation_sites.push_back(program->var_sites.size() - 1);
            fd->continuation_sites.push_back(program->var_sites.size() - 1);

            //unwind_deletion_site(program, block, token, scope, i);
            block->children.insert(block->children.begin() + i, cont_t);

            i += 2;
        }
        else {
            ++i;
        }

        //++i;

        if (token->children[1]->type == Token_Type::REFERENCE_FEATURE) {
            //vd->is_removed = true;
            vd->is_dependent = false;
        }
        else if (token->children[1]->type == Token_Type::COPY) {
            vd->is_dependent = true;
        }
    }
    else {
        ++i;
    }

}

void Var_Scope_Analyzer::unwind_deletion_site(Program *program, Token *block, Token *token, Scope *scope, Position &del_lim, unsigned int &pos) {
    Function_Def *fd = program->funs[scope->parent->owner->definition_number];

    std::vector<int> delete_site = build_delete_list(program, scope, del_lim);
    std::vector<int> continuation_site = build_push_pop_list(program, scope, token->start_pos, token->end_pos);
    continuation_site.insert(continuation_site.begin(), delete_site.begin(), delete_site.end());

    if (delete_site.size() > 0) {
        program->var_sites.push_back(continuation_site);

        //Function_Def *fd = program->funs[token->definition_number];
        //delete_t->children.push_back(child->children[1]);
        //token->children.insert(token->children.begin() + i, delete_t);

        //Token *deletion = new Token(Token_Type::DELETION_SITE);

        for (unsigned int i = 0; i < delete_site.size(); ++i) {
            Token *var_ref = new Token(Token_Type::VAR_CALL);
            Var_Def *vd = program->vars[delete_site[i]];
            var_ref->definition_number = delete_site[i];
            var_ref->type_def_num = vd->type_def_num;
            vd->is_removed = true;

            Token *delete_t = new Token(Token_Type::DELETE);
            delete_t->children.push_back(var_ref);

            Token *cont_t = new Token(Token_Type::CONTINUATION_SITE);
            cont_t->children.push_back(delete_t);
            //cont_t->definition_number = program->var_sites.size() - 1;
            cont_t->definition_number = fd->continuation_sites.size();

            fd->continuation_sites.push_back(program->var_sites.size() - 1);
            fd->continuation_sites.push_back(program->var_sites.size() - 1);

            //token->children[2]->children.push_back(cont_t);

            block->children.insert(block->children.begin() + pos, cont_t);
            ++pos;
        }
        //program->var_sites.push_back(delete_site);
        //deletion->definition_number = program->var_sites.size() - 1;

        //token->children[2]->children.push_back(deletion);
    }

}
void Var_Scope_Analyzer::analyze_copy_delete(Program *program, Token *token, Token *bounds, Scope *scope) {
    if (bounds == NULL) {
        switch (token->type) {
            case (Token_Type::ACTION_CALL) :
            case (Token_Type::ARRAY_CALL) :
            case (Token_Type::CONCATENATE) :
            case (Token_Type::CONCATENATE_ARRAY) :
            case (Token_Type::CONSTRUCTOR_CALL) :
            case (Token_Type::FUN_CALL) :
            case (Token_Type::METHOD_CALL) :
            case (Token_Type::NEW_ALLOC) :
            case (Token_Type::RETURN_CALL) :
            case (Token_Type::SYMBOL) :
                bounds = token;
            break;
            default:
                break;
        }
    }

    Function_Def *fd = NULL;

    if (scope != NULL) {
        fd = program->funs[scope->owner->definition_number];
    }

    if ((token->type == Token_Type::ACTION_DEF) || (token->type == Token_Type::FUN_DEF)) {
        //scope = token->scope;
        scope = token->children[2]->scope;

        analyze_copy_delete(program, token->children[2], bounds, scope);
        if (token->children[2]->children.size() > 0) {
            if (token->children[2]->children[token->children[2]->children.size() - 1]->type != Token_Type::RETURN_CALL) {
                unsigned int i = token->children[2]->children.size();
                unwind_deletion_site(program, token->children[2], token, scope, token->end_pos, i);
            }
        }
    }
    /*
    else if ((token->type == Token_Type::FEATURE_DEF) || (token->type == Token_Type::ACTOR_DEF) ||
            (token->type == Token_Type::ISOLATED_ACTOR_DEF)) {
        analyze_copy_delete(program, token->children[2], bounds, scope);
    }
    */
    else {
        if (token->type == Token_Type::BLOCK) {
            unsigned int i = 0;
            while (i < token->children.size()) {
                Token *child = token->children[i];

                switch (child->type) {
                    case (Token_Type::ACTION_CALL) :
                    case (Token_Type::ARRAY_CALL) :
                    case (Token_Type::CONCATENATE) :
                    case (Token_Type::CONCATENATE_ARRAY) :
                    case (Token_Type::CONSTRUCTOR_CALL) :
                    case (Token_Type::FUN_CALL) :
                    case (Token_Type::METHOD_CALL) :
                    case (Token_Type::NEW_ALLOC) :
                    case (Token_Type::RETURN_CALL) :
                    case (Token_Type::SYMBOL) :
                        bounds = child;
                    break;
                    default:
                        bounds = NULL;
                }

                //While handles analysis differently, so we check for that here
                if ((child->type != Token_Type::WHILE_BLOCK) && (child->type != Token_Type::FOR_BLOCK)) {
                    analyze_copy_delete(program, token->children[i], bounds, scope);
                }

                if (child->type == Token_Type::CONTINUATION_SITE) {
                    unwind_deletion_site(program, token, child, scope, child->start_pos, i);

                    if (child->children.size() > 0) {
                        if (child->children[0]->contents == "=") {
                            bounds = child->children[0];
                            examine_equation_for_copy_delete(program, token, child->children[0], bounds, scope, i);
                        }
                        else if ((child->children[0]->type == Token_Type::FUN_CALL) ||
                                (child->children[0]->type == Token_Type::METHOD_CALL) ||
                                (child->children[0]->type == Token_Type::ACTION_CALL)) {
                            examine_port_of_exit(program, child->children[0], bounds);
                            ++i;
                        }

                        else {
                            ++i;
                        }

                    }
                    else {
                        ++i;
                    }
                }
                else if ((child->type == Token_Type::WHILE_BLOCK) || (child->type == Token_Type::FOR_BLOCK) || (child->type == Token_Type::IF_BLOCK)) {
                    unwind_deletion_site(program, token, child, scope, child->start_pos, i);
                    std::vector<int> delete_site = build_delete_list(program, scope, child->start_pos);
                    analyze_copy_delete(program, token->children[i], bounds, scope);
                    ++i;
                }
                else if (child->type == Token_Type::RETURN_CALL) {
                    if (child->children.size() > 1) {
                        if (child->children[1]->type == Token_Type::VAR_CALL) {

                            Var_Def *vd = program->vars[child->children[1]->definition_number];
                            if (vd->is_dependent == true) {
                                vd->is_removed = true;
                                unwind_deletion_site(program, token, child, scope, child->end_pos, i);
                                ++i;

                                if (vd->usage_end == child->end_pos) {
                                    vd->is_removed = true;
                                }
                                else {
                                    vd->is_removed = false;
                                }
                            }
                            else {
                                if (is_complex_type(program, child->children[1]->type_def_num)) {
                                    Token *copy_t = new Token(Token_Type::COPY);
                                    copy_t->start_pos = child->children[1]->start_pos;
                                    copy_t->end_pos = child->children[1]->end_pos;
                                    copy_t->type_def_num = child->children[1]->type_def_num;
                                    copy_t->children.push_back(child->children[1]);

                                    Token *replace_t = create_temp_replacement(program, child->children[1], bounds, scope, child->children[1]->type_def_num, false);

                                    Token *eq_t = new Token(Token_Type::SYMBOL);
                                    eq_t->contents = "=";
                                    eq_t->children.push_back(replace_t);
                                    eq_t->children.push_back(copy_t);

                                    std::vector<int> continuation_site = build_push_pop_list(program, scope, token->start_pos, token->end_pos);
                                    program->var_sites.push_back(continuation_site);

                                    Token *cont_t = new Token(Token_Type::CONTINUATION_SITE);
                                    cont_t->children.push_back(eq_t);
                                    cont_t->definition_number = fd->continuation_sites.size();

                                    fd->continuation_sites.push_back(program->var_sites.size() - 1);
                                    fd->continuation_sites.push_back(program->var_sites.size() - 1);

                                    token->children.insert(token->children.begin() + i, cont_t);
                                    i += 2;

                                    //child->children[1] = copy_t;
                                    child->children[1] = replace_t;
                                }
                                unwind_deletion_site(program, token, child, scope, child->end_pos, i);
                                ++i;
                            }
                        }
                        else {
                            ++i;
                        }
                    }
                    else {
                        unwind_deletion_site(program, token, child, scope, child->end_pos, i);
                    }
                }
                else if ((child->type == Token_Type::ACTION_CALL) || (child->type == Token_Type::METHOD_CALL)
                        || (child->type == Token_Type::FUN_CALL)) {

                    examine_port_of_exit(program, child, bounds);
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

                                Token *replace_t = create_temp_replacement(program, child->children[1], bounds, scope, child->children[1]->type_def_num, false);

                                Token *eq_t = new Token(Token_Type::SYMBOL);
                                eq_t->contents = "=";
                                eq_t->children.push_back(replace_t);
                                eq_t->children.push_back(copy_t);

                                std::vector<int> continuation_site = build_push_pop_list(program, scope, token->start_pos, token->end_pos);
                                program->var_sites.push_back(continuation_site);

                                Token *cont_t = new Token(Token_Type::CONTINUATION_SITE);
                                cont_t->children.push_back(eq_t);
                                cont_t->definition_number = fd->continuation_sites.size();

                                fd->continuation_sites.push_back(program->var_sites.size() - 1);
                                fd->continuation_sites.push_back(program->var_sites.size() - 1);

                                token->children.insert(token->children.begin() + i, cont_t);
                                i += 2;

                                //child->children[1] = copy_t;
                                child->children[1] = replace_t;
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

                            Token *cont_t = new Token(Token_Type::CONTINUATION_SITE);
                            cont_t->children.push_back(delete_t);
                            cont_t->definition_number = fd->continuation_sites.size();

                            fd->continuation_sites.push_back(program->var_sites.size() - 1);
                            fd->continuation_sites.push_back(program->var_sites.size() - 1);

                            //token->children.insert(token->children.begin() + i, delete_t);
                            token->children.insert(token->children.begin() + i, cont_t);

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
                    examine_equation_for_copy_delete(program, token, child, bounds, scope, i);
                }
                else {
                    ++i;
                }
            }
        }
        else {
            for (unsigned int i = 0; i < token->children.size(); ++i) {
                analyze_copy_delete(program, token->children[i], bounds, scope);
            }
        }
    }
}
