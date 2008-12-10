// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <map>
#include <vector>
#include <sstream>

class Scope;

class Token_Type {
public:
    enum Type {
        EMPTY, INT, FLOAT, BOOL, FILE, ID, SYMBOL, BLOCK, FUN_CALL, ARRAY_CALL, QUOTED_STRING, SINGLE_QUOTED_STRING,
        FUN_DEF, ACTION_DEF, ACTOR_DEF, FEATURE_DEF, USE_CALL, IF_BLOCK, ELSEIF_BLOCK, ELSE_BLOCK, WHILE_BLOCK, //20
        ACTION_CALL, RETURN_CALL, VAR_DECL, NAMESPACE, NEW_ALLOC, SPAWN_ACTOR, REFERENCE_FEATURE, RESUME_SITE, //28
        METHOD_CALL, ATTRIBUTE_CALL, VAR_CALL, THIS, CONTINUATION_SITE, DELETION_SITE, EXTERN_FUN_DEF, ISOLATED_ACTOR_DEF, COPY, //37
        DELETE, CONSTRUCTOR_CALL, QUOTED_STRING_CONST, CONCATENATE, EOL
    };
};

class Position {
public:
    unsigned int line;
    unsigned int col;
    char *filename;

    Position() : line(1), col(1), filename(NULL) { }
    Position(unsigned int l, unsigned int c, char *fn) : line(l), col(c), filename(fn) { }

    bool operator<(const Position &other) const {
        if (this->line < other.line) {
            return true;
        }
        else if ((this->line == other.line) && (this->col < other.col)) {
            return true;
        }
        else {
            return false;
        }
    }
    bool operator>(const Position &other) const {
        if (this->line > other.line) {
            return true;
        }
        else if ((this->line == other.line) && (this->col > other.col)) {
            return true;
        }
        else {
            return false;
        }
    }
    bool operator<=(const Position &other) const {
        if (this->line <= other.line) {
            return true;
        }
        else if ((this->line == other.line) && (this->col <= other.col)) {
            return true;
        }
        else {
            return false;
        }
    }
    bool operator>=(const Position &other) const {
        if (this->line >= other.line) {
            return true;
        }
        else if ((this->line == other.line) && (this->col >= other.col)) {
            return true;
        }
        else {
            return false;
        }
    }
};

class Token {
public:
    Token_Type::Type type;
    std::string contents;
    Position start_pos, end_pos;
    std::vector<Token*> children;
    int definition_number;
    int type_def_num; //this is an index into the global types
    Scope *scope;

    Token(Token_Type::Type ttype, std::string content, Position &start, Position &end) :
        type(ttype), contents(content), start_pos(start), end_pos(end), definition_number(-1), type_def_num(-1), scope(NULL) { }

    Token(Token_Type::Type ttype, Position &start, Position &end) :
        type(ttype), contents(""), start_pos(start), end_pos(end), definition_number(-1), type_def_num(-1), scope(NULL) { }

    Token(Token_Type::Type ttype, std::string content) :
        type(ttype), contents(content), definition_number(-1), type_def_num(-1), scope(NULL) { }

    Token(Token_Type::Type ttype) : type(ttype), definition_number(-1), type_def_num(-1), scope(NULL) { }
};

class Compiler_Exception {
public:
    std::string reason;
    Position where;

    Compiler_Exception(std::string reason_, Position &where_) : reason(reason_), where(where_) { }
};

class Function_Def {
public:
    Token *token;
    unsigned int return_type_def_num;
    std::vector<unsigned int> arg_def_nums;
    bool is_internal;
    bool is_constructor;
    std::string external_name; //if set use this name instead in codegen
    std::vector<int> continuation_sites;

    Function_Def() : token(NULL), is_internal(false), is_constructor(false) {
        continuation_sites.push_back(-1);
    }
    Function_Def(bool internal) : token(NULL), is_internal(internal), is_constructor(false) {
        continuation_sites.push_back(-1);
    }
    Function_Def(bool internal, unsigned int return_type, unsigned int lhs, unsigned int rhs) : token(NULL), return_type_def_num(return_type),
        is_internal(internal), is_constructor(false) {

        continuation_sites.push_back(-1);

        arg_def_nums.push_back(lhs);
        arg_def_nums.push_back(rhs);
    }
};

class Container_Type {
    public:
        enum Type { SCALAR, LIST };
};

class Type_Def {
public:
    Token *token;
    bool is_internal;
    bool is_isolated_actor;  //todo: move this to being part of an attribute flag set
    int contained_type_def_num;
    Container_Type::Type container;

    Type_Def() : token(NULL), is_internal(false), is_isolated_actor(false),
        contained_type_def_num(-1), container(Container_Type::SCALAR) { }
    Type_Def(bool internal) : token(NULL), is_internal(internal), is_isolated_actor(false),
        contained_type_def_num(-1), container(Container_Type::SCALAR) { }
};

class Var_Def {
public:
    Token *token;
    unsigned int type_def_num;
    bool is_property;
    bool is_removed; //todo: use proper tracing instead of the work around
    Position usage_start, usage_end;

    Var_Def() : token(NULL), type_def_num(-1), is_property(false), is_removed(false) {}
};

class Scope {
public:
    Scope *parent;
    std::map<std::string, unsigned int> local_funs;
    std::map<std::string, unsigned int> local_types;
    std::map<std::string, unsigned int> local_vars;
    std::map<std::string, Scope*> namespaces;

    Token *owner;  //For reverse lookup back to the owner of the scope

    Scope() {
        parent = NULL;
        owner = NULL;
    }
};

#include "Program.hpp"

#endif /* COMMON_HPP_ */