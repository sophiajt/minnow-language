// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

#include <string.h>
#include <stdlib.h>

#include "Compiler.hpp"

void print_help() {
    printf("Usage: minnowc <options> <filename(s)>\n");
    printf("-o <filename>   : compile and output to a binary file\n");
    printf("-O <level>      : set the optimization level\n");
    printf("-L <lib dir>    : add a library directory to the library search path\n");
    printf("-l <library>    : add a library to link against\n");
    printf("-C              : output the generated C file to the stdout\n");
    printf("-h              : this help\n");
}

int main(int argc, char *argv[]) {
    Compiler compiler;

    char *output_file;
    output_file = (char*)malloc(sizeof(char) * 7);
    strcpy(output_file, "noname");

    std::string current_bin = argv[0];

    if (argc < 2) {
        print_help();
        printf("Please specify the file to compile\n");
        exit(0);
    }

    std::vector<std::string> libs;
    libs.push_back("aquarium");

    std::string prelude_dir = "";
    std::string prefix_dir = "";
    std::string lib_dir = ".";
    std::string include_dir = "aquarium";
    std::string optimization_level = "0";

    #ifdef INSTALLPREFIX
        prefix_dir = INSTALLPREFIX;
        prelude_dir = prefix_dir + "share/minnow/";
        lib_dir = prefix_dir + "lib";
        include_dir = prefix_dir + "include/minnow";
    #endif

    std::vector<std::string> lib_dirs;
    lib_dirs.push_back(lib_dir);

    try {
        compiler.translate_file(prelude_dir + "prelude.mno");

        //for (int i = 1; i < argc; ++i) {
        int i = 1;
        while (i < argc) {
            /*
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
            else if (strcmp(argv[i], "-L") == 0) {
                //grab output file
                if ((i+1) < argc) {
                    std::string libdir = argv[i+1];
                    lib_dirs.push_back(libdir);
                    i = i + 2;
                }
                else {
                    printf("Missing library directory:  Use -L <director> to add a library directory\n");
                    exit(0);
                }
            }
            else if (strcmp(argv[i], "-l") == 0) {
                //grab output file
                if ((i+1) < argc) {
                    std::string libname = argv[i+1];
                    libs.push_back(libname);
                    i = i + 2;
                }
                else {
                    printf("Missing library filename:  Use -l <filename> to add a library\n");
                    exit(0);
                }
            }
            */
            if (argv[i][0] == '-') {
                int arglen = strlen(argv[i]);
                if (arglen > 1) {
                    switch(argv[i][1]) {
                        case ('h') :
                            print_help();
                            exit(0);
                        break;
                        case ('o') :
                            if (arglen == 2) {
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
                                //grab output file
                                output_file = (char *)(argv[i] + 2);
                                ++i;
                            }
                        break;
                        case ('C'):
                            output_file = NULL;
                            ++i;
                        break;
                        case ('L') :
                            if (arglen == 2) {
                                if ((i+1) < argc) {
                                    std::string libdir = argv[i+1];
                                    lib_dirs.push_back(libdir);
                                    i = i + 2;
                                }
                                else {
                                    printf("Missing library directory:  Use -L <director> to add a library directory\n");
                                    exit(0);
                                }
                            }
                            else {
                                //grab output file
                                std::string libdir = (char *)(argv[i] + 2);
                                lib_dirs.push_back(libdir);
                                ++i;
                            }
                        break;
                        case ('l') :
                            if (arglen == 2) {
                                if ((i+1) < argc) {
                                    std::string libname = argv[i+1];
                                    libs.push_back(libname);
                                    i = i + 2;
                                }
                                else {
                                    printf("Missing library filename:  Use -l <filename> to add a library\n");
                                    exit(0);
                                }
                            }
                            else {
                                //grab output file
                                std::string libname = (char *)(argv[i] + 2);
                                libs.push_back(libname);
                                ++i;
                            }
                        break;
                        case ('O') :
                            if (arglen == 2) {
                                if ((i+1) < argc) {
                                    optimization_level = argv[i+1];
                                    i = i + 2;
                                }
                                else {
                                    printf("Missing optimization level:  Use -O <level> to set the optimization level\n");
                                    exit(0);
                                }
                            }
                            else {
                                //grab output file
                                optimization_level = (char *)(argv[i] + 2);
                                ++i;
                            }
                        break;
                        default:
                            printf("Unknown option '%c'\n", argv[i][1]);
                            print_help();
                            exit(0);

                    }
                }

            }
            else {
                compiler.translate_file(argv[i]);
                ++i;
            }
        }
        //Once we're parsed and ready, analyze what we have
        compiler.analyze_files();

        //Then, start outputting code
        compiler.compile_program(argc, argv, output_file, include_dir, lib_dirs, libs, optimization_level);

    }
    catch (Compiler_Exception &ce) {
        //debug_print_def(p, t, "");
        //debug_print_vars(p, t);
        //debug_print(p, "");
        compiler.print_error(ce);
        exit(0);
    }
}
