// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <map>
#include <vector>
#include <sstream>
#include <string.h>

#define USE_MEMBLOCK_CACHE

class Token;

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

class Token_Type {
public:
    enum Type {
        EMPTY, INT, FLOAT, BOOL, FILE, ID, SYMBOL, BLOCK, FUN_CALL, ARRAY_CALL, QUOTED_STRING, SINGLE_QUOTED_STRING,
        FUN_DEF, ACTION_DEF, ACTOR_DEF, FEATURE_DEF, USE_CALL, IF_BLOCK, ELSEIF_BLOCK, ELSE_BLOCK, WHILE_BLOCK, //20
        ACTION_CALL, RETURN_CALL, VAR_DECL, NAMESPACE, NEW_ALLOC, SPAWN_ACTOR, REFERENCE_FEATURE, RESUME_SITE, //28
        METHOD_CALL, ATTRIBUTE_CALL, VAR_CALL, THIS, CONTINUATION_SITE, DELETION_SITE, EXTERN_FUN_DEF, ISOLATED_ACTOR_DEF, COPY, //37
        DELETE, CONSTRUCTOR_CALL, QUOTED_STRING_CONST, CONCATENATE, APPLICATION, ENUM_DEF, ENUM_CALL, REFERENCE_ENUM, FOR_BLOCK, BREAK,
        TRY_BLOCK, CATCH_BLOCK, EXCEPTION, ARRAY_INIT, CONCATENATE_ARRAY, LIBRARY_EXTERN_BLOCK, QUICK_VAR_DECL, EOL
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
    bool operator==(const Position &other) const {
        if ((this->line == other.line) && (this->col == other.col) && (this->filename == other.filename)) {
            return true;
        }
        else {
            return false;
        }
    }
    bool operator<=(const Position &other) const {
        if (this->line < other.line) {
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
        if (this->line > other.line) {
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

    Token(Token &t) {
        type = t.type;
        contents = t.contents;
        start_pos = t.start_pos;
        end_pos = t.end_pos;
        for (unsigned int i = 0; i < t.children.size(); ++i) {
            children.push_back(new Token(*(t.children[i])));
        }
        definition_number = t.definition_number;
        type_def_num = t.type_def_num;
        //todo: what do I do with this?!
        scope = t.scope;
        /*
        if (t.scope == NULL) {
            scope = t.scope;
        }
        else {
            scope = new class Scope(*(t.scope));
        }
        */

    }
    //todo: I should probably have something like this, but before I add it I want to make sure I don't share references
    /*
    ~Token() {
        for (unsigned int i = 0; i < children.size(); ++i) {
            delete(children[i]);
        }
    }
    */
};

class Compiler_Exception {
public:
    std::string reason;
    Position where_begin;
    Position where_end;

    Compiler_Exception(std::string reason_, Position &where_begin_, Position &where_end_) :
        reason(reason_), where_begin(where_begin_), where_end(where_end_) { }
};

class Function_Def {
public:
    Token *token;
    unsigned int return_type_def_num;
    std::vector<unsigned int> arg_def_nums;
    bool is_used;
    bool is_internal;
    bool is_constructor;
    bool is_port_of_entry; //if this function can yield return values that the scope is responsible for
    bool is_port_of_exit; //if the args given to this function draw them out of scope (forcing a copy if that's not possible)
    std::string external_name; //if set use this name instead in codegen
    std::vector<int> continuation_sites;

    Function_Def() : token(NULL), is_used(true), is_internal(false), is_constructor(false), is_port_of_entry(true), is_port_of_exit(false) {
        continuation_sites.push_back(-1);
    }
    Function_Def(bool internal) : token(NULL), is_used(true), is_internal(internal), is_constructor(false), is_port_of_entry(true), is_port_of_exit(false) {
        continuation_sites.push_back(-1);
    }
    Function_Def(bool internal, unsigned int return_type, unsigned int lhs, unsigned int rhs) : token(NULL), return_type_def_num(return_type),
        is_used(true), is_internal(internal), is_constructor(false), is_port_of_entry(true), is_port_of_exit(false) {

        continuation_sites.push_back(-1);

        arg_def_nums.push_back(lhs);
        arg_def_nums.push_back(rhs);
    }
};

class Container_Type {
    public:
        enum Type { SCALAR, ARRAY, DICT, FUNCTOR };
};

class Type_Def {
public:
    Token *token;
    bool is_internal;
    bool is_isolated_actor;  //todo: move this to being part of an attribute flag set
    std::vector<int> contained_type_def_nums;

    Container_Type::Type container;

    Type_Def() : token(NULL), is_internal(false), is_isolated_actor(false),
        container(Container_Type::SCALAR) { }
    Type_Def(bool internal) : token(NULL), is_internal(internal), is_isolated_actor(false),
        container(Container_Type::SCALAR) { }
};

class Extent_Type {
public:
    enum Type { DECLARE, IF_START, ELSEIF_START, ELSE_START, IF_JOIN, READ, WRITE, LOOP_START, LOOP_JOIN };
};

class Extent_Site_Type {
public:
    enum Type { UNKNOWN, READ, WRITE };
};

class Extent_Color_Type {
public:
    /**
     * The extent "colors" that make up variable tracing:
     * Store - This variable's value must be preserved between continuations
     * Default - This variable should be given a default value on resume, but ignored when swapping out
     * Delete - This variable should be deleted when swapping out, and given a default value on resume
     * Ignore - This variable shouldn't be dealt with
     */
    enum Type { STORE, DEFAULT, DELETE, IGNORE };
};

class Extent {
public:
    Extent_Type::Type type;
    Extent_Site_Type::Type site_type;
    Extent *prev;
    Extent *next;
    Position start_pos, end_pos;

    Extent() : type(Extent_Type::READ), site_type(Extent_Site_Type::UNKNOWN), prev(NULL), next(NULL) { }
};

class Extent_Color {
    Extent_Color_Type::Type type;
    Extent_Color *next;
    Position start_pos, end_pos;

    Extent_Color() : type(Extent_Color_Type::STORE), next(NULL) { }
};

class Var_Def {
public:
    Token *token;
    int type_def_num;
    bool is_property;
    bool is_removed; //todo: use proper tracing instead of the work around.  Todo: remove this.
    bool is_dependent; //if we're responsible for this variable directly.
    bool is_temporary;

    Position usage_start, usage_end;

    Extent *extent;
    Extent_Color *extent_color;

    Var_Def() : token(NULL), type_def_num(-1), is_property(false), is_removed(false),
        is_dependent(true), is_temporary(false), extent(NULL), extent_color(NULL) {}
};



#include "Program.hpp"

#endif /* COMMON_HPP_ */
