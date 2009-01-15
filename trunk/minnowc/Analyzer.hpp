// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef ANALYZER_HPP_
#define ANALYZER_HPP_

#include "Common.hpp"

class Analyzer {
    int next_continuation;
    int next_deletion;
public:
    Scope *find_or_create_namespace(Program *program, Token *ns);
    Scope *find_namespace(Program *program, Token *ns, bool throw_exceptions);
    unsigned int find_type(Program *program, Token *ns, Scope *scope);
    void find_constructor(Program *program, Token *ns, Scope *scope);

    unsigned int build_var_def(Program *program, Token *token, Scope *scope);
    std::string build_function_arg_types(Program *program, Token *token, Scope *scope);
    std::string build_function_name(Program *program, Token *token, Scope *scope);

    std::string build_function_def(Program *program, Token *token, Scope *scope, Function_Def *fd);
    void build_function_args(Program *program, Token *token, Scope *scope, Function_Def *fd);

    void analyze_strays(Token *token);
    void analyze_type_blocks(Program *program, Token *token, Scope **scope);
    void analyze_fun_blocks(Program *program, Token *token, Scope **scope);

    void add_implied_constructors(Program *program);

    void analyze_var_type_and_scope(Program *program, Token *token, Scope *scope);

    void check_fun_call(Program *program, Token *token, Scope *fun_scope, Scope *parm_scope);
    void analyze_token_types(Program *program, Token *token, Scope *scope);

    void analyze_implied_this(Program *program, Token *token, Scope *scope);

    Token *extract_function(Program *program, Token *token, Scope *var_scope);
    void analyze_embedded_functions(Program *program, Token *token);

    void analyze_return_calls(Program *program, Token *token, unsigned int allowed_return_type);

    bool contains_var(Token *token, unsigned int var_def_num);
    void find_var_endpoints(Program *program, Token *token, unsigned int var_def_num);
    void analyze_var_visibility(Program *program, Token *token);
    void analyze_usage_extents(Program *program, Token *token, Token *bounds, Scope *scope);
    void analyze_usage_extent_colors(Program *program);

    std::vector<int> build_push_pop_list(Program *program, Scope *scope, Position &tok_start, Position &tok_end);
    void analyze_freeze_resume(Program *program, Token *token, Scope *scope);

    Token *create_temp_replacement(Program *program, Token *token, Token *bounds, Scope *var_scope,
            unsigned int type_def_num, bool is_dependent);
    Token *analyze_ports_of_entry(Program *program, Token *token, Token *bounds, Scope *scope, bool is_lhs, bool is_block_bound);

    std::vector<int> build_delete_list(Program *program, Scope *scope, Position &position);
    std::vector<int> build_delete_remaining_list(Program *program, Scope *scope);

    void examine_port_of_exit(Program *program, Token *token);
    void examine_equation_for_copy_delete(Program *program, Token *block, Token *token, unsigned int &i);
    void analyze_copy_delete(Program *program, Token *token, Scope *scope);

    Analyzer() : next_continuation(0), next_deletion(0) { }
};

bool is_complex_type(Program *program, unsigned int type_def_num);
bool is_complex_var(Program *program, unsigned int definition_number) ;

#endif /* ANALYZER_HPP_ */
