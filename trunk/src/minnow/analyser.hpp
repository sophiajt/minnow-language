#ifndef ANALYSER_HPP
#define ANALYSER_HPP

#include <map>
#include "parser.hpp"

class Analyser {
public:
    std::map<std::string, ActorAST*> actors;
    std::map<std::string, ClassAST*> classes;
    std::vector<VariableInfo*> scopeStack;
    std::vector<PrototypeAST*> funStack;

    VariableInfo* findVarInScope(const std::string &name) {
        for (std::vector<VariableInfo*>::reverse_iterator iter = scopeStack.rbegin(), end = scopeStack.rend(); iter != end; ++iter) {
            if ((*iter)->name == name) {
                return *iter;
            }
        }

        return NULL;
    }

    ASTNode *analyseScopeAndTypes(ASTNode* input);
};

#endif /* ANALYSER_HPP */
