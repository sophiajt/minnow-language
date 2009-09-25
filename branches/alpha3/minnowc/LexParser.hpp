// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LEXPARSE_HPP_
#define LEXPARSE_HPP_

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <stdlib.h>

#include "Common.hpp"
#include "Program.hpp"

class Lex_Parser {
    Program *program;
    std::vector<std::string> *use_list;

public:
    bool lexparse_whitespace(std::string::iterator &curr, std::string::iterator &end, Position &p);

    bool lexparse_comment(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_number(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_id(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_parens(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_square_brackets(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_quoted_string(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_single_quoted_string(std::string::iterator &curr, std::string::iterator &end, Position &p);

    Token *lexparse_def(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_library(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_extern(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_action(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_isolated(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_new(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_spawn(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_namespace(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_use(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_actor(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_feature(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_return(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_if(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_try(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_while(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_enum(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_for(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);

    Token *lexparse_reserved(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_primary(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_operator(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_binop(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *lhs, unsigned int expr_prec);
    Token *lexparse_eol(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_expression(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_expression(std::string::iterator &curr, std::string::iterator &end, Position &p, unsigned int precedence);
    Token *lexparse_block(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_ifblock(std::string::iterator &curr, std::string::iterator &end, Position &p, Token **continuation);
    Token *lexparse_tryblock(std::string::iterator &curr, std::string::iterator &end, Position &p, Token **continuation);

    Token *lexparse_file(std::string &name, std::string &contents, std::vector<std::string> *included_files);

    Lex_Parser(Program *p) {
        program = p;
    }
};

#endif /* LEXPARSE_HPP_ */
