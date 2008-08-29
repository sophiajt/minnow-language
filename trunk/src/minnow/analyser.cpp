#include <iostream>

#include "analyser.hpp"
#include "parser.hpp"

ASTNode *Analyser::scopeAndTypesPass(ASTNode* input) {
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
    ActorAST *actorast;
    ClassAST *classast;

    std::ostringstream msg;

    unsigned int varScopeDepth, funScopeDepth;

    if (input == NULL) {
        //NOP
        //throw CompilerException("Null value found");
        return NULL;
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
            if ((veast->name == "done") || (veast->name == "return")) {
                break;
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
            scopeAndTypesPass(input->children[0]);
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
                else {
                    if (vi->type.containedTypes.size() < 1) {
                        throw CompilerException("Internal error: array representation insufficient");
                    }
                    else {
                        input->programmaticType = vi->type.containedTypes[0];
                    }
                }

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
            scopeAndTypesPass(input->children[0]);

            if (beast->op == "::") {
                std::map<std::string, ActorAST*>::iterator location =
                    this->actors.find(input->children[0]->programmaticType.declType);
                if (location == this->actors.end()) {
                    msg << "Could not find referenced actor: '" <<
                        input->children[0]->programmaticType.declType << "'";
                    throw CompilerException(msg.str(), input->children[0]->filepos);
                }
                else {
                    CallExprAST *as_call = dynamic_cast<CallExprAST*>(input->children[1]);
                    if (as_call != NULL) {
                        //visit children
                        for (unsigned int i = 0; i < as_call->children.size(); ++i) {
                            scopeAndTypesPass(as_call->children[i]);
                        }
                    }

                    bool isFound = false;
                    for (std::vector<ASTNode*>::iterator citer = location->second->children.begin(),
                            cend = location->second->children.end(); citer != cend; ++citer) {

                        if ((*citer)->type() == NodeType::Action) {
                            proto = dynamic_cast<PrototypeAST*>((*citer)->children[0]);
                            //FIXME: This does -not- allow overloading
                            if (as_call->name == proto->name) {
                                isFound = true;
                            }
                        }
                    }
                    if (!isFound) {
                        throw CompilerException("Could not find action", input->children[1]->filepos);
                    }
                }
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
                    if (as_call != NULL) {
                        //visit children
                        for (unsigned int i = 0; i < as_call->children.size(); ++i) {
                            scopeAndTypesPass(as_call->children[i]);
                        }
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
                                    input->children[1]->programmaticType = varDecl->vi->type;
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
                                    input->children[1]->programmaticType = proto->returnType;
                                }
                            }

                        }
                    }
                    if (!isFound) {
                        throw CompilerException("Could not find referenced item", input->children[1]->filepos);
                    }
                }
            }
            else {
                scopeAndTypesPass(input->children[1]);

                if (input->children[0]->programmaticType !=
                    input->children[1]->programmaticType) {
                    msg << "Unmatched types: " << input->children[0]->programmaticType.declType
                        << "(" << input->children[0]->programmaticType.containerType << ")"
                        << "[" << input->children[0]->programmaticType.containedTypes.size() << "]"
                        << " vs " << input->children[1]->programmaticType.declType
                        << "(" << input->children[1]->programmaticType.containerType << ")"
                        << "[" << input->children[1]->programmaticType.containedTypes.size() << "]";
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
            input->programmaticType.declType = "string";
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
            for (unsigned int i = 0; i < input->children.size(); ++i) {
                scopeAndTypesPass(input->children[i]);
            }
            break;
        case (NodeType::Prototype):
            proto = dynamic_cast<PrototypeAST*>(input);
            if (proto == NULL) {
                throw CompilerException("FIXME: prototype compiler exception");
            }
            //std::cout << "Pushed: " << proto->name << std::endl;
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                scopeAndTypesPass(*cnode);
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
            for (unsigned int i = 0; i < input->children.size(); ++i) {
                scopeAndTypesPass(input->children[i]);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::Action):
            varScopeDepth = varScopeStack.size();
            for (unsigned int i = 0; i < input->children.size(); ++i) {
                scopeAndTypesPass(input->children[i]);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::While):
            varScopeDepth = varScopeStack.size();
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                scopeAndTypesPass(*cnode);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::If):
            varScopeDepth = varScopeStack.size();
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                scopeAndTypesPass(*cnode);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::Block):
            varScopeDepth = varScopeStack.size();
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                scopeAndTypesPass(*cnode);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            break;
        case (NodeType::Actor):
            funScopeDepth = funScopeStack.size();
            varScopeDepth = varScopeStack.size();

            actorast = dynamic_cast<ActorAST*>(input);
            if (actorast == NULL) {
                throw CompilerException("Internal error: actor reference is null",
                        input->filepos);
            }

            //push variable that will represent "this"
            vi = new VariableInfo();
            vi->name = "this";
            vi->scopeType = ScopeType::Struct;
            vi->type.declType = actorast->name;
            vi->type.containerType = ContainerType::Scalar;
            varScopeStack.push_back(vi);

            //find prototypes and variables that are in the closed scope, and
            //do those first.
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {

                if (*cnode == NULL) {
                    throw CompilerException("Internal error: child is null", input->filepos);
                }
                switch ((*cnode)->type()) {
                    case (NodeType::Function) :
                        proto = dynamic_cast<PrototypeAST*>((*cnode)->children[0]);
                        funScopeStack.push_back(proto);

                        //analyseScopeAndTypes((*cnode)->children[0]);
                        break;
                    case (NodeType::Action) :
                        proto = dynamic_cast<PrototypeAST*>((*cnode)->children[0]);
                        funScopeStack.push_back(proto);

                        //analyseScopeAndTypes((*cnode)->children[0]);
                        break;
                    case (NodeType::VarDecl) :
                        scopeAndTypesPass(*cnode);
                        break;
                    default :
                        //do nothing
                        break;
                }
            }
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                switch ((*cnode)->type()) {
                    case (NodeType::VarDecl) :
                        break;
                    default :
                        scopeAndTypesPass(*cnode);
                        break;
                }

                //analyseScopeAndTypes(*cnode);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            while (funScopeDepth != funScopeStack.size()) {
                funScopeStack.pop_back();
            }
            break;
        case (NodeType::Class):
            funScopeDepth = funScopeStack.size();
            varScopeDepth = varScopeStack.size();

            classast = dynamic_cast<ClassAST*>(input);
            if (classast == NULL) {
                throw CompilerException("Internal error: class reference is null",
                        input->filepos);
            }

            //FIXME: this will leak
            //push variable that will represent "this"
            vi = new VariableInfo();
            vi->name = "this";
            vi->scopeType = ScopeType::Struct;
            vi->type.declType = classast->name;
            vi->type.containerType = ContainerType::Scalar;
            varScopeStack.push_back(vi);

            //find prototypes and variables that are in the closed scope, and
            //do those first.
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {

                if (*cnode == NULL) {
                    throw CompilerException("Internal error: child is null", input->filepos);
                }
                switch ((*cnode)->type()) {
                    case (NodeType::Function) :
                        proto = dynamic_cast<PrototypeAST*>((*cnode)->children[0]);
                        funScopeStack.push_back(proto);

                        //analyseScopeAndTypes((*cnode)->children[0]);
                        break;
                    case (NodeType::VarDecl) :
                        scopeAndTypesPass(*cnode);
                    default :
                        //do nothing
                        break;
                }
            }
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                switch ((*cnode)->type()) {
                    case (NodeType::VarDecl) :
                        break;
                    default :
                        scopeAndTypesPass(*cnode);
                        break;
                }

                //analyseScopeAndTypes(*cnode);
            }
            while (varScopeDepth != varScopeStack.size()) {
                varScopeStack.pop_back();
            }
            while (funScopeDepth != funScopeStack.size()) {
                funScopeStack.pop_back();
            }
            break;
        case (NodeType::App):
            funScopeDepth = funScopeStack.size();
            varScopeDepth = varScopeStack.size();
            //find prototypes and variables that are in the closed scope, and
            //do those first.
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {

                if (*cnode == NULL) {
                    throw CompilerException("Internal error: child is null", input->filepos);
                }
                switch ((*cnode)->type()) {
                    case (NodeType::Function) :
                        proto = dynamic_cast<PrototypeAST*>((*cnode)->children[0]);
                        funScopeStack.push_back(proto);

                        //analyseScopeAndTypes((*cnode)->children[0]);
                        break;
                    case (NodeType::Action) :
                        proto = dynamic_cast<PrototypeAST*>((*cnode)->children[0]);
                        funScopeStack.push_back(proto);

                        //analyseScopeAndTypes((*cnode)->children[0]);
                        break;
                    case (NodeType::Actor) :
                        actorast = dynamic_cast<ActorAST*>(*cnode);
                        if (actorast == NULL) {
                            throw CompilerException("Internal error: actor is null", input->filepos);
                        }
                        actors[actorast->name] = actorast;
                        break;
                    case (NodeType::Class) :
                        classast = dynamic_cast<ClassAST*>(*cnode);
                        if (classast == NULL) {
                            throw CompilerException("Internal error: class is null", input->filepos);
                        }
                        classes[classast->name] = classast;
                        break;
                    case (NodeType::VarDecl) :
                        scopeAndTypesPass(*cnode);
                        break;
                    default :
                        //do nothing
                        break;
                }
            }
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {

                switch ((*cnode)->type()) {
                    case (NodeType::VarDecl) :
                        break;
                    default :
                        scopeAndTypesPass(*cnode);
                        break;
                }

                //analyseScopeAndTypes(*cnode);
            }
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
            break;
    }

    return input;
}

void Analyser::debugOutputPass(ASTNode* input, unsigned int depth) {
    NumberExprAST *neast;
    BooleanExprAST *boeast;
    QuoteExprAST *qeast;
    VariableExprAST *veast;
    ArrayIndexedExprAST *aieast;
    BinaryExprAST *beast;
    CallExprAST *ceast;
    PrototypeAST *proto;
    VarDeclExprAST *vdeast;
    ActorAST *actorast;
    ClassAST *classast;

    std::ostringstream msg;

    if (input == NULL) {
        //NOP
        return;
    }
    switch (input->type()) {
        case (NodeType::Number) :
            neast = dynamic_cast<NumberExprAST*>(input);
            if (neast == NULL) {
                throw CompilerException("FIXME: Number compiler exception\n");
            }
            indentDepth(depth);
            std::cout << "Num: " << neast->val << std::endl;
            break;
        case (NodeType::Boolean) :
            boeast = dynamic_cast<BooleanExprAST*>(input);
            if (boeast == NULL) {
                throw CompilerException("FIXME: Boolean compiler exception\n");
            }
            indentDepth(depth);
            std::cout << "Bool: " << boeast->val << std::endl;
            break;
        case (NodeType::Variable) :
            veast = dynamic_cast<VariableExprAST*>(input);
            if (veast == NULL) {
                throw CompilerException("FIXME: Variable compiler exception\n");
            }
            indentDepth(depth);
            std::cout << "Var: " << veast->name << std::endl;
            break;
        case (NodeType::ArrayIndexed) :
            aieast = dynamic_cast<ArrayIndexedExprAST*>(input);
            if (input->children.size() == 0) {
                throw CompilerException("Incomplete array index", input->filepos);
            }
            indentDepth(depth);
            std::cout << "Indexed: " << aieast->name << std::endl;
            debugOutputPass(input->children[0], depth+1);
            break;
        case (NodeType::Binary) :
            beast = dynamic_cast<BinaryExprAST*>(input);
            if (beast == NULL) {
                throw CompilerException("FIXME: Binary compiler exception\n");
            }
            indentDepth(depth);
            std::cout << "Binary: " << beast->op << std::endl;
            debugOutputPass(input->children[0], depth+1);
            debugOutputPass(input->children[1], depth+1);
            break;

        case (NodeType::Quote) :
            qeast = dynamic_cast<QuoteExprAST*>(input);
            if (qeast == NULL) {
                throw CompilerException("FIXME: Quote compiler exception\n");
            }
            indentDepth(depth);
            std::cout << "String: " << qeast->val << std::endl;
            break;
        case (NodeType::Call) :
            ceast = dynamic_cast<CallExprAST*>(input);
            if (ceast == NULL) {
                throw CompilerException("FIXME: Call compiler exception\n");
            }
            indentDepth(depth);
            std::cout << "Call: " << ceast->name << std::endl;
            for (unsigned int i = 0; i < input->children.size(); ++i) {
                debugOutputPass(input->children[i], depth+1);
            }
            break;
        case (NodeType::Prototype):
            proto = dynamic_cast<PrototypeAST*>(input);
            if (proto == NULL) {
                throw CompilerException("FIXME: prototype compiler exception");
            }
            indentDepth(depth);
            std::cout << "Proto: " << proto->name << std::endl;
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                debugOutputPass(*cnode, depth+1);
            }
            break;
        case (NodeType::VarDecl):
            vdeast = dynamic_cast<VarDeclExprAST*>(input);
            if (vdeast == NULL) {
                throw CompilerException("FIXME: vardecl compiler exception");
            }
            indentDepth(depth);
            std::cout << "Decl: " << vdeast->vi->name << std::endl;
            break;
        case (NodeType::Function):
            indentDepth(depth);
            std::cout << "Function: " << std::endl;
            for (unsigned int i = 0; i < input->children.size(); ++i) {
                debugOutputPass(input->children[i], depth+1);
            }
            break;
        case (NodeType::Action):
            indentDepth(depth);
            std::cout << "Action: " << std::endl;
            for (unsigned int i = 0; i < input->children.size(); ++i) {
                debugOutputPass(input->children[i], depth+1);
            }
            break;
        case (NodeType::While):
            indentDepth(depth);
            std::cout << "While: " << std::endl;
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                debugOutputPass(*cnode, depth+1);
            }
            break;
        case (NodeType::If):
            indentDepth(depth);
            std::cout << "If: " << std::endl;
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                debugOutputPass(*cnode, depth+1);
            }
            break;
        case (NodeType::Block):
            indentDepth(depth);
            std::cout << "Block: " << std::endl;
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {
                debugOutputPass(*cnode, depth+1);
            }
            break;
        case (NodeType::Actor):
            actorast = dynamic_cast<ActorAST*>(input);
            if (actorast == NULL) {
                throw CompilerException("Internal error: actor reference is null",
                        input->filepos);
            }
            indentDepth(depth);
            std::cout << "Actor: " << actorast->name << std::endl;

            //find prototypes and variables that are in the closed scope, and
            //do those first.
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {

                debugOutputPass(*cnode, depth+1);
            }

            break;
        case (NodeType::Class):
            classast = dynamic_cast<ClassAST*>(input);
            if (classast == NULL) {
                throw CompilerException("Internal error: class reference is null",
                        input->filepos);
            }

            indentDepth(depth);
            std::cout << "Class: " << classast->name << std::endl;

            //find prototypes and variables that are in the closed scope, and
            //do those first.
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {

                debugOutputPass(*cnode, depth+1);
            }
            break;
        case (NodeType::App):
            indentDepth(depth);
            std::cout << "App: " << std::endl;
            //find prototypes and variables that are in the closed scope, and
            //do those first.
            for (std::vector<ASTNode*>::iterator cnode = input->children.begin(),
                    cend = input->children.end(); cnode != cend; ++cnode) {

                debugOutputPass(*cnode, depth+1);
            }
            break;
        default:
            msg << "Unhandled element during analysis: " << input->type();
            throw CompilerException(msg.str(), input->filepos);
            break;
    }
}
