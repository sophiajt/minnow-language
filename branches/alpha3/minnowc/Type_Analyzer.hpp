// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef TYPE_ANALYZER_HPP_
#define TYPE_ANALYZER_HPP_

class Type_Analyzer {
    int for_inner_var_id;

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

    void analyze_stacked_fun_call(Program *program, Token *token, Scope *scope);
    void analyze_var_type_and_scope(Program *program, Token *token, Scope *scope);

    void check_fun_call(Program *program, Token *token, Scope *fun_scope, Scope *parm_scope);
    void analyze_token_types(Program *program, Token *token, Scope *scope);

    void analyze_implied_this(Program *program, Token *token, Scope *scope);

    void analyze_return_calls(Program *program, Token *token, unsigned int allowed_return_type);
    bool analyze_required_return_calls(Program *program, Token *token);

    Type_Analyzer() : for_inner_var_id(0) { }
};

bool is_complex_type(Program *program, unsigned int type_def_num);
bool is_complex_var(Program *program, unsigned int definition_number) ;


#endif /* TYPE_ANALYZER_HPP_ */
