// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef COMPILER_HPP_
#define COMPILER_HPP_

#include "Common.hpp"
#include "LexParser.hpp"
#include "Analyzer.hpp"
#include "Codegen_C.hpp"

void debug_print(Program *p, Scope *ns, std::string prepend);
void debug_print(Program *p, std::string prepend);
void debug_print_vars(Program *p, Token *token);
void debug_print_def(Program *p, Token *token, std::string prepend);
void debug_print_position(Position &pos);
void debug_print_extents(Program *p);

class Compiler {
    Analyzer an;
    Lex_Parser *lp;
    Codegen c;
    Program *p;

    Token *app;

public:
    std::vector<std::string> search_path;

    Compiler() {
        app = new Token(Token_Type::APPLICATION);
        p = new Program();
        lp = new Lex_Parser(p);
    }

    std::string load_file(const char *filename) {
        std::ifstream infile (filename, std::ios::in | std::ios::ate);
        unsigned int i = 0;
        while ((!infile.is_open()) && (i < search_path.size())) {
            infile.open((search_path[i] + "/" + std::string(filename)).c_str(), std::ios::in | std::ios::ate);
            ++i;
        }

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

    void translate_file(std::string filename) {
        Token *t;

        std::vector<std::string> use_list;

        std::string contents = load_file(filename.c_str());
        t = lp->lexparse_file(filename, contents, &use_list);
        app->children.push_back(t);
        p->files.push_back(t);

        for (unsigned int i = 0; i < use_list.size(); ++i) {
            contents = load_file(use_list[i].c_str());
            t = lp->lexparse_file(filename, contents, &use_list);
            app->children.push_back(t);
            p->files.push_back(t);
        }

        return;
    }

    void analyze_files() {
        Scope *start;

        for (unsigned int i = 0; i < p->files.size(); ++i) {
            Token *t = p->files[i];

            an.analyze_strays(t);

            //Start building app
            start = p->global;
            an.analyze_type_blocks(p, t, &start);
        }


        for (unsigned int i = 0; i < p->files.size(); ++i) {
            Token *t = p->files[i];

            start = p->global;
            an.analyze_fun_blocks(p, t, &start);
        }


        Token *t = app;

        //start = p->global;

        an.add_implied_constructors(p);
        an.analyze_var_type_and_scope(p, t, p->global);

        an.analyze_token_types(p, t, p->global);

        an.analyze_ports_of_entry(p, t, NULL, p->global, false, false);
        an.analyze_implied_this(p, t, p->global);
        an.analyze_return_calls(p, t, 0);
        an.analyze_var_visibility(p, t);
        an.analyze_freeze_resume(p, t, p->global);
        an.analyze_copy_delete(p, t, NULL, p->global);

        an.analyze_usage_extents(p, t, NULL, p->global);
        an.analyze_usage_extent_colors(p);

        //debug_print_extents(p);

        //debug_print_def(p, t, "");
        //debug_print_vars(p, t);
        //debug_print(p, "");

    }
    void compile_program(int argc, char *argv[], char *output_file, std::string include_dir,
            const std::vector<std::string> &lib_dirs, const std::vector<std::string> &libs, const std::string optimization_level) {

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

#if defined (__SVR4) && defined (__sun)
            exe_cmdline << "g++ -ggdb -O" << optimization_level << " -o \"" << output_file << "\" tmpXXXXX.c -Werror -I\"" << include_dir << "\" ";
#else
            exe_cmdline << "gcc -ggdb -O" << optimization_level << " -o \"" << output_file << "\" tmpXXXXX.c -Werror -I\"" << include_dir << "\" ";
#endif

            for (unsigned int i = 0; i < lib_dirs.size(); ++i) {
                exe_cmdline << "-L\"" << lib_dirs[i] << "\" ";
            }

            for (unsigned int i = 0; i < libs.size(); ++i) {
                exe_cmdline << "-l" << libs[i] << " ";
            }

            if (system(exe_cmdline.str().c_str()) == 0) {
                remove("tmpXXXXX.c");
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

    void print_positions(std::string &contents, Position &start_pos, Position &end_pos) {
        unsigned int line, col;
        line = 1; col = 1;

        std::string::iterator p = contents.begin();

        while (line < start_pos.line) {
            if (*p == '\n') {
                ++line;
            }
            ++p;
        }

        std::string::iterator reset = p;

        //while we're in the right area, print the line, then print the highlight
        while (line <= end_pos.line) {
            col = 1;

            //First pass, print the line:
            reset = p;
            while ((*p != '\n') && (p != contents.end())) {
                std::cout << *p;
                ++p;
            }
            std::cout << std::endl;

            //Second pass, print the highlight:
            p = reset;
            while ((*p != '\n') && (p != contents.end())) {
                if ((start_pos.line == end_pos.line) && (col >= start_pos.col) && (col < end_pos.col)) {
                    std::cout << '^';
                }
                else if ((start_pos.line != end_pos.line) && (((line == start_pos.line) && (col >= start_pos.col))
                        || ((line > start_pos.line) && (line < end_pos.line))
                        || ((line == end_pos.line) && (col < end_pos.col)))) {
                    std::cout << '^';
                }
                else if ((start_pos == end_pos) && (line == start_pos.line) && (col == start_pos.col)) {
                    std::cout << '^';
                }
                else {
                    std::cout << ' ';
                }
                if (*p != '\n') {
                    ++col;
                }
                ++p;
            }
            ++line;
            ++p;
            std::cout << std::endl;
        }

    }

    void print_error(Compiler_Exception &ce) {
        //Reload file to see the error
        if ((ce.where_begin.filename != NULL) && (ce.where_begin.line > 0)) {
            std::string contents = load_file(ce.where_begin.filename);

            print_positions(contents, ce.where_begin, ce.where_end);

            std::cerr << "Error: " << ce.reason << " at line " << ce.where_begin.line << " col "
                << ce.where_begin.col << " in " << ce.where_begin.filename << std::endl;
        }
        else {
            std::cerr << "Error: " << ce.reason << std::endl;
        }
    }
};

#endif /* COMPILER_HPP_ */
