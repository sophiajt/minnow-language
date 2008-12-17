// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <stdlib.h>

#include "LexParser.hpp"

#define NAMESPACE_PREC 110

void debug_print(Token *token, std::string prepend) {
    if (token->contents == "") {
        std::cout << prepend << "(" << token->type << " " << token->start_pos.line << ","
            << token->start_pos.col << " - " << token->end_pos.line
            << "," << token->end_pos.col << ")" << "[" << token->start_pos.filename << "]" << std::endl;
    }
    else {
        std::cout << prepend << token->contents << " (" << token->type << " " << token->start_pos.line << ","
            << token->start_pos.col << " - " << token->end_pos.line
            << "," << token->end_pos.col << ")" << "[" << token->start_pos.filename << "]" << std::endl;
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        debug_print(token->children[i], "   " + prepend);
    }
}

bool is_symbol(char c) {
    if (strchr("!@#$%^&*<>,.?/:|\\-+=~`", c) != NULL) {
        return true;
    }
    else {
        return false;
    }
}

unsigned int get_precedence(Token *token) {
    if (token->contents == ",") {
        return 10;
    }
    else if (token->contents == "::") {
        return 20;
    }
    else if (token->contents == "=") {
        return 30;
    }
    else if ((token->contents == "+=") || (token->contents == "-=")) {
        return 40;
    }
    else if ((token->contents == "<+") || (token->contents == "+>")) {
        return 50;
    }
    else if ((token->contents == "==") || (token->contents == ">=") || (token->contents == "<=") || (token->contents == "!=") ||
            (token->contents == ">") || (token->contents == "<")) {
        return 60;
    }
    else if ((token->contents == "&&") || (token->contents == "||")) {
        return 70;
    }
    else if ((token->contents == "+") || (token->contents == "-")) {
        return 80;
    }
    else if ((token->contents == "*") || (token->contents == "/")) {
        return 90;
    }
    else if (token->contents == "**") {
        return 100;
    }
    else if ((token->contents == ":")) {
        return 110;
    }
    else if (token->contents == ".") {
        return 120;
    }
    return 0;
}

bool Lex_Parser::lexparse_comment(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    lexparse_whitespace(curr, end, p);
    if (curr == end) return false;

    if (*curr == '/') {
        std::string::iterator orig_iter = curr;
        Position orig_p = p;
        ++curr;
        ++p.col;
        if ((curr != end) && (*curr == '/')) {
            //single line comment
            while ((curr != end) && (*curr != '\n')) {
                ++curr;
                ++p.col;
            }
            return true;
        }
        else if ((curr != end) && (*curr == '*')) {
            char prev = 0;
            //multi-line comment
            while (curr != end) {
                ++p.col;
                if (*curr == '\n') {
                    p.col = 1;
                    ++p.line;
                }
                ++curr;
                if (curr != end) {
                    if ((prev == '*') && (*curr == '/')) {
                        ++curr;
                        ++p.col;
                        return true;
                    }
                    prev = *curr;
                }
            }
            throw Compiler_Exception("Multi-line comment not closed", orig_p);
        }
        else {
            curr = orig_iter;
            p = orig_p;
            return false;
        }
    }
    return false;
}

Token *Lex_Parser::lexparse_number(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    lexparse_whitespace(curr, end, p);
    if (curr == end) return NULL;
    Position start_p;

    Position initial_p = p;
    std::string::iterator initial_c = curr;

    if (*curr == '-') {
        ++p.col;
        ++curr;
    }

    if (((*curr >= '0') && (*curr <= '9'))) {
        //starting on a number
        bool have_decimal = false;
        std::string::iterator start = initial_c;
        Position start_p = initial_p;
        while (((*curr >= '0') && (*curr <= '9')) || (*curr == '.')) {
            if (*curr == '.') {
                if (have_decimal) {
                    //throw Compiler_Exception("Multiple decimal points.  For functions use (0.0).function() format", p);
                    break;
                }
                have_decimal = true;
            }
            ++curr;
            ++p.col;
        }

        --curr;
        if (*curr == '.') {
            //if we end in a point, we're still not a float since it's probably a function call
            have_decimal = false;
        }
        else {
            ++curr;
        }

        std::string val(start, curr);

        Token *token;
        if (have_decimal) {
            token = new Token(Token_Type::FLOAT, val, start_p, p);
        }
        else {
            token = new Token(Token_Type::INT, val, start_p, p);
        }
        return token;
    }
    else {
        p = initial_p;
        curr = initial_c;

        return NULL;
    }
}

Token *Lex_Parser::lexparse_id(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    lexparse_whitespace(curr, end, p);
    if (curr == end) return NULL;

    if (((*curr >= 'A') && (*curr <= 'Z')) || ((*curr >= 'a') && (*curr <= 'z')) || (*curr == '_')) {
        //starting on a number
        std::string::iterator start = curr;
        Position start_p = p;
        while (((*curr >= 'A') && (*curr <= 'Z')) || ((*curr >= 'a') && (*curr <= 'z')) || (*curr == '_') || ((*curr >= '0') && (*curr <= '9'))) {
            ++curr;
            ++p.col;
        }
        std::string val(start, curr);
        Token *token = new Token(Token_Type::ID, val, start_p, p);
        return token;
    }
    else {
        return NULL;
    }
}

Token *Lex_Parser::lexparse_parens(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    lexparse_whitespace(curr, end, p);
    Token *t = lexparse_expression(curr, end, p);
    lexparse_whitespace(curr, end, p);

    if ((curr == end) || (*curr != ')')) {
        throw Compiler_Exception("Unclosed parentheses", p);
    }
    else {
        ++p.col;
        ++curr;
    }

    return t;
}

Token *Lex_Parser::lexparse_square_brackets(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    lexparse_whitespace(curr, end, p);
    Token *t = lexparse_expression(curr, end, p);
    lexparse_whitespace(curr, end, p);

    if ((curr == end) || (*curr != ']')) {
        throw Compiler_Exception("Unclosed square brackets", p);
    }
    else {
        ++p.col;
        ++curr;
    }

    return t;
}

Token *Lex_Parser::lexparse_quoted_string(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    std::string contents = "";
    Position start = p;
    while ((curr != end) && (*curr != '\"')) {

        if (*curr == '\\') {
            contents.append(1, *curr);
            if (curr == end) {
                throw Compiler_Exception("Unclosed quotation", p);
            }
            ++curr;
            ++p.col;
        }
        contents.append(1, *curr);
        ++curr;
        ++p.col;
    }
    if (curr == end) {
        throw Compiler_Exception("Unclosed quotation", p);
    }
    else {
        ++curr;
        ++p.col;
    }

    Token *t = new Token(Token_Type::QUOTED_STRING_CONST, contents, start, p);
    return t;
}

Token *Lex_Parser::lexparse_single_quoted_string(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    std::string contents = "";
    Position start = p;
    while ((curr != end) && (*curr != '\'')) {
        if (*curr == '\\') {
            contents.append(1, *curr);
            if (curr == end) {
                throw Compiler_Exception("Unclosed single quote", p);
            }
            ++curr;
            ++p.col;
        }

        contents.append(1, *curr);

        ++curr;
        ++p.col;
    }
    if (curr == end) {
        throw Compiler_Exception("Unclosed single quote", p);
    }
    else {
        ++curr;
        ++p.col;
    }

    Token *t = new Token(Token_Type::SINGLE_QUOTED_STRING, contents, start, p);
    return t;
}

Token *Lex_Parser::lexparse_reserved(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id) {
    if (id->contents == "def") {
        Token *t = lexparse_expression(curr, end, p);
        switch (t->type) {
            case (Token_Type::SYMBOL) : //for return types
            case (Token_Type::FUN_CALL) :
            case (Token_Type::ID) : {
                Token *fun_def = new Token(Token_Type::FUN_DEF, id->start_pos, t->end_pos);
                fun_def->children.push_back(id);
                fun_def->children.push_back(t);

                Token *name = t;
                while (name->children.size() > 0) {
                    name = name->children[0];
                }
                fun_def->contents = name->contents;

                Token *block = lexparse_block(curr, end, p);
                fun_def->children.push_back(block);
                fun_def->end_pos = p;

                return fun_def;
            }
            break;
            default:
                throw Compiler_Exception("'def' not followed by function heading", p);
        }
    }
    else if (id->contents == "extern") {
        delete id;
        Token *id = lexparse_id(curr, end, p);
        if (id->contents != "def") {
            throw Compiler_Exception("Extern not followed by 'def' keyword", id->start_pos);
        }
        else {
            Token *t = lexparse_expression(curr, end, p);
            switch (t->type) {
                case (Token_Type::SYMBOL) : //for return types
                case (Token_Type::FUN_CALL) :
                case (Token_Type::ID) : {
                    Token *fun_def = new Token(Token_Type::EXTERN_FUN_DEF, id->start_pos, t->end_pos);
                    fun_def->children.push_back(id);
                    fun_def->children.push_back(t);

                    Token *name = t;
                    while (name->children.size() > 0) {
                        name = name->children[0];
                    }
                    fun_def->contents = name->contents;
                    fun_def->end_pos = p;

                    return fun_def;
                }
                break;
                default:
                    throw Compiler_Exception("'def' not followed by function heading", p);
            }

        }
    }
    else if (id->contents == "action") {
        Token *t = lexparse_expression(curr, end, p);
        switch (t->type) {
            case (Token_Type::SYMBOL) : //for return types
            case (Token_Type::FUN_CALL) :
            case (Token_Type::ID) : {
                Token *action_def = new Token(Token_Type::ACTION_DEF, id->start_pos, t->end_pos);
                action_def->children.push_back(id);
                action_def->children.push_back(t);

                Token *name = t;
                while (name->children.size() > 0) {
                    name = name->children[0];
                }
                action_def->contents = name->contents;

                Token *block = lexparse_block(curr, end, p);
                action_def->children.push_back(block);
                action_def->end_pos = p;

                return action_def;
            }
            break;
            default:
                throw Compiler_Exception("'action' not followed by action heading", p);
        }
    }
    else if (id->contents == "isolated") {
        delete id;
        Token *action_def = lexparse_primary(curr, end, p);
        if (action_def->type != Token_Type::ACTOR_DEF) {
            std::cout << "TYPE: " << action_def->type << std::endl;
            throw Compiler_Exception("Isolated not followed by action definition", action_def->start_pos);
        }
        action_def->type = Token_Type::ISOLATED_ACTOR_DEF;

        return action_def;
    }
    else if ((id->contents == "true") || (id->contents == "false")) {
        Token *bool_val = id;
        bool_val->type = Token_Type::BOOL;

        return bool_val;
    }
    else if (id->contents == "this") {
        Token *this_val = id;
        this_val->type = Token_Type::THIS;

        return this_val;
    }
    else if (id->contents == "new") {
        Token *t = lexparse_expression(curr, end, p, NAMESPACE_PREC);
        switch (t->type) {
            case (Token_Type::FUN_CALL) :
            case (Token_Type::SYMBOL) :
            case (Token_Type::ARRAY_CALL) :
            case (Token_Type::ID) : {
                Token *new_alloc = new Token(Token_Type::NEW_ALLOC, id->start_pos, t->end_pos);
                new_alloc->children.push_back(id);
                new_alloc->children.push_back(t);

                return new_alloc;
            }
            break;
            default:
                throw Compiler_Exception("'new' not followed by feature or container", p);
        }
    }
    else if (id->contents == "spawn") {
        Token *t = lexparse_expression(curr, end, p, NAMESPACE_PREC);
        switch (t->type) {
            case (Token_Type::FUN_CALL) :
            case (Token_Type::SYMBOL) :
            case (Token_Type::ID) : {
                Token *spawn_actor = new Token(Token_Type::SPAWN_ACTOR, id->start_pos, t->end_pos);
                spawn_actor->children.push_back(id);
                spawn_actor->children.push_back(t);

                return spawn_actor;
            }
            break;
            default:
                throw Compiler_Exception("'new' not followed by feature or container", p);
        }
    }
    else if (id->contents == "namespace") {
        Token *t = lexparse_expression(curr, end, p);
        switch (t->type) {
            case (Token_Type::SYMBOL) : //for return types
            case (Token_Type::ID) : {
                Token *namespace_start = new Token(Token_Type::NAMESPACE, id->start_pos, t->end_pos);
                namespace_start->children.push_back(id);
                namespace_start->children.push_back(t);

                return namespace_start;
            }
            break;
            default:
                throw Compiler_Exception("'namespace' not followed by name", p);
        }
    }
    else if (id->contents == "use") {
        Token *t = lexparse_expression(curr, end, p);
        if (t != NULL) {
            Token *use_call = new Token(Token_Type::USE_CALL, id->start_pos, t->end_pos);
            use_call->children.push_back(id);
            use_call->children.push_back(t);

            return use_call;
        }
        else {
            throw Compiler_Exception("'use' not followed by file reference", p);
        }
    }
    else if (id->contents == "actor") {
        Token *t = lexparse_primary(curr, end, p);
        switch (t->type) {
            case (Token_Type::ID) : {
                Token *actor_def = new Token(Token_Type::ACTOR_DEF, id->start_pos, t->end_pos);
                actor_def->children.push_back(id);
                actor_def->children.push_back(t);

                Token *name = t;
                while (name->children.size() > 0) {
                    name = name->children[0];
                }
                actor_def->contents = name->contents;

                Token *block = lexparse_block(curr, end, p);
                actor_def->children.push_back(block);
                actor_def->end_pos = p;

                return actor_def;
            }
            break;
            default:
                throw Compiler_Exception("'actor' not followed by actor name", p);
        }
    }
    else if (id->contents == "feature") {
        Token *t = lexparse_primary(curr, end, p);
        switch (t->type) {
            case (Token_Type::ID) : {
                Token *feature_def = new Token(Token_Type::FEATURE_DEF, id->start_pos, t->end_pos);
                feature_def->children.push_back(id);
                feature_def->children.push_back(t);

                Token *name = t;
                while (name->children.size() > 0) {
                    name = name->children[0];
                }
                feature_def->contents = name->contents;

                Token *block = lexparse_block(curr, end, p);
                feature_def->children.push_back(block);
                feature_def->end_pos = p;

                return feature_def;
            }
            break;
            default:
                throw Compiler_Exception("'feature' not followed by feature name", p);
        }
    }
    else if (id->contents == "return") {
        Token *return_call;
        Token *t = lexparse_expression(curr, end, p);

        if (t != NULL) {
            return_call = new Token(Token_Type::RETURN_CALL, id->start_pos, t->end_pos);
            return_call->children.push_back(id);

            return_call->children.push_back(t);
        }
        else {
            return_call = new Token(Token_Type::RETURN_CALL, id->start_pos, id->end_pos);
            return_call->children.push_back(id);
        }

        return return_call;
    }
    else if (id->contents == "elseif") {
        return id;
    }
    else if (id->contents == "else") {
        return id;
    }
    else if (id->contents == "if") {
        Token *t = lexparse_expression(curr, end, p);
        switch (t->type) {
            case (Token_Type::SYMBOL) : //for return types
            case (Token_Type::BOOL) :
            case (Token_Type::ID) : {
                Token *if_block = new Token(Token_Type::IF_BLOCK, id->start_pos, t->end_pos);
                if_block->children.push_back(id);

                //push condition into its own block
                Token *if_cond = new Token(Token_Type::BLOCK, t->start_pos, t->end_pos);
                if_cond->children.push_back(t);
                if_block->children.push_back(if_cond);

                /*
                Token *name = t;
                while (name->children.size() > 0) {
                    name = name->children[0];
                }
                if_block->contents = name->contents;
                */

                Token *continuation = NULL;
                Token *block = lexparse_ifblock(curr, end, p, &continuation);
                if_block->children.push_back(block);

                while (continuation != NULL) {
                    if (continuation->contents == "elseif") {
                        Token *t = lexparse_expression(curr, end, p);
                        switch (t->type) {
                            case (Token_Type::SYMBOL) : //for return types
                            case (Token_Type::ID) : {
                                Token *elseif_block = new Token(Token_Type::ELSEIF_BLOCK, continuation->start_pos, t->end_pos);
                                elseif_block->children.push_back(continuation);
                                Token *elseif_cond = new Token(Token_Type::BLOCK, t->start_pos, t->end_pos);
                                elseif_cond->children.push_back(t);
                                elseif_block->children.push_back(elseif_cond);

                                //elseif_block->children.push_back(t);

                                /*
                                Token *name = t;
                                while (name->children.size() > 0) {
                                    name = name->children[0];
                                }
                                elseif_block->contents = name->contents;
                                */

                                continuation = NULL;
                                Token *block = lexparse_ifblock(curr, end, p, &continuation);
                                elseif_block->children.push_back(block);
                                elseif_block->end_pos = p;
                                if_block->children.push_back(elseif_block);
                                if_block->end_pos = p;
                            }
                            break;
                            default:
                                continuation = NULL;
                        }
                    }
                    else if (continuation->contents == "else") {
                        Token *block = lexparse_block(curr, end, p);
                        Token *else_block = new Token(Token_Type::ELSE_BLOCK, continuation->start_pos, continuation->end_pos);
                        else_block->children.push_back(continuation);
                        else_block->children.push_back(block);

                        if_block->children.push_back(else_block);
                        if_block->end_pos = p;
                        continuation = NULL;
                    }
                    else {
                        continuation = NULL;
                    }
                }

                return if_block;
            }
            break;
            default:
                throw Compiler_Exception("'if' not followed by condition", p);
        }
    }
    else if (id->contents == "while") {
        Token *t = lexparse_expression(curr, end, p);
        switch (t->type) {
            case (Token_Type::SYMBOL) : //for return types
            case (Token_Type::BOOL) :
            case (Token_Type::ID) : {
                Token *while_block = new Token(Token_Type::WHILE_BLOCK, id->start_pos, t->end_pos);
                while_block->children.push_back(id);

                //push condition into its own block
                Token *while_cond = new Token(Token_Type::BLOCK, t->start_pos, t->end_pos);
                while_cond->children.push_back(t);
                while_block->children.push_back(while_cond);

                Token *name = t;
                while (name->children.size() > 0) {
                    name = name->children[0];
                }
                while_block->contents = name->contents;

                Token *block = lexparse_block(curr, end, p);
                while_block->children.push_back(block);
                while_block->end_pos = p;

                return while_block;
            }
            break;
            default:
                throw Compiler_Exception("'while' not followed by condition", p);
        }
    }

    return NULL;
}

Token *Lex_Parser::lexparse_primary(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    lexparse_whitespace(curr, end, p);
    if (curr == end) return NULL;

    Token *result = NULL;

    if ( (result = lexparse_number(curr, end, p)) ) {
        return result;
    }
    else if ( (result = lexparse_id(curr, end, p)) ) {
        //If it's an id, let's see if it's also a function.  We might want to check for certain reserved words too.
        lexparse_whitespace(curr, end, p);

        if (curr != end) {
            //first check reserved words, then check call modes (these do not overlap)
            Token *t = lexparse_reserved(curr, end, p, result);
            if (t != NULL) {
                return t;
            }
            else if (*curr == '(') {
                ++curr;
                ++p.col;
                Token *fun_args = lexparse_parens(curr, end, p);
                Token *fun_call = new Token(Token_Type::FUN_CALL, result->start_pos, p);
                fun_call->children.push_back(result);
                if (fun_args != NULL) {
                    fun_call->children.push_back(fun_args);
                }
                return fun_call;
            }
            else if (*curr == '[') {
                ++curr;
                ++p.col;
                Token *array_args = lexparse_square_brackets(curr, end, p);
                Token *array_call = new Token(Token_Type::ARRAY_CALL, result->start_pos, p);
                array_call->children.push_back(result);
                if (array_args != NULL) {
                    array_call->children.push_back(array_args);
                }
                while (*curr == '[') {
                    ++curr;
                    ++p.col;
                    array_args = lexparse_square_brackets(curr, end, p);
                    Token *next_call = new Token(Token_Type::ARRAY_CALL, result->start_pos, p);
                    next_call->children.push_back(array_call);
                    if (array_args != NULL) {
                        next_call->children.push_back(array_args);
                    }
                    array_call = next_call;
                }
                return array_call;
            }
        }
        return result;
    }
    else if (*curr == '(') {
        ++curr;
        ++p.col;
        return lexparse_parens(curr, end, p);
    }
    else if (*curr == '"') {
        ++curr;
        ++p.col;
        return lexparse_quoted_string(curr, end, p);
    }
    else if (*curr == '\'') {
        ++curr;
        ++p.col;
        return lexparse_single_quoted_string(curr, end, p);
    }
    return NULL;
}

Token *Lex_Parser::lexparse_operator(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    lexparse_whitespace(curr, end, p);
    if (curr == end) return NULL;

    //Skip all the comments
    while (lexparse_comment(curr, end, p));

    if (is_symbol(*curr)) {
        std::string::iterator start = curr;
        Position start_p = p;

        while (is_symbol(*curr)) {
            if (*curr == '/') {
                //Make sure it's not a comment
                std::string::iterator end_iter = curr;
                Position end_p = p;
                if (lexparse_comment(curr, end, p)) {
                    std::string val(start, end_iter);
                    Token *token = new Token(Token_Type::SYMBOL, val, start_p, end_p);
                    return token;
                }
            }

            ++curr;
            ++p.col;
        }
        std::string val(start, curr);
        Token *token = new Token(Token_Type::SYMBOL, val, start_p, p);
        return token;
    }
    else {
        return NULL;
    }
}

Token *Lex_Parser::lexparse_binop(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *lhs, unsigned int expr_prec) {
    std::string::iterator prev_tok;
    Position prev_pos;

    while (true) {
        if (curr == end) return lhs;

        prev_pos = p;
        prev_tok = curr;
        Token *op = lexparse_operator(curr, end, p);
        if (op == NULL) return lhs;

        unsigned int tok_prec = get_precedence(op);
        if (tok_prec < expr_prec) {
            p = prev_pos;
            curr = prev_tok;
            return lhs;
        }

        Token *rhs = lexparse_primary(curr, end, p);

        if (rhs == NULL) {
            throw Compiler_Exception("Incomplete expression", p);
        }

        prev_pos = p;
        prev_tok = curr;
        Token *next_op = lexparse_operator(curr, end, p);
        p = prev_pos;
        curr = prev_tok;

        if (next_op != NULL) {
            unsigned int next_prec = get_precedence(next_op);

            if (tok_prec < next_prec) {
                rhs = lexparse_binop(curr, end, p, rhs, tok_prec+1);
            }
            else if ((tok_prec == next_prec) && (next_op->contents == "=")) {
                rhs = lexparse_binop(curr, end, p, rhs, tok_prec);
            }
        }

        op->children.push_back(lhs);
        op->children.push_back(rhs);
        op->start_pos = lhs->start_pos;
        op->end_pos = rhs->end_pos;
        lhs = op;
    }
}

bool Lex_Parser::lexparse_whitespace(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    if (curr == end)
        return false;

    if ((curr != end) && ((*curr == ' ') || (*curr == '\t'))) {
        while ((curr != end) && ((*curr == ' ') || (*curr == '\t'))) {
            ++p.col;
            ++curr;
        }
        return true;
    }
    else {
        return false;
    }
}

Token *Lex_Parser::lexparse_eol(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    if (curr == end)
        return false;

    if (*curr == '\r') {
        if (curr != end) {
            ++curr;
        }
    }
    if ((*curr == '\n') || (*curr == ';')) {
        Token *eol = new Token(Token_Type::EOL, "eol", p, p);
        p.col = 1;
        ++p.line;

        ++curr;
        return eol;
    }
    else {
        return NULL;
    }
}

Token *Lex_Parser::lexparse_expression(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    Token *prim = lexparse_primary(curr, end, p);
    if (prim != NULL) {
        Token *binop = lexparse_binop(curr, end, p, prim, 0);
        return binop;
    }

    return NULL;
}

Token *Lex_Parser::lexparse_expression(std::string::iterator &curr, std::string::iterator &end, Position &p, unsigned int precedence) {
    Token *prim = lexparse_primary(curr, end, p);
    if (prim != NULL) {
        Token *binop = lexparse_binop(curr, end, p, prim, precedence);
        return binop;
    }

    return NULL;
}

Token *Lex_Parser::lexparse_block(std::string::iterator &curr, std::string::iterator &end, Position &p) {
    Token *block = new Token(Token_Type::BLOCK);
    block->start_pos = p;

    while (curr != end) {
        if (lexparse_comment(curr, end, p)) {
            continue;
        }
        Token *child = lexparse_expression(curr, end, p);
        if (child != NULL) {
            if (child->contents == "end") {
                delete child;
                return block;
            }
            block->children.push_back(child);
            continue;
        }
        if (lexparse_whitespace(curr, end, p)) {
            continue;
        }
        child = lexparse_eol(curr, end, p);
        if (child != NULL){
            block->children.push_back(child);
            continue;
        }

        throw Compiler_Exception("Can not parse element", p);
    }

    block->end_pos = p;
    return block;
}

Token *Lex_Parser::lexparse_ifblock(std::string::iterator &curr, std::string::iterator &end, Position &p, Token **continuation) {
    Token *block = new Token(Token_Type::BLOCK);
    block->start_pos = p;

    while (curr != end) {
        if (lexparse_comment(curr, end, p)) {
            continue;
        }
        Token *child = lexparse_expression(curr, end, p);
        if (child != NULL) {
            if (child->contents == "end") {
                delete child;
                return block;
            }
            else if (child->contents == "elseif") {
                *continuation = child;
                return block;
            }
            else if (child->contents == "else") {
                *continuation = child;
                return block;
            }

            block->children.push_back(child);
            continue;
        }
        if (lexparse_whitespace(curr, end, p)) {
            continue;
        }
        child = lexparse_eol(curr, end, p);
        if (child != NULL){
            block->children.push_back(child);
            continue;
        }

        throw Compiler_Exception("Can not parse element", p);
    }

    block->end_pos = p;
    return block;
}

Token *Lex_Parser::lexparse_file(std::string &name, std::string &contents) {
    Position p;

    //std::cout << "File contents: " << contents << std::endl;

    char *filename = new char[name.length()+1];

    filename = strcpy(filename, name.c_str());

    p.filename = filename;
    //std::cout << "Filename: " << p.filename << std::endl;

    Token *token = new Token(Token_Type::FILE, name);
    std::string::iterator curr = contents.begin(), end = contents.end();
    token->start_pos = p;
    token->end_pos = p;

    while (curr != end) {
        Token *block = lexparse_block(curr, end, p);
        if (token != NULL) {
            token->children.push_back(block);
        }
    }

    return token;
}

