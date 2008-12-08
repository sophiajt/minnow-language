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

void debug_print(Token *token, std::string prepend);

class Lex_Parser {

public:
    bool lexparse_whitespace(std::string::iterator &curr, std::string::iterator &end, Position &p);

    bool lexparse_comment(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_number(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_id(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_parens(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_square_brackets(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_quoted_string(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_single_quoted_string(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_reserved(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *id);
    Token *lexparse_primary(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_operator(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_binop(std::string::iterator &curr, std::string::iterator &end, Position &p, Token *lhs, unsigned int expr_prec);
    Token *lexparse_eol(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_expression(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_expression(std::string::iterator &curr, std::string::iterator &end, Position &p, unsigned int precedence);
    Token *lexparse_block(std::string::iterator &curr, std::string::iterator &end, Position &p);
    Token *lexparse_ifblock(std::string::iterator &curr, std::string::iterator &end, Position &p, Token **continuation);

    Token *lexparse_file(std::string &name, std::string &contents);
};

#endif /* LEXPARSE_HPP_ */
