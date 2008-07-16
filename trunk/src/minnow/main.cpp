#include <iostream>
#include <fstream>
#include <vector>

#include <stdlib.h>

#include <boost/program_options.hpp>

#include "parser.hpp"
#include "codegen_cppoutput.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    std::string outfile_name = "";
    std::string outexe_name = "";
    bool output_debug = false;
    std::vector<Token*> allToks;

    po::options_description general_opts("General options");
    general_opts.add_options()
        ("help,h", "help message")
        //("debugtree,t", "output debug parse tree")
        ("tocppfile,c", po::value<std::string>(), "output to C++ file")
        ("toexe,o", po::value<std::string>(), "(attempt to) compile to an exe")
        ;
    po::options_description hidden_opts("Hidden options");
    hidden_opts.add_options()
        ("input-file", po::value< std::vector<std::string> >(), "input file")
        ;
    po::positional_options_description p;
    p.add("input-file", -1);

    po::options_description visible_opts("");
    visible_opts.add(general_opts);

    po::options_description all_opts("All options");
    all_opts.add(general_opts).add(hidden_opts);


    po::variables_map vm;
    //po::store(po::parse_command_line(argc, argv, desc), vm);
    po::store(po::command_line_parser(argc, argv).options(all_opts).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: minnow <options> <input files>" << std::endl;
        std::cout << visible_opts << std::endl;
        return 0;
    }

    if (vm.count("debugtree")) {
        output_debug = true;
    }

    if (vm.count("tocppfile")) {
        //std::cout << "Outputing to: " << vm["tocppfile"].as<std::string>() << std::endl;
        outfile_name = vm["tocppfile"].as<std::string>();
    }

    if (vm.count("toexe")) {
        //std::cout << "Outputing to: " << vm["tocppfile"].as<std::string>() << std::endl;
        outexe_name = vm["toexe"].as<std::string>();
    }

    if (vm.count("input-file")) {
        //std::cout << "Input files are: " << std::endl;
        std::vector<std::string> files = vm["input-file"].as< std::vector<std::string> >();
        for (std::vector<std::string>::iterator iter = files.begin(), end = files.end(); iter != end; ++iter) {
            std::ifstream sourceFile(iter->c_str(), std::ios::binary);
            if (sourceFile.good()) {
                // get length of file:
                sourceFile.seekg (0, std::ios::end);
                int length = sourceFile.tellg();
                sourceFile.seekg (0, std::ios::beg);

                char *fBuffer = new char[length+1];
                sourceFile.read(fBuffer, length);
                fBuffer[length] = 0;
                std::string sourceCode(fBuffer);
                //std::string sourceFilename(argv[i]);

                //printf("Size: %i %i\n", length, strlen(fBuffer));
                //puts(fBuffer);

                std::vector<Token*> toks = tokenize(sourceCode, *iter);
                allToks.insert(allToks.end(), toks.begin(), toks.end());
            }
            else {
                std::cout << "Can not find file: " << *iter << std::endl;
                exit(1);
            }
        }
    }
    else {
        std::cout << "Usage: minnow <options> <input files>" << std::endl;
        std::cout << visible_opts << std::endl;
        return 0;
    }

    std::vector<Token*>::iterator beginToks = allToks.begin(), endToks = allToks.end();

    try {
        CodegenCPPOutput cppoutput;

        AppAST *ast = parseApp(beginToks, endToks);
        //std::cout << "Parsed successfully" << std::endl;

        std::string output = cppoutput.translate(ast);
        //std::cout << "File: " << std::endl;
        //std::cout << output;
        if (outexe_name != "") {
            std::ofstream outfile;
            outfile.open("tmpXXXXX.cpp");
            outfile << output;
            outfile.close();
            std::ostringstream exe_cmdline;

            //Linux (tested on Ubuntu w/ boost 1.35 packages)
            //exe_cmdline << "g++ -O3 -o " << outexe_name << " tmpXXXXX.cpp -Isrc/aquarium -L. -laquarium -lboost_thread";

            //OS X + MacPorts
            exe_cmdline << "g++ -O3 -o " << outexe_name << " tmpXXXXX.cpp -Isrc/aquarium -L. -laquarium -I/opt/local/include/boost-1_35 -L/opt/local/lib -lboost_thread-mt -lboost_program_options-mt";
            if (system(exe_cmdline.str().c_str()) == 0) {
                //remove("tmpXXXXX.cpp");
            }
            else {
                std::cout << "Used cmdline: " << exe_cmdline.str() << std::endl;
            }
        }
        else if (outfile_name != "") {
            std::ofstream outfile;
            outfile.open(outfile_name.c_str());
            outfile << output;
            outfile.close();
        }
        else {
            std::cout << output;
        }
    }
    catch (CompilerException &ce) {
        std::cout << "Error: " << ce.what() << std::endl;
    }

    /*
    AnalyserMachine am;
    //Once parsed and tokenized, we need to run some filters and sanity checks on the model to make sure the program is valid in a higher level before we try to
    //translate/compile it to its output format
    if (am.analyse(ps.tokens)) {

        //if we get to here, we're ready to translate to our output format (which for the current version is C++)
        ps.translate(outfile_name);
    }
    */
}
