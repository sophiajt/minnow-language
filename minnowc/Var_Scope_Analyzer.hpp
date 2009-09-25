// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef ANALYZER_HPP_
#define ANALYZER_HPP_

#include "Common.hpp"
#include "Type_Analyzer.hpp"

class Var_Scope_Analyzer {
    int next_continuation;
    int next_deletion;
public:

    bool contains_var(Token *token, unsigned int var_def_num);
    void find_var_endpoints(Program *program, Token *token, Token *bounds, unsigned int var_def_num);
    void analyze_var_visibility(Program *program, Token *token);

    //Currently unused
    void analyze_usage_extents(Program *program, Token *token, Token *bounds, Scope *scope);
    void analyze_usage_extent_for_var(Program *program, Extent *extent);
    void analyze_usage_extent_colors(Program *program);

    std::vector<int> build_push_pop_list(Program *program, Scope *scope, Position &tok_start, Position &tok_end);
    void analyze_freeze_resume(Program *program, Token *token, Scope *scope);

    Token *create_temp_replacement(Program *program, Token *token, Token *bounds, Scope *var_scope,
            unsigned int type_def_num, bool is_dependent);
    Token *analyze_ports_of_entry(Program *program, Token *token, Token *bounds, Scope *scope, bool is_lhs, bool is_block_bound);

    std::vector<int> build_delete_list(Program *program, Scope *scope, Position &position);
    std::vector<int> build_delete_remaining_list(Program *program, Scope *scope);

    void examine_port_of_exit(Program *program, Token *token, Token *bounds);
    void examine_equation_for_copy_delete(Program *program, Token *block, Token *token, Token *bounds, Scope *scope, unsigned int &i);
    void unwind_deletion_site(Program *program, Token *block, Token *token, Scope *scope, Position &del_lim, unsigned int &pos);
    void analyze_copy_delete(Program *program, Token *token, Token *bounds, Scope *scope);

    Var_Scope_Analyzer() : next_continuation(0), next_deletion(0){ }
};

#endif /* ANALYZER_HPP_ */
