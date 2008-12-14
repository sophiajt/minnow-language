// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

#include <string.h>
#include <stdlib.h>
#include "Common.hpp"
#include "LexParser.hpp"
#include "Analyzer.hpp"
#include "Codegen_C.hpp"

void debug_print(Program *p, Scope *ns, std::string prepend) {
    if (ns->parent == NULL) {
        std::cout << prepend << "scope: " << ns << " " << ns->parent << std::endl;
    }
    else {
        std::cout << prepend << " " << ns << " " << ns->parent << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_funs.begin(), end = ns->local_funs.end(); iter != end; ++iter) {
        std::cout << prepend << " fun: " << iter->first << " " << iter->second << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_types.begin(), end = ns->local_types.end(); iter != end; ++iter) {
        std::cout << prepend << " type: " << iter->first << " " << iter->second << std::endl;
    }
    for (std::map<std::string, unsigned int>::iterator iter = ns->local_vars.begin(), end = ns->local_vars.end(); iter != end; ++iter) {
        std::cout << prepend << " var: " << iter->first << " def: " << iter->second << " type: " << p->vars[iter->second]->type_def_num << std::endl;
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
        std::cout << prepend << "(" << token->type << " def:" << token->definition_number << " type:" << token->type_def_num << " " << token->scope << ")" << std::endl;
    }
    else {
        std::cout << prepend << token->contents << " (" << token->type << " def:" << token->definition_number
            << " type:" << token->type_def_num << " " << token->scope << ")" << std::endl;
    }

    if (token->scope != NULL) {
        debug_print(p, token->scope, prepend + "  ");
    }

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        debug_print_def(p, token->children[i], "   " + prepend);
    }
}

std::string load_file(const char *filename) {
    std::ifstream infile (filename, std::ios::in | std::ios::ate);
    if (!infile.is_open()) {
        std::cerr << "Can not open " << filename << std::endl;
        exit(0);
    }

    std::streampos size = infile.tellg();
    infile.seekg(0, std::ios::beg);

    std::vector<char> v(size);
    infile.read(&v[0], size);

    std::string ret_val (v.empty() ? std::string() : std::string (v.begin(), v.end()).c_str());

    return ret_val;
}

void translate_file(Program *p, std::string filename) {
    Token *t;

    Analyzer an;
    Lex_Parser lp;

    std::string contents = load_file(filename.c_str());
    t = lp.lexparse_file(filename, contents);
    an.analyze_strays(t);

    //Start building app
    Scope *start = p->global;
    an.analyze_type_blocks(p, t, &start);
    start = p->global;
    an.analyze_fun_blocks(p, t, &start);
    an.add_implied_constructors(p);
    an.analyze_var_type_and_scope(p, t, p->global);
    an.analyze_token_types(p, t, p->global);

    an.analyze_ports_of_entry(p, t, p->global);
    an.analyze_implied_this(p, t, p->global);
    an.analyze_return_calls(p, t, 0);
    an.analyze_var_visibility(p, t);
    an.analyze_freeze_resume(p, t, p->global);
    an.analyze_copy_delete(p, t, p->global);
    /*
    an.analyze_embedded_functions(p, t);
    an.analyze_implied_this(p, t, p->global);
    an.analyze_return_calls(p, t, 0);
    an.analyze_var_visibility(p, t);
    an.analyze_freeze_resume(p, t, p->global);
    an.analyze_copy_delete(p, t, p->global);
    */

    //debug_print_def(p, t, "");
    //debug_print_vars(p, t);
    //debug_print(p, "");

    p->files.push_back(t);

    return;
}

int main(int argc, char *argv[]) {
    Program *p = new Program();
    Codegen c;
    char *output_file = NULL;

    if (argc < 2) {
        printf("Please specify the file to compile\n");
        exit(0);
    }

    try {
        translate_file(p, "prelude.mno");

        //for (int i = 1; i < argc; ++i) {
        int i = 1;
        while (i < argc) {
            if (strcmp(argv[i], "-o") == 0) {
                //grab output file
                if ((i+1) < argc) {
                    output_file = argv[i+1];
                    i = i + 2;
                }
                else {
                    printf("Missing output filename:  Use -o <filename> to output a binary\n");
                    exit(0);
                }
            }
            else {
                translate_file(p, argv[i]);
                ++i;
            }
        }
        //Start outputting code

        std::ostringstream output;
        c.codegen(p, p->files[0], output);
        if (output_file == NULL) {
            std::cout << output.str();
        }
        else {
            std::ofstream outfile;
            outfile.open("tmpXXXXX.c");
            outfile << output.str();
            outfile.close();
            std::ostringstream exe_cmdline;

            exe_cmdline << "gcc -ggdb -O3 -o " << output_file << " tmpXXXXX.c -Werror -Iaquarium -L. -laquarium";

            if (system(exe_cmdline.str().c_str()) == 0) {
                remove("tmpXXXXX.cpp");
            }
            else {
                std::cout << "For commandline: ";
                for (int i = 0; i < argc; ++i) {
                    std::cout << argv[i] << " ";
                }
                std::cout << std::endl;
                std::cout << "Used compile line: " << exe_cmdline.str() << std::endl;
                exit(-1);
            }

        }

    }
    catch (Compiler_Exception &ce) {
        //debug_print_def(p, t, "");
        //debug_print_vars(p, t);
        //debug_print(p, "");
        if (ce.where.line > 0) {
            std::cerr << "Error: " << ce.reason << " at line " << ce.where.line << " col " << ce.where.col << " in " << ce.where.filename << std::endl;
        }
        else {
            std::cerr << "Error: " << ce.reason << std::endl;
        }
        exit(0);
    }
}
