#ifndef ANALYSER_HPP
#define ANALYSER_HPP

#include <map>
#include "parser.hpp"

class Analyser {
public:
    std::map<std::string, ActorAST*> actors;
    std::map<std::string, ClassAST*> classes;
    std::vector<VariableInfo*> varScopeStack;
    std::vector<PrototypeAST*> funScopeStack;

    VariableInfo* findVarInScope(const std::string &name) {
        for (std::vector<VariableInfo*>::reverse_iterator iter = varScopeStack.rbegin(),
                end = varScopeStack.rend(); iter != end; ++iter) {


            if ((*iter) == NULL) {
                throw CompilerException("Internal Error: variableinfo is null");
            }
            if ((*iter)->name == name) {
                return *iter;
            }
        }

        return NULL;
    }

    TypeInfo lookupReturnTypeInfo(const CallExprAST *ast) {
        for (std::vector<PrototypeAST*>::reverse_iterator iter = funScopeStack.rbegin(),
                end = funScopeStack.rend(); iter != end; ++iter) {

            //FIXME: This is insufficient for overloaded functions
            if ((*iter)->name == ast->name) {
                //TypeInfo ti((*iter)->type, TypeType::Scalar);
                return (*iter)->returnType;
            }
        }

        std::ostringstream msg;
        msg << "Can not find function '" << ast->name << "'";
        std::string outmsg = msg.str();
        throw CompilerException(outmsg, ast->filepos);
    }

    void indentDepth(unsigned int depth) {
        for (unsigned int i = 0; i < depth; ++i)
            std::cout << "  ";
    }
    ASTNode *scopeAndTypesPass(ASTNode* input);
    void debugOutputPass(ASTNode* input, unsigned int depth);
};

#endif /* ANALYSER_HPP */
