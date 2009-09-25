// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include "Common.hpp"
#include "Compiler.hpp"

void debug_print(Program *p, Scope *ns, std::string prepend) {
    if (ns->parent == NULL) {
        std::cout << prepend << "scope: " << ns << " Owner: " << ns->owner << std::endl;
    }
    else {
        std::cout << prepend << " " << ns << " Parent: " << ns->parent << " Owner: " << ns->owner << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_funs.begin(), end = ns->local_funs.end(); iter != end; ++iter) {
        std::cout << prepend << " fun: " << iter->first << " " << iter->second << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_types.begin(), end = ns->local_types.end(); iter != end; ++iter) {
        std::cout << prepend << " type: " << iter->first << " " << iter->second << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_vars.begin(), end = ns->local_vars.end(); iter != end; ++iter) {
        std::cout << prepend << " var: " << iter->first << " def: " << iter->second << " type: " << p->vars[iter->second]->type_def_num << " Usage: "
            << p->vars[iter->second]->usage_start.line
            << " " << p->vars[iter->second]->usage_start.col
            << ", " << p->vars[iter->second]->usage_end.line
            << " " << p->vars[iter->second]->usage_end.col  << std::endl;
    }
    for (std::map<std::string, Scope*>::iterator iter = ns->namespaces.begin(), end = ns->namespaces.end(); iter != end; ++iter) {
        std::cout << prepend << " child: " << iter->first;
        debug_print(p, iter->second, prepend + "  ");
    }
}

void debug_print(Program *p, std::string prepend) {
    //For now let's just print the namespaces
    for (unsigned int i = 0; i < p->var_sites.size(); ++i) {
        std::cout << "--" << i << " " << p->var_sites[i].size() << std::endl;
        for (unsigned int j = 0; j < p->var_sites[i].size(); ++j) {
            std::cout << prepend << p->var_sites[i][j] << std::endl;
        }
    }
    //for (std::vector<Var_Def*>::iterator iter = p->vars.begin(), end = p->vars.end(); iter != end; ++iter) {
    for (unsigned int i = 0; i < p->vars.size(); ++i) {
        std::cout << prepend << " var: " << i << " def: " << p->vars[i] << " type: " << p->vars[i]->type_def_num << " Usage: "
            << p->vars[i]->usage_start.line
            << " " << p->vars[i]->usage_start.col
            << ", " << p->vars[i]->usage_end.line
            << " " << p->vars[i]->usage_end.col  << std::endl;
    }

    debug_print(p, p->global, prepend);
}

void debug_print_vars(Program *p, Token *token) {
    if (token->scope != NULL) {
        Scope *scope = token->scope;
        for (std::map<std::string, unsigned int>::iterator iter = scope->local_vars.begin(), end = scope->local_vars.end(); iter != end; ++iter) {
            std::cout << iter->first << " " << iter->second << "(" << p->vars[iter->second]->usage_start.line << ", " << p->vars[iter->second]->usage_start.col << ") "
                << "(" << p->vars[iter->second]->usage_end.line << ", " << p->vars[iter->second]->usage_end.col << ") " << std::endl;
        }
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        debug_print_vars(p, token->children[i]);
    }
}

void debug_print_def(Program *p, Token *token, std::string prepend) {
    if (token->contents == "") {
        std::cout << prepend << "(" << token->type << " def:" << token->definition_number << " type:" << token->type_def_num << " "
            << token->scope << " " << token->start_pos.line << " " << token->start_pos.col << " to " << token->end_pos.line << " " << token->end_pos.col << ")" << std::endl;
    }
    else {
        std::cout << prepend << token->contents << " (" << token->type << " def:" << token->definition_number
            << " type:" << token->type_def_num << " " << token->scope << " " << token->start_pos.line << " " << token->start_pos.col << " to "
            << token->end_pos.line << " " << token->end_pos.col << ")" << std::endl;
    }

    if (token->scope != NULL) {
        debug_print(p, token->scope, prepend + "  ");
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        debug_print_def(p, token->children[i], "   " + prepend);
    }
}

void debug_print_position(Position &pos) {
    std::cout << "f: " << pos.filename << " l: " << pos.line << " c: " << pos.col;
}

void debug_print_extents(Program *p) {
    for (unsigned int i = 0; i < p->vars.size(); ++i) {
        std::cout << "var: " << i;
        Extent *extent = p->vars[i]->extent;
        while (extent != NULL) {
            std::cout << "[";
            switch (extent->type) {
                case (Extent_Type::DECLARE) : std::cout << "Decl"; break;
                case (Extent_Type::ELSEIF_START) : std::cout << "Elseif"; break;
                case (Extent_Type::ELSE_START) : std::cout << "Else"; break;
                case (Extent_Type::IF_JOIN) : std::cout << "EndIf"; break;
                case (Extent_Type::IF_START) : std::cout << "If"; break;
                case (Extent_Type::LOOP_JOIN) : std::cout << "EndLoop"; break;
                case (Extent_Type::LOOP_START) : std::cout << "Loop"; break;
                case (Extent_Type::READ) : std::cout << "Read"; break;
                case (Extent_Type::WRITE) : std::cout << "Write"; break;
            }
            std::cout << " ";
            debug_print_position(extent->start_pos);
            std::cout << " ";
            debug_print_position(extent->end_pos);
            std::cout << "]" << std::endl;
            extent = extent->next;
        }
        std::cout << std::endl;
    }
}
