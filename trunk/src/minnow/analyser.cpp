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

    if (input == NULL) {
        //NOP
        throw CompilerException("Internal error: AST node is NULL.");
    }
    //Number, Variable, ArrayIndexed, Binary, Quote, Call, DefFun, End, VarDecl, ArrayDecl, If, While
    switch (input->type()) {
        case (NodeType::Number) :
            neast = dynamic_cast<NumberExprAST*>(input);
            if (neast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            //FIXME: Check for dot for floats
            input->programmaticType.declType = "int";
            input->programmaticType.containerType = ContainerType::Scalar;
            break;
        case (NodeType::Boolean) :
            boeast = dynamic_cast<BooleanExprAST*>(input);
            if (boeast == NULL) {
                printf("FIXME: Boolean compiler exception\n");
            }
            input->programmaticType.declType = "bool";
            input->programmaticType.containerType = ContainerType::Scalar;
            break;

        case (NodeType::Variable) :
            veast = dynamic_cast<VariableExprAST*>(input);
            if (veast == NULL) {
                printf("FIXME: Variable compiler exception\n");
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
                printf("FIXME: Array indexed compiler exception\n");
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
                printf("FIXME: Binary compiler exception\n");
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
                            if ((*citer)->nodeType == NodeType::VarDecl) {
                                VarDeclExprAST *varDecl = dynamic_cast<VarDeclExprAST*>(*citer);
                                if (varDecl->vi->name == as_var->name) {
                                    isFound = true;
                                    input->programmaticType = varDecl->vi->type;
                                }
                            }
                        }
                        else if (as_call != NULL) {
                            if ((*citer)->nodeType == NodeType::Function) {
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
                    throw CompilerException("Unmatched types", input->filepos);
                }
                else {
                    input->programmaticType = input->children[0]->programmaticType;
                }
            }
            break;

        case (NodeType::Quote) :
            qeast = dynamic_cast<QuoteExprAST*>(input);
            if (qeast == NULL) {
                printf("FIXME: Quote compiler exception\n");
            }
            input->programmaticType.declType = "quote";
            input->programmaticType.containerType = ContainerType::Scalar;
            break;
        case (NodeType::Call) :
            ceast = dynamic_cast<CallExprAST*>(input);
            if (ceast == NULL) {
                printf("FIXME: Call compiler exception\n");
            }

            input->programmaticType = lookupReturnTypeInfo(ceast);
            //typeInfo = lookupReturnTypeInfo(ceast);
            //ti.get()->declType = typeInfo.declType;
            //ti.get()->containerType = typeInfo.containerType;
            //gc = visit(ceast);
            break;

        default:
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                analyseScopeAndTypes(*cnode);
            }
    }

    return input;
}
