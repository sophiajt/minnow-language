#include <iostream>

#include "analyser.hpp"
#include "parser.hpp"

ASTNode *Analyser::analyseScopeAndTypes(ASTNode* input) {
    VariableInfo *vi;

    NumberExprAST *neast;
    BooleanExprAST *boeast;
    QuoteExprAST *qeast;
    VariableExprAST *veast;
    ArrayIndexedExprAST *aieast;
    BinaryExprAST *beast;
    CallExprAST *ceast;
    PrototypeAST *proto;
    VarDeclExprAST *vdeast;

    std::ostringstream msg;

    unsigned int varScopeDepth, funScopeDepth;

    if (input == NULL) {
        //NOP
        return NULL;
    }
    else {
        std::cout << "Int: " << input->type() << std::endl;
    }
    //Number, Variable, ArrayIndexed, Binary, Quote, Call, DefFun, End, VarDecl, ArrayDecl, If, While
    switch (input->type()) {
        case (NodeType::Number) :
            neast = dynamic_cast<NumberExprAST*>(input);
            if (neast == NULL) {
                throw CompilerException("FIXME: Number compiler exception\n");
            }
            //FIXME: Check for dot for floats
            input->programmaticType.declType = "int";
            input->programmaticType.containerType = ContainerType::Scalar;
            break;
        case (NodeType::Boolean) :
            boeast = dynamic_cast<BooleanExprAST*>(input);
            if (boeast == NULL) {
                throw CompilerException("FIXME: Boolean compiler exception\n");
            }
            input->programmaticType.declType = "bool";
            input->programmaticType.containerType = ContainerType::Scalar;
            break;

        case (NodeType::Variable) :
            veast = dynamic_cast<VariableExprAST*>(input);
            if (veast == NULL) {
                throw CompilerException("FIXME: Variable compiler exception\n");
            }
            vi = findVarInScope(veast->name);
            if (vi != NULL) {
                input->programmaticType = vi->type;
            }
            else {
                std::ostringstream msg;
                msg << "Can not find variable '" << veast->name << "'";
                throw CompilerException(msg.str(), input->filepos);
            }
            break;
        case (NodeType::ArrayIndexed) :
            aieast = dynamic_cast<ArrayIndexedExprAST*>(input);
            if (input->children.size() == 0) {
                throw CompilerException("Incomplete array index", input->filepos);
            }
            analyseScopeAndTypes(input->children[0]);
            if (input->children[0]->programmaticType.declType != "int") {
                throw CompilerException("Non-integer array index", input->children[0]->filepos);
            }
            if (aieast == NULL) {
                throw CompilerException("FIXME: Array indexed compiler exception\n");
            }
            vi = findVarInScope(aieast->name);
            if (vi != NULL) {
                if (vi->type.containerType != ContainerType::Array) {
                    throw CompilerException("Indexing of non-array variable", input->filepos);
                }
                input->programmaticType = vi->type.containedTypes[0];
            }
            else {
                std::ostringstream msg;
                msg << "Can not find variable '" << aieast->name << "'";
                throw CompilerException(msg.str(), input->filepos);
            }
            break;
        case (NodeType::Binary) :
            beast = dynamic_cast<BinaryExprAST*>(input);
            if (beast == NULL) {
                throw CompilerException("FIXME: Binary compiler exception\n");
            }
            if (input->children.size() != 2) {
                throw CompilerException("Internal Error: incorrect number of binary children",
                        input->filepos);
            }
            analyseScopeAndTypes(input->children[0]);
            analyseScopeAndTypes(input->children[1]);

            if (beast->op == "::") {
                input->programmaticType.declType = "void";
                input->programmaticType.containerType = ContainerType::Scalar;
            }
            else if (beast->op == "."){
                if (input->children[0]->programmaticType.containerType != ContainerType::Scalar) {
                    throw CompilerException("Attempting to reference a non-scalar type", input->filepos);
                }
                std::map<std::string, ClassAST*>::iterator location =
                    this->classes.find(input->children[0]->programmaticType.declType);
                if (location == this->classes.end()) {
                    throw CompilerException("Could not find referenced class", input->filepos);
                }
                else {
                    VariableExprAST *as_var = dynamic_cast<VariableExprAST*>(input->children[1]);
                    CallExprAST *as_call = dynamic_cast<CallExprAST*>(input->children[1]);
                    if ((as_var == NULL) && (as_call == NULL)) {
                        std::ostringstream msg;
                        msg << "Can't use '.' in this context";
                        throw CompilerException(msg.str(), input->filepos);
                    }
                    bool isFound = false;
                    for (std::vector<ASTNode*>::iterator citer = location->second->children.begin(),
                            cend = location->second->children.end(); citer != cend; ++citer) {

                        if (as_var != NULL) {
                            if ((*citer)->type() == NodeType::VarDecl) {
                                VarDeclExprAST *varDecl = dynamic_cast<VarDeclExprAST*>(*citer);
                                if (varDecl->vi->name == as_var->name) {
                                    isFound = true;
                                    input->programmaticType = varDecl->vi->type;
                                }
                            }
                        }
                        else if (as_call != NULL) {
                            if ((*citer)->type() == NodeType::Function) {
                                if ((*citer)->children.size() == 0) {
                                    std::ostringstream msg;
                                    msg << "Internal error: function missing prototype";
                                    throw CompilerException(msg.str(), (*citer)->filepos);
                                }
                                PrototypeAST *proto = dynamic_cast<PrototypeAST*>((*citer)->children[0]);

                                //FIXME: this does _not_ allow overloading
                                if (proto->name == as_call->name) {
                                    isFound = true;
                                    input->programmaticType = proto->returnType;
                                }
                            }

                        }
                    }
                }
            }
            else {
                if (input->children[0]->programmaticType !=
                    input->children[1]->programmaticType) {
                    msg << "Unmatched types: " << input->children[0]->programmaticType.declType
                        << " vs " << input->children[1]->programmaticType.declType;
                    throw CompilerException(msg.str(), input->filepos);
                }
                else {
                    input->programmaticType = input->children[0]->programmaticType;
                }
            }
            break;

        case (NodeType::Quote) :
            qeast = dynamic_cast<QuoteExprAST*>(input);
            if (qeast == NULL) {
                throw CompilerException("FIXME: Quote compiler exception\n");
            }
            input->programmaticType.declType = "quote";
            input->programmaticType.containerType = ContainerType::Scalar;
            break;
        case (NodeType::Call) :
            ceast = dynamic_cast<CallExprAST*>(input);
            if (ceast == NULL) {
                throw CompilerException("FIXME: Call compiler exception\n");
            }
            //FIXME: Need to make sure "return" is in function scope
            if (ceast->name != "return") {
                input->programmaticType = lookupReturnTypeInfo(ceast);
            }
            break;
        case (NodeType::Prototype):
            proto = dynamic_cast<PrototypeAST*>(input);
            if (proto == NULL) {
                throw CompilerException("FIXME: prototype compiler exception");
            }
            funScopeStack.push_back(proto);
            std::cout << "Pushed: " << proto->name << std::endl;
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                analyseScopeAndTypes(*cnode);
            }
            break;
        case (NodeType::VarDecl):
            vdeast = dynamic_cast<VarDeclExprAST*>(input);
            if (vdeast == NULL) {
                throw CompilerException("FIXME: vardecl compiler exception");
            }
            varScopeStack.push_back(vdeast->vi);
            input->programmaticType = vdeast->vi->type;
            break;
        case (NodeType::Function):
            //prototype is captured earlier
            varScopeDepth = varScopeStack.size();
            for (unsigned int i = 1; i < input->children.size(); ++i) {
                analyseScopeAndTypes(input->children[i]);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::Action):
            varScopeDepth = varScopeStack.size();
            for (unsigned int i = 0; i < input->children.size(); ++i) {
                analyseScopeAndTypes(input->children[i]);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
        case (NodeType::While):
            varScopeDepth = varScopeStack.size();
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                analyseScopeAndTypes(*cnode);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::If):
            varScopeDepth = varScopeStack.size();
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                analyseScopeAndTypes(*cnode);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::Block):
            varScopeDepth = varScopeStack.size();
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                analyseScopeAndTypes(*cnode);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::Actor):
            std::cout << "Actor" << std::endl;
            break;
        case (NodeType::Class):
            std::cout << "Class" << std::endl;
            break;
        case (NodeType::App):
            std::cout << "App" << std::endl;
            funScopeDepth = funScopeStack.size();
            varScopeDepth = varScopeStack.size();
            //find prototypes and variables that are in the closed scope, and
            //do those first.
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                if (*cnode != NULL) {
                    std::cout << "[" << (*cnode)->type() << "]" << std::endl;
                    switch ((*cnode)->type()) {
                        case (NodeType::Function) :
                            std::cout << "Function uncovered" << std::endl;
                            analyseScopeAndTypes((*cnode)->children[0]);
                            break;
                        case (NodeType::VarDecl) :
                            std::cout << "Var uncovered" << std::endl;
                            analyseScopeAndTypes(*cnode);
                        default :
                            //do nothing
                            break;
                    }
                }
            }
            std::cout << "-->" << std::endl;
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                switch ((*cnode)->type()) {
                    case (NodeType::VarDecl) :
                        break;
                    default :
                        analyseScopeAndTypes(*cnode);
                        break;
                }

                //analyseScopeAndTypes(*cnode);
            }
            std::cout << "<--" << std::endl;
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            while (funScopeDepth != funScopeStack.size()) {
                funScopeStack.pop_back();
            }
            break;
        default:
            msg << "Unhandled element during analysis: " << input->type();
            throw CompilerException(msg.str(), input->filepos);
    }

    return input;
}
