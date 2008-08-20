#include "codegen_cppoutput.hpp"

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string>

//catch all that will dispatch out to others
boost::shared_ptr<TypeInfo> CodegenCPPOutput::resolveType(ASTNode *ast) {
    boost::shared_ptr<TypeInfo> ti= boost::shared_ptr<TypeInfo>(new TypeInfo);
    TypeInfo typeInfo;

    VariableInfo *vi;

    NumberExprAST *neast;
    BooleanExprAST *boeast;
    QuoteExprAST *qeast;
    VariableExprAST *veast;
    ArrayIndexedExprAST *aieast;
    BinaryExprAST *beast;
    CallExprAST *ceast;

    if (ast == NULL) {
        //NOP
        ti = boost::shared_ptr<TypeInfo>(new TypeInfo);
        return ti;
    }
    //Number, Variable, ArrayIndexed, Binary, Quote, Call, DefFun, End, VarDecl, ArrayDecl, If, While
    switch (ast->type()) {
        case (NodeType::Number) :
            neast = dynamic_cast<NumberExprAST*>(ast);
            if (neast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            ti.get()->declType = "int";
            ti.get()->containerType = ContainerType::Scalar;
            break;
        case (NodeType::Boolean) :
            boeast = dynamic_cast<BooleanExprAST*>(ast);
            if (boeast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            ti.get()->declType = "bool";
            ti.get()->containerType = ContainerType::Scalar;
            break;
        case (NodeType::Variable) :
            veast = dynamic_cast<VariableExprAST*>(ast);
            if (veast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            vi = findVarInScope(veast->name);
            if (vi != NULL) {
                ti.get()->declType = vi->type.declType;
                ti.get()->containerType = vi->type.containerType;
            }
            else {
                std::ostringstream msg;
                msg << "Can not find variable '" << veast->name << "'";
                throw CompilerException(msg.str(), ast->filepos);
            }
            break;
        case (NodeType::ArrayIndexed) :
            aieast = dynamic_cast<ArrayIndexedExprAST*>(ast);
            if (aieast == NULL) {
                printf("FIXME: Array indexed compiler exception\n");
            }
            vi = findVarInScope(aieast->name);
            if (vi != NULL) {
                ti.get()->declType = vi->type.declType;
                ti.get()->containerType = vi->type.containerType;
            }
            else {
                std::ostringstream msg;
                msg << "Can not find variable '" << aieast->name << "'";
                throw CompilerException(msg.str(), ast->filepos);
            }
            break;
        case (NodeType::Binary) :
            beast = dynamic_cast<BinaryExprAST*>(ast);
            if (beast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            ti = resolveType(beast->children[0]);
            break;
        case (NodeType::Quote) :
            qeast = dynamic_cast<QuoteExprAST*>(ast);
            if (qeast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            ti.get()->declType = "quote";
            ti.get()->containerType = ContainerType::Scalar;
            break;
        case (NodeType::Call) :
            ceast = dynamic_cast<CallExprAST*>(ast);
            if (ceast == NULL) {
                printf("FIXME: Call compiler exception\n");
            }
            typeInfo = lookupReturnTypeInfo(ceast);
            ti.get()->declType = typeInfo.declType;
            ti.get()->containerType = typeInfo.containerType;
            //gc = visit(ceast);
            break;
        default:
            throw CompilerException("Can't resolve type during lookup", ast->filepos);
    }

    return ti;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::handleCall(CallExprAST *ast, const std::string &container, const std::string &container_name) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode), gc_temp;
    std::vector<boost::shared_ptr<GeneratedCode> > gc_args;
    bool isExtern = checkIfExtern(ast->name);

    for (int i = 0, j = ast->children.size(); i != j; ++i) {
        gc_args.push_back(visit (ast->children[i]));
    }

    for (int i = 0, j = ast->children.size(); i != j; ++i) {
        //if (i != 0)
        //    gc.get()->inits << ", ";
        if (gc_args[i].get()->decls.str() != "") {
            gc.get()->decls << gc_args[i].get()->decls.str();
        }
        if (gc_args[i].get()->inits.str() != "") {
            gc.get()->inits << gc_args[i].get()->inits.str();
        }
    }

    if (ast->name == "return") {
        //if this is a return call, we need to clear our scope stack up to where we came from

        if (gc_args.size() > 1) {
            throw CompilerException("Too many arguments in return", ast->filepos);
        }

        int unwindAmount = currentScopeCount.back();
        for (int i = 0; i < unwindAmount; ++i) {
            VariableInfo *vi = scopeStack[scopeStack.size()-1-i];
            if ((vi->type.requiresCopyDelete())&&(!checkIfActor(vi->type.declType))) {
                //since it's a return, we don't want to delete what we're returning, so be careful.  This isn't the best way to do this, so I'm open to suggestions.
                if ((gc_args.size() == 1) && (vi->name != gc_args[0].get()->output.str())) {
                    gc.get()->inits << "if (" << vi->name << " != NULL) {" << std::endl;
                    gc.get()->inits << "  delete(" << vi->name << ");" << std::endl;
                    gc.get()->inits << "}" << std::endl;
                }
            }
        }

        gc.get()->output << "return (";
        for (int i = 0, j = ast->children.size(); i != j; ++i) {
            if (i != 0)
                gc.get()->output << ", ";

            if (gc_args[i].get()->output.str() != "") {
                gc.get()->output << gc_args[i].get()->output.str();
            }
        }
        gc.get()->output << ")";

        //"return" is a special case, so get out of here
        return gc;
    }

    std::string tmpName = nextTemp();
    std::string retType = lookupReturnType(ast, container);
    if (retType != "void") {
        gc.get()->decls << retType << " " << tmpName << ";" << std::endl;
    }

    gc.get()->inits << "case(" << currentContId << "):" << std::endl;
    ++currentContId;

    gc.get()->inits << outputResumeBlock();

    if (!isExtern) {
        gc.get()->inits << "actor__->parentThread->timeSliceEndTime = timeLeft__;" << std::endl;
    }

    if (retType != "void") {
        gc.get()->inits << tmpName << " = ";
        if (container_name != "") {
            gc.get()->inits << container_name << "->";
        }
        gc.get()->inits << ast->name << "(";
    }
    else {
        if (container_name != "") {
            gc.get()->inits << container_name << "->";
        }
        gc.get()->inits << ast->name << "(";
    }

    for (int i = 0, j = ast->children.size(); i != j; ++i) {
        if (i != 0)
            gc.get()->inits << ", ";

        if (gc_args[i].get()->output.str() != "") {
            gc.get()->inits << gc_args[i].get()->output.str();
        }
    }

    if (!isExtern) {
        if (ast->children.size() > 0) {
            gc.get()->inits << ", ";
        }
        gc.get()->inits << "actor__";
    }

    gc.get()->inits << ");" << std::endl;
    if (isExtern) {
        //since the external call will no decrement the timeslice, we need to do it manually
        gc.get()->inits << "--timeLeft__;" << std::endl;
    }
    if (!isExtern) {
        gc.get()->inits << "timeLeft__ = actor__->parentThread->timeSliceEndTime;" << std::endl;
    }
    gc.get()->inits << outputPauseBlock(false);

    if (retType != "void") {
        gc.get()->output << tmpName;
    }

    return gc;
}

//catch all that will dispatch out to others
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ASTNode *ast) {
    boost::shared_ptr<GeneratedCode> gc;

    NumberExprAST *neast;
    BooleanExprAST *boeast;
    QuoteExprAST *qeast;
    VariableExprAST *veast;
    VarDeclExprAST *vdeast;
    ArrayIndexedExprAST *aieast;
    //ArrayDeclExprAST *adeast;
    EndExprAST *eeast;
    IfExprAST *ieast;
    WhileExprAST *weast;
    BinaryExprAST *beast;
    CallExprAST *ceast;
    ActionAST *actast;
    FunctionAST *funast;
    ActorAST *actorast;
    ClassAST *classast;
    AppAST *appast;

    if (ast == NULL) {
        //NOP
        gc = boost::shared_ptr<GeneratedCode>(new GeneratedCode);
        return gc;
    }
    //Number, Variable, ArrayIndexed, Binary, Quote, Call, DefFun, End, VarDecl, If, While
    switch (ast->type()) {
        case (NodeType::Number) :
            neast = dynamic_cast<NumberExprAST*>(ast);
            if (neast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            gc = visit(neast);
            break;
        case (NodeType::Boolean) :
            boeast = dynamic_cast<BooleanExprAST*>(ast);
            if (boeast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            gc = visit(boeast);
            break;
        case (NodeType::Variable) :
            veast = dynamic_cast<VariableExprAST*>(ast);
            if (veast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            gc = visit(veast);
            break;
        case (NodeType::ArrayIndexed) :
            aieast = dynamic_cast<ArrayIndexedExprAST*>(ast);
            if (aieast == NULL) {
                printf("FIXME: Array indexed compiler exception\n");
            }
            gc = visit(aieast);
            break;
        case (NodeType::Binary) :
            beast = dynamic_cast<BinaryExprAST*>(ast);
            if (beast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            gc = visit(beast);
            break;
        case (NodeType::Quote) :
            qeast = dynamic_cast<QuoteExprAST*>(ast);
            if (qeast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            gc = visit(qeast);
            break;
        case (NodeType::Call) :
            ceast = dynamic_cast<CallExprAST*>(ast);
            if (ceast == NULL) {
                printf("FIXME: Call compiler exception\n");
            }
            gc = visit(ceast);
            break;
        case (NodeType::End) :
            eeast = dynamic_cast<EndExprAST*>(ast);
            if (eeast == NULL) {
                printf("FIXME: End compiler exception\n");
            }
            gc = visit(eeast);
            break;
        case (NodeType::VarDecl) :
            vdeast = dynamic_cast<VarDeclExprAST*>(ast);
            if (vdeast == NULL) {
                printf("FIXME: VarDecl compiler exception\n");
            }
            gc = visit(vdeast);
            break;
        case (NodeType::If) :
            ieast = dynamic_cast<IfExprAST*>(ast);
            if (ieast == NULL) {
                printf("FIXME: If compiler exception\n");
            }

            gc = visit(ieast);
            break;
        case (NodeType::While) :
            weast = dynamic_cast<WhileExprAST*>(ast);
            if (weast == NULL) {
                printf("FIXME: While compiler exception\n");
            }
            gc = visit(weast);
            break;
        case (NodeType::Action) :
            actast = dynamic_cast<ActionAST*>(ast);
            if (actast == NULL) {
                printf("FIXME: action compiler exception\n");
            }
            gc = visit(actast);
            break;
        case (NodeType::Function) :
            funast = dynamic_cast<FunctionAST*>(ast);
            if (funast == NULL) {
                printf("FIXME: function compiler exception\n");
            }
            gc = visit(funast);
            break;
        case (NodeType::Actor) :
            actorast = dynamic_cast<ActorAST*>(ast);
            if (actorast == NULL) {
                printf("FIXME: actor compiler exception\n");
            }
            gc = visit(actorast);
            break;
        case (NodeType::Class) :
            classast = dynamic_cast<ClassAST*>(ast);
            if (classast == NULL) {
                printf("FIXME: class compiler exception\n");
            }
            gc = visit(classast);
            break;
        case (NodeType::App) :
            appast = dynamic_cast<AppAST*>(ast);
            if (appast == NULL) {
                printf("FIXME: app compiler exception\n");
            }
            gc = visit(appast);
            break;
        default :
            throw CompilerException("Unknown element", ast->filepos);
    }

    return gc;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(NumberExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    gc.get()->output << ast->val;

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(QuoteExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    gc.get()->output << "\"" << ast->val << "\"";

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(BooleanExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    if (ast->val == true) {
        gc.get()->output << "true";
    }
    else {
        gc.get()->output << "false";
    }
    return gc;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(VariableExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    //TODO: Putting these here for now, but we should handle keywords in a more special place in the future
    if ((ast->name == "return") || (ast->name == "done")) {
        if (inAction) {
            //if this is a return call, we need to clear our scope stack up to where we came from
            int unwindAmount = currentScopeCount.back();
            for (int i = 0; i < unwindAmount; ++i) {
                VariableInfo *vi = scopeStack[scopeStack.size()-1-i];
                if ((vi->type.requiresCopyDelete())&&(!checkIfActor(vi->type.declType))) {
                    gc.get()->output << "if (" << vi->name << " != NULL) {" << std::endl;
                    gc.get()->output << "  delete(" << vi->name << ");" << std::endl;
                    gc.get()->output << "}" << std::endl;
                }
            }

            if (ast->name == "return") {
                gc.get()->output << "--timeLeft__;" << std::endl;
                gc.get()->output << "actor__->parentThread->timeSliceEndTime = timeLeft__;" << std::endl;
                gc.get()->output << "actor__->actorState = ActorState::WAITING_FOR_ACTION;" << std::endl;
                gc.get()->output << "return; " << std::endl;
            }
            else if (ast->name == "done") {
                gc.get()->output << "--timeLeft__;" << std::endl;
                gc.get()->output << "actor__->parentThread->timeSliceEndTime = timeLeft__;" << std::endl;
                gc.get()->output << "actor__->actorState = ActorState::DELETED;" << std::endl;
                gc.get()->output << "return; " << std::endl;
            }
            return gc;
        }
    }

    VariableInfo *vi = findVarInScope(ast->name);
    if (vi == NULL) {
        std::ostringstream oss;
        oss << "Unknown variable '" << ast->name << "'";
        throw CompilerException(oss.str(), ast->filepos);
    }

    if (vi->scopeType == ScopeType::Actor) {
        if (vi->name == "this") {
            gc.get()->output << "actor__->actorId";
        }
        else {
            gc.get()->output << "actor__->" << ast->name;
        }
    }
    else {
        gc.get()->output << ast->name;
    }

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(VarDeclExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    VariableInfo *vi = ast->vi; //new VariableInfo(ast->name, ast->declType, ContainerType::Scalar, ScopeType::CodeBlock);

    if (vi->type.containerType == ContainerType::Scalar) {
        std::map<std::string, ActorAST*>::iterator finder = actors.find(vi->type.declType);

        if (finder != actors.end()) {
            if (ast->isSpawn) {
                std::string actorTemp = nextTemp();
                gc.get()->decls << "actorId_t " << vi->name << ";" << std::endl;
                gc.get()->inits << vi->type.declType << " *" << actorTemp << ";" << std::endl;
                gc.get()->output << "{ " << actorTemp << " = new " << vi->type.declType << "();" << std::endl;
                if (finder->second->isIsolated) {
                    gc.get()->output << "  actor__->parentThread->ScheduleNewIsolatedActor(" << actorTemp << ");" << std::endl;
                }
                else {
                    gc.get()->output << "  actor__->parentThread->ScheduleNewActor(" << actorTemp << ");" << std::endl;
                }
                gc.get()->output << "  " << vi->name << " = " << actorTemp << "->actorId; }" << std::endl;
            }
            else {
                gc.get()->decls << "actorId_t " << vi->name << ";" << std::endl;
            }
        }
        else {
            gc.get()->decls << lookupAssocType(vi->type) << " " << vi->name << ";" << std::endl;
            if (ast->isAlloc) {
                //TRIM
                std::string allocType = lookupAssocType(vi->type);
                allocType = allocType.erase(allocType.size()-1);

                gc.get()->output << vi->name << " = new " << allocType << "();" << std::endl;
            }
            gc.get()->output << vi->name;
        }
    }
    else if (vi->type.containerType == ContainerType::Array) {
        std::map<std::string, ActorAST*>::iterator finder = actors.find(vi->type.declType);

        if (finder != actors.end()) {
            if (ast->isSpawn) {
                std::string tmpName = nextTemp();
                gc.get()->decls << "int " << tmpName << ";" << std::endl;

                std::string actorTemp = nextTemp();
                std::string loopTemp = nextTemp();
                gc.get()->decls << vi->type.declType << " *" << actorTemp << ";" << std::endl;

                gc.get()->decls << "std::vector<actorId_t> *" << vi->name << ";" << std::endl;

                gc.get()->output << tmpName << " = ";

                if (vi->size != NULL) {
                    boost::shared_ptr<GeneratedCode> gc_temp = visit (vi->size);
                    if (gc_temp.get()->decls.str() != "") {
                        gc.get()->decls << gc_temp.get()->decls.str();
                    }
                    if (gc_temp.get()->inits.str() != "") {
                        gc.get()->output << gc_temp.get()->inits.str();
                    }
                    if (gc_temp.get()->output.str() != "") {
                        gc.get()->output << gc_temp.get()->output.str();
                    }
                }
                else {
                    gc.get()->output << "0";
                }
                gc.get()->output << ";" << std::endl;
                gc.get()->output << vi->name << " = new std::vector<actorId_t>(" << tmpName << ");" << std::endl;

                gc.get()->output << "for (int " << loopTemp << "=0; " << loopTemp << " < " << tmpName << "; ++" << loopTemp << ") {" << std::endl;
                gc.get()->output << "  " << actorTemp << " = new " << vi->type.declType << "();" << std::endl;
                if (finder->second->isIsolated) {
                    gc.get()->output << "  actor__->parentThread->ScheduleNewIsolatedActor(" << actorTemp << ");" << std::endl;
                }
                else {
                    gc.get()->output << "  actor__->parentThread->ScheduleNewActor(" << actorTemp << ");" << std::endl;
                }
                gc.get()->output << "  (*" << vi->name << ")[" << loopTemp << "] = " << actorTemp << "->actorId;" << std::endl;
                gc.get()->output << "}" << std::endl;
            }
            else {
                gc.get()->decls << "std::vector<actorId_t> *" << vi->name << ";" << std::endl;
                gc.get()->output << vi->name << " = new std::vector<actorId_t>(";
                if (vi->size != NULL) {
                    boost::shared_ptr<GeneratedCode> gc_temp = visit (vi->size);
                    if (gc_temp.get()->decls.str() != "") {
                        gc.get()->decls << gc_temp.get()->decls.str();
                    }
                    if (gc_temp.get()->inits.str() != "") {
                        gc.get()->output << gc_temp.get()->inits.str();
                    }
                    if (gc_temp.get()->output.str() != "") {
                        gc.get()->output << gc_temp.get()->output.str();
                    }
                }
                else {
                    gc.get()->output << "0";
                }
                gc.get()->output << ");" << std::endl;
            }
        }
        else {
            gc.get()->decls << lookupAssocType(vi->type) << " " << vi->name << ";" << std::endl;
            std::string allocType = lookupAssocType(vi->type);
            allocType = allocType.erase(allocType.size()-1); //TRIM off the trailing '*'
            gc.get()->output << vi->name << " = new " << allocType << "(";
            if (vi->size != NULL) {
                boost::shared_ptr<GeneratedCode> gc_temp = visit (vi->size);
                if (gc_temp.get()->decls.str() != "") {
                    gc.get()->decls << gc_temp.get()->decls.str();
                }
                if (gc_temp.get()->inits.str() != "") {
                    gc.get()->output << gc_temp.get()->inits.str();
                }
                if (gc_temp.get()->output.str() != "") {
                    gc.get()->output << gc_temp.get()->output.str();
                }
            }
            else {
                gc.get()->output << "0";
            }
            gc.get()->output << ")";

            //FIXME?  Would you ever follow an array decl with an assignment?  If so, we should probably check for that
        }
    }
    ++(currentScopeCount.back());
    scopeStack.push_back(vi);

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ArrayIndexedExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    VariableInfo *vi = findVarInScope(ast->name);
    if (vi == NULL) {
        std::ostringstream oss;
        oss << "Unknown array variable '" << ast->name << "'";
        throw CompilerException(oss.str(), ast->filepos);
    }
    else if (vi->type.containerType != ContainerType::Array) {
        std::ostringstream oss;
        oss << "Variable '" << ast->name << "' is not of an array type";
        throw CompilerException(oss.str(), ast->filepos);
    }
    if (vi->scopeType == ScopeType::Actor) {
        gc.get()->output << "(*(actor__->" << ast->name << "))";
    }
    else {
        gc.get()->output << "(*" << ast->name << ")";
    }
    gc.get()->output << "[";
    boost::shared_ptr<GeneratedCode> gc_temp = visit (ast->children[0]);
    if (gc_temp.get()->decls.str() != "") {
        gc.get()->decls << gc_temp.get()->decls.str();
    }
    if (gc_temp.get()->inits.str() != "") {
        gc.get()->output << gc_temp.get()->inits.str();
    }
    if (gc_temp.get()->output.str() != "") {
        gc.get()->output << gc_temp.get()->output.str();
    }
    gc.get()->output << "]";

    return gc;
}
/*
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ArrayDeclExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    VariableInfo *vi = new VariableInfo(ast->name, ast->declType, TypeType::Array, ast->size, ScopeType::CodeBlock);

    ++(currentScopeCount.back());
    scopeStack.push_back(vi);

    std::map<std::string, ActorAST*>::iterator finder = actors.find(ast->declType);

    if (finder != actors.end()) {
        if (ast->isSpawn) {
            std::string tmpName = nextTemp();
            gc.get()->decls << "int " << tmpName << ";" << std::endl;

            std::string actorTemp = nextTemp();
            std::string loopTemp = nextTemp();
            gc.get()->decls << ast->declType << " *" << actorTemp << ";" << std::endl;

            gc.get()->decls << "std::vector<actorId_t> *" << ast->name << ";" << std::endl;

            gc.get()->output << tmpName << " = ";

            if (ast->size != NULL) {
                boost::shared_ptr<GeneratedCode> gc_temp = visit (ast->size);
                if (gc_temp.get()->decls.str() != "") {
                    gc.get()->decls << gc_temp.get()->decls.str();
                }
                if (gc_temp.get()->inits.str() != "") {
                    gc.get()->output << gc_temp.get()->inits.str();
                }
                if (gc_temp.get()->output.str() != "") {
                    gc.get()->output << gc_temp.get()->output.str();
                }
            }
            else {
                gc.get()->output << "0";
            }
            gc.get()->output << ";" << std::endl;
            gc.get()->output << ast->name << " = new std::vector<actorId_t>(" << tmpName << ");" << std::endl;

            gc.get()->output << "for (int " << loopTemp << "=0; " << loopTemp << " < " << tmpName << "; ++" << loopTemp << ") {" << std::endl;
            gc.get()->output << "  " << actorTemp << " = new " << ast->declType << "();" << std::endl;
            if (finder->second->isIsolated) {
                gc.get()->output << "  actor__->parentThread->ScheduleNewIsolatedActor(" << actorTemp << ");" << std::endl;
            }
            else {
                gc.get()->output << "  actor__->parentThread->ScheduleNewActor(" << actorTemp << ");" << std::endl;
            }
            gc.get()->output << "  (*" << ast->name << ")[" << loopTemp << "] = " << actorTemp << "->actorId;" << std::endl;
            gc.get()->output << "}" << std::endl;
        }
        else {
            gc.get()->decls << "std::vector<actorId_t> *" << ast->name << ";" << std::endl;
            gc.get()->output << ast->name << " = new std::vector<actorId_t>(";
            if (ast->size != NULL) {
                boost::shared_ptr<GeneratedCode> gc_temp = visit (ast->size);
                if (gc_temp.get()->decls.str() != "") {
                    gc.get()->decls << gc_temp.get()->decls.str();
                }
                if (gc_temp.get()->inits.str() != "") {
                    gc.get()->output << gc_temp.get()->inits.str();
                }
                if (gc_temp.get()->output.str() != "") {
                    gc.get()->output << gc_temp.get()->output.str();
                }
            }
            else {
                gc.get()->output << "0";
            }
            gc.get()->output << ");" << std::endl;
        }
    }
    else {
        gc.get()->decls << lookupAssocType(vi->type) << " " << ast->name << ";" << std::endl;
        std::string allocType = lookupAssocType(vi->type);
        allocType = allocType.erase(allocType.size()-1); //TRIM off the trailing '*'
        gc.get()->output << ast->name << " = new " << allocType << "(";
        if (ast->size != NULL) {
            boost::shared_ptr<GeneratedCode> gc_temp = visit (ast->size);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }
        }
        else {
            gc.get()->output << "0";
        }
        gc.get()->output << ")";

        //FIXME?  Would you ever follow an array decl with an assignment?  If so, we should probably check for that
    }
    return gc;
}
*/
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(EndExprAST *ast) {
    //do nothing
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    return gc;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(IfExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);
    boost::shared_ptr<GeneratedCode> gc_cond;

    std::string condTemp = nextTemp();

    gc.get()->decls << "bool " << condTemp << ";" << std::endl;

    gc_cond = visit (ast->children[0]);
    if (gc_cond.get()->decls.str() != "") {
        gc.get()->decls << gc_cond.get()->decls.str();
    }
    if (gc_cond.get()->inits.str() != "") {
        gc.get()->output << gc_cond.get()->inits.str();
    }

    if (gc_cond.get()->output.str() != "") {
        gc.get()->output << condTemp << " = " << gc_cond.get()->output.str() << ";" << std::endl;
    }

    gc.get()->output << "if (" << condTemp << ") {" << std::endl;

    for (std::vector<ASTNode*>::iterator iter = ast->children[1]->children.begin(),
            end = ast->children[1]->children.end(); iter != end; ++iter) {

        boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->decls << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
        gc.get()->output << ";" << std::endl;
    }
    gc.get()->output << "}";
    if (ast->children[2]->children.size() > 0) {
        gc.get()->output << "else {" << std::endl;
        for (std::vector<ASTNode*>::iterator iter = ast->children[2]->children.begin(),
                end = ast->children[2]->children.end(); iter != end; ++iter) {

            boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }
            gc.get()->output << ";" << std::endl;
        }
        gc.get()->output << "}" << std::endl;
    }

    return gc;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(WhileExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);
    boost::shared_ptr<GeneratedCode> gc_cond, gc_temp;

    std::string condTemp = nextTemp();

    gc.get()->decls << "bool " << condTemp << ";" << std::endl;
    gc.get()->output << "case(" << currentContId << "):" << std::endl;
    ++currentContId;
    gc.get()->output << (outputResumeBlock());

    gc_cond = visit (ast->children[0]);
    if (gc_cond.get()->decls.str() != "") {
        gc.get()->decls << gc_cond.get()->decls.str();
    }
    if (gc_cond.get()->inits.str() != "") {
        gc.get()->output << gc_cond.get()->inits.str();
    }
    if (gc_cond.get()->output.str() != "") {
        gc.get()->output << condTemp << " = " << gc_cond.get()->output.str() << ";" << std::endl;
    }
    gc.get()->output << "while (" << condTemp << ") {" << std::endl;
    gc.get()->output << (outputPauseBlock(true));
    for (std::vector<ASTNode*>::iterator iter = ast->children[1]->children.begin(),
            end = ast->children[1]->children.end(); iter != end; ++iter) {
        gc_temp = visit (*iter);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->decls << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }

        gc.get()->output << ";" << std::endl;
    }
    gc_cond = visit (ast->children[0]);
    if (gc_cond.get()->decls.str() != "") {
        gc.get()->decls << gc_cond.get()->decls.str();
    }
    if (gc_cond.get()->inits.str() != "") {
        gc.get()->output << gc_cond.get()->inits.str();
    }
    if (gc_cond.get()->output.str() != "") {
        gc.get()->output << condTemp << " = " << gc_cond.get()->output.str() << ";" << std::endl;
    }

    /*
    if (gc_cond.get()->output.str() != "") {
        gc.get()->output << condTemp << " = " << gc_cond.get()->output.str() << ";" << std::endl;
    }
    */

    gc.get()->output << "}";

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(BinaryExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);
    boost::shared_ptr<GeneratedCode> gc_temp;

    //std::cout << "Binary: " << ast->op << std::endl;

    if (ast->op == "::") {
        CallExprAST *ceast = dynamic_cast<CallExprAST*>(ast->children[1]);
        VariableExprAST *veast = dynamic_cast<VariableExprAST*>(ast->children[0]);
        ArrayIndexedExprAST *aieast = dynamic_cast<ArrayIndexedExprAST*>(ast->children[0]);

        VariableInfo *vi;

        if (ceast == NULL) {
            //std::cout << "Can't build message, is type: " << ast->RHS->type() << std::endl;
            //exit(1);
            std::ostringstream msg;
            msg << "Can't build message, right hand side is type: " << ast->children[1]->type();
            throw CompilerException(msg.str(), ast->filepos);
        }

        if ((veast == NULL) && (aieast == NULL)) {
            std::ostringstream msg;
            msg << "Can't build message, left hand side is type: " << ast->children[0]->type();
            throw CompilerException(msg.str(), ast->filepos);
        }
        else if (veast != NULL) {
            vi = findVarInScope(veast->name);
            if (vi == NULL) {
                std::ostringstream oss;
                oss << "Unknown variable for message '" << veast->name << "'";
                throw CompilerException(oss.str(), veast->filepos);
            }
        }
        else { //if (aieast != NULL) {
            vi = findVarInScope(aieast->name);
            if (vi == NULL) {
                std::ostringstream oss;
                oss << "Unknown variable for message '" << aieast->name << "'";
                throw CompilerException(oss.str(), aieast->filepos);
            }
        }

        //int argSize = ceast->args.size();
        std::string msgName = nextTemp();
        std::string actorIfLocal = nextTemp();
        std::string msgArray;

        gc.get()->decls << "Actor *" << actorIfLocal << ";" << std::endl;

        gc.get()->output << actorIfLocal << " = actor__->parentThread->ActorIfLocal(";
        gc_temp = visit (ast->children[0]);

        if (gc_temp.get()->decls.str() != "") {
            gc.get()->decls << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
        gc.get()->output << ", actor__);" << std::endl;

        gc.get()->output << "if ((" << actorIfLocal << " == NULL) || (" << actorIfLocal << "->actorState != ActorState::WAITING_FOR_ACTION)) {" << std::endl;
        gc.get()->decls << "Message " << msgName << ";" << std::endl;
        gc.get()->output << "  " << msgName << ".recipient = ";

        gc_temp = visit (ast->children[0]);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->decls << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }

        gc.get()->output << ";" << std::endl;
        gc.get()->output << "  " << msgName << ".numArgs = " << ceast->children.size() << ";" << std::endl;
        gc.get()->output << "  " << msgName << ".messageType = MessageType::ACTION_MESSAGE;" << std::endl;
        gc.get()->output << "  " << msgName << ".task = &" << vi->type.declType << "__" << ceast->name << "_action;" << std::endl;

        //std::cout << "arg size: " << ceast->args.size() << std::endl;
        if (ceast->children.size() > 4 ) {
            msgArray = nextTemp();
            gc.get()->decls << "std::vector<TypeUnion> *" << msgArray << " = new std::vector<TypeUnion>();" << std::endl;
        }
        for (unsigned int i = 0; i < ceast->children.size(); ++i) {
            boost::shared_ptr<TypeInfo> ti = resolveType(ceast->children[i]);

            boost::shared_ptr<GeneratedCode> gc_temp = visit (ceast->children[i]);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                //gc.get()->output << gc_temp.get()->output.str();
                gc.get()->output << "  " << lookupPushForTypeAndBlock(ti, gc_temp.get()->output.str());
                if (ceast->children.size() > 4 ) {
                    gc.get()->output << "  " << msgArray << "->push_back(tmpTU__);" << std::endl;
                }
                else {
                    gc.get()->output << "  " << msgName << ".arg[" << i << "] = tmpTU__;" << std::endl;
                }
            }

            //gc.get()->output << lookupPushForVar(ceast->args[i]);
            //gc.get()->output << ";" << std::endl;
        }
        if (ceast->children.size() > 4 ) {
            gc.get()->output << "  " << msgName << ".arg[0].VoidPtr = " << msgArray << ";" << std::endl;
        }

        gc.get()->output << "actor__->parentThread->SendMessage(" << msgName << ");" << std::endl;
        gc.get()->output << "}" << std::endl;
        gc.get()->output << "else {" << std::endl;

        for (unsigned int i = 0; i < ceast->children.size(); ++i) {
            boost::shared_ptr<TypeInfo> ti = resolveType(ceast->children[i]);
            //gc.get()->output << "tmpTU__.UInt32 = ";

            boost::shared_ptr<GeneratedCode> gc_temp = visit (ceast->children[i]);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                //gc.get()->output << gc_temp.get()->output.str();
                gc.get()->output << "  " << lookupPushForTypeAndBlock(ti, gc_temp.get()->output.str());
                //gc.get()->output << "  " << msgName << ".arg[" << i << "] = tmpTU__;" << std::endl;
            }

            gc.get()->output << ";" << std::endl;
            gc.get()->output << actorIfLocal << "->heapStack.push_back(tmpTU__);" << std::endl;
        }

        gc.get()->output << actorIfLocal << "->task = &" << vi->type.declType << "__" << ceast->name << "_action;" << std::endl;
        gc.get()->output << actorIfLocal << "->actorState = ActorState::ACTIVE;" << std::endl;

        gc.get()->output << "actor__->parentThread->hotActor = " << actorIfLocal << ";" << std::endl;
        gc.get()->output << "if (" << actorIfLocal << "->runQueueRevId != actor__->parentThread->runQueueRevId) {" << std::endl;
        gc.get()->output << " actor__->parentThread->runningActors.push_back(" << actorIfLocal << ");" << std::endl;
        gc.get()->output << " " << actorIfLocal << "->runQueueRevId = actor__->parentThread->runQueueRevId;" << std::endl;
        gc.get()->output << " }" << std::endl;
        gc.get()->output << "}" << std::endl;
    }
    else if (ast->op == ".") {
        VariableExprAST *veast = dynamic_cast<VariableExprAST*>(ast->children[1]);
        CallExprAST *ceast = dynamic_cast<CallExprAST*>(ast->children[1]);
        if ((veast == NULL) && (ceast == NULL)) {
            std::cout << "Can't use '.' in this context, is type: " << ast->children[1]->type() << std::endl;
            exit(1);
        }
        else {
            VariableExprAST *lhs_ast = dynamic_cast<VariableExprAST*>(ast->children[0]);
            //ArrayIndexedExprAST *lhs_aieast = dynamic_cast<ArrayIndexedExprAST*>(ast->LHS);
            if (lhs_ast == NULL) {
                throw CompilerException("Left hand side is not a variable (this is a limitation of the current system)", ast->filepos);
            }
            boost::shared_ptr<TypeInfo> lhs_type = resolveType(ast->children[0]);
            //std::cout << "Accessing type: " << lhs_type.get()->declType << std::endl;

            if (veast != NULL) {
                ClassAST* s = this->classes[lhs_type.get()->declType];
                //check to see if the struct has an attribute member named this
                //TODO: Fix re-enable this

                bool foundVar = false;
                for (std::vector<ASTNode*>::iterator iter = s->children.begin(),
                        end = s->children.end(); iter != end; ++iter) {

                    VarDeclExprAST *vdeast = dynamic_cast<VarDeclExprAST*>(*iter);
                    if (vdeast != NULL) {
                        if (vdeast->vi->name == veast->name) {
                            gc.get()->output << lhs_ast->name << "->" << veast->name;
                            foundVar = true;
                            break;
                        }
                    }
                }
                if (foundVar == false) {
                    std::ostringstream msg;
                    msg << "Can't find '" << veast->name << "' inside of '" << lhs_ast->name << "'";
                    throw CompilerException(msg.str(), ast->filepos);
                }
                /*
                if (s->vars.find(veast->name) != s->vars.end()) {
                    gc.get()->output << lhs_ast->name << "->" << veast->name;
                }
                else {
                    std::ostringstream msg;
                    msg << "Can't find '" << veast->name << "' inside of '" << lhs_ast->name << "'";
                    throw CompilerException(msg.str(), ast->filepos);
                }
                */
            }
            if (ceast != NULL) {
                gc_temp = handleCall(ceast, lhs_type.get()->declType, lhs_ast->name);
                if (gc_temp.get()->decls.str() != "") {
                    gc.get()->decls << gc_temp.get()->decls.str();
                }
                if (gc_temp.get()->inits.str() != "") {
                    gc.get()->output << gc_temp.get()->inits.str();
                }
                if (gc_temp.get()->output.str() != "") {
                    gc.get()->output << gc_temp.get()->output.str();
                }
            }
        }
    }
    else {
        gc_temp = visit (ast->children[0]);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->decls << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->inits << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
        gc.get()->output << ast->op;
        gc_temp = visit (ast->children[1]);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->decls << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->inits << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }
    return gc;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(CallExprAST *ast) {
    return handleCall(ast, "", "");
}


boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ClassAST *ast, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode), gc_temp(new GeneratedCode);

    FunctionAST *funast;
    VarDeclExprAST *varDecl;

    //DeclStage::Stage stage;
    if (stage == DeclStage::FORWARD) {
        this->classes[ast->name] = ast;
        gc.get()->output << "class " << ast->name << ";" << std::endl;
    }
    if (stage == DeclStage::IMPL) {
        std::map<std::string, ActorAST*>::iterator finder;

        currentScopeCount.push_back(0);
        scopeContainerId = currentScopeCount.size(); //remember where we started
        gc.get()->output << "class " << ast->name << " {" << std::endl << "public: " << std::endl;
        currentFunGroup = funStack.size();

        //then push a variable that will be how we message ourselves
        std::string name("this");
        VariableInfo *vi = new VariableInfo(name, ast->name, ContainerType::Scalar, ScopeType::Struct);
        ++(currentScopeCount.back());
        scopeStack.push_back(vi);

        for (std::vector<ASTNode*>::iterator iter = ast->children.begin(), end = ast->children.end(); iter != end; ++iter) {
            switch ((*iter)->type()) {
                case (NodeType::Function) :
                    funStack.push_back(dynamic_cast<PrototypeAST*>((*iter)->children[0]));
                break;
                case (NodeType::VarDecl) :
                    varDecl = dynamic_cast<VarDeclExprAST*>(*iter);

                    varDecl->vi->scopeType = ScopeType::Struct;

                    ++(currentScopeCount.back());
                    scopeStack.push_back(varDecl->vi);
                break;
                default:
                    throw CompilerException("Unknown feature insde of class", (*iter)->filepos);
                    break;
            }
        }

        for (std::vector<ASTNode*>::iterator iter = ast->children.begin(), end = ast->children.end(); iter != end; ++iter) {
            switch ((*iter)->type()) {
                case (NodeType::Function) :
                    funast = dynamic_cast<FunctionAST*>(*iter);
                    gc_temp = visit(funast, stage);
                    break;
                case (NodeType::VarDecl) :
                    varDecl = dynamic_cast<VarDeclExprAST*>(*iter);

                    finder = actors.find(varDecl->vi->type.declType);

                    if (finder != actors.end()) {
                        if (varDecl->vi->type.containerType == ContainerType::Scalar) {
                            gc.get()->output << "actorId_t " << varDecl->vi->name << ";" << std::endl;
                        }
                        else if (varDecl->vi->type.containerType == ContainerType::Array) {
                            gc.get()->output << "actorId_t " << varDecl->vi->name << "[";

                            boost::shared_ptr<GeneratedCode> gc_temp = visit (varDecl->vi->size);
                            if (gc_temp.get()->decls.str() != "") {
                                gc.get()->output << gc_temp.get()->decls.str();
                            }
                            if (gc_temp.get()->inits.str() != "") {
                                gc.get()->output << gc_temp.get()->inits.str();
                            }
                            if (gc_temp.get()->output.str() != "") {
                                gc.get()->output << gc_temp.get()->output.str();
                            }

                            gc.get()->output << "];" << std::endl;
                        }

                    }
                    else {
                        if (varDecl->vi->type.containerType == ContainerType::Scalar) {
                            gc.get()->output << lookupAssocType(varDecl->vi->type) << " " << varDecl->vi->name << ";" << std::endl;
                        }
                        else if (varDecl->vi->type.containerType == ContainerType::Array) {
                            gc.get()->output << lookupAssocType(varDecl->vi->type) << " " << varDecl->vi->name << "[";
                            boost::shared_ptr<GeneratedCode> gc_temp = visit (varDecl->vi->size);

                            if (gc_temp.get()->decls.str() != "") {
                                gc.get()->output << gc_temp.get()->decls.str();
                            }
                            if (gc_temp.get()->inits.str() != "") {
                                gc.get()->output << gc_temp.get()->inits.str();
                            }
                            if (gc_temp.get()->output.str() != "") {
                                gc.get()->output << gc_temp.get()->output.str();
                            }

                            gc.get()->output << "];" << std::endl;
                        }

                    }
                    break;
                default:
                    throw CompilerException("Unknown feature insde of class", (*iter)->filepos);
                    break;
            }
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->output << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }
        }

        gc.get()->output << ast->name << "() { }" << std::endl;

        //TODO: I'm not sure if I need a traditional copy constructor, or this pointer style
        //gc.get()->output << ast->name << "(const " << ast->name << "& p) {" << std::endl;
        gc.get()->output << ast->name << "(" << ast->name << "* p__) {" << std::endl;

        for (std::vector<ASTNode*>::iterator iter = ast->children.begin(), end = ast->children.end(); iter != end; ++iter) {
            VarDeclExprAST *vdeast = dynamic_cast<VarDeclExprAST*>(*iter);
            if (vdeast != NULL) {
                if (vdeast->vi->type.requiresCopyDelete()) {
                    gc.get()->output << "if (" << vdeast->vi->name
                        << " != NULL) { delete " << vdeast->vi->name << "; };" << std::endl;
                    gc.get()->output << vdeast->vi->name << " = new "
                        << lookupAssocType(vdeast->vi->type) << "(p__->" << vdeast->vi->name << ");" << std::endl;
                }
                else {
                    gc.get()->output << vdeast->vi->name << " = " << "p__->" << vdeast->vi->name << ";" << std::endl;
                }
            }
        }
        /*
        for (std::map<std::string, VariableInfo*>::iterator iter = ast->vars.begin(), end = ast->vars.end(); iter != end; ++iter) {
            if ((iter->second)->needsCopyDelete) {
                gc.get()->output << "if (" << (iter->second)->name << " != NULL) { delete " << (iter->second)->name << "; };" << std::endl;
                gc.get()->output << (iter->second)->name << " = new " << lookupAssocType((iter->second)->type) << "(p__->" << (iter->second)->name << ");" << std::endl;
            }
            else {
                gc.get()->output << (iter->second)->name << " = " << "p__->" << (iter->second)->name << ";" << std::endl;
            }
        }
        */
        gc.get()->output << "}" << std::endl;
        gc.get()->output << "};" << std::endl;

        int unwindAmount = currentScopeCount.back();
        for (int i = 0; i < unwindAmount; ++i) {
            scopeStack.pop_back();
        }
        currentScopeCount.pop_back();

        //Get us back to seeing only the functions we could see before we entered the actor
        while (funStack.size() != currentFunGroup) {
            funStack.pop_back();
        }
    }
    /*
    if (stage == DeclStage::FORWARD) {
        this->classes[ast->name] = ast;
        gc.get()->output << "struct " << ast->name << ";" << std::endl;
    }
    else if (stage == DeclStage::IMPL) {
        currentFunGroup = funStack.size();
        for (std::vector<FunctionAST*>::iterator iter = ast->funs.begin(), end = ast->funs.end(); iter != end; ++iter) {
            funStack.push_back((*iter)->proto);
        }

        currentScopeCount.push_back(0);
        scopeContainerId = currentScopeCount.size(); //remember where we started
        gc.get()->output << "struct " << ast->name << "{" << std::endl;
        for (std::map<std::string, VariableInfo*>::iterator iter = ast->vars.begin(), end = ast->vars.end(); iter != end; ++iter) {
            ++(currentScopeCount.back());
            scopeStack.push_back(iter->second);
            std::map<std::string, ActorAST*>::iterator finder = actors.find(iter->second->type.declType);

            if (finder != actors.end()) {
                if (iter->second->type.typeType == TypeType::Scalar) {
                    gc.get()->output << "actorId_t " << iter->first << ";" << std::endl;
                }
                else if (iter->second->type.typeType == TypeType::Array) {
                    gc.get()->output << "actorId_t " << iter->first << "[";
                    boost::shared_ptr<GeneratedCode> gc_temp = visit (iter->second->size);
                    if (gc_temp.get()->decls.str() != "") {
                        gc.get()->output << gc_temp.get()->decls.str();
                    }
                    if (gc_temp.get()->inits.str() != "") {
                        gc.get()->output << gc_temp.get()->inits.str();
                    }
                    if (gc_temp.get()->output.str() != "") {
                        gc.get()->output << gc_temp.get()->output.str();
                    }

                    gc.get()->output << "];" << std::endl;
                }
            }
            else {
                if (iter->second->type.typeType == TypeType::Scalar) {
                    gc.get()->output << lookupAssocType(iter->second->type) << " " << iter->first << ";" << std::endl;
                }
                else if (iter->second->type.typeType == TypeType::Array) {
                    gc.get()->output << lookupAssocType(iter->second->type) << " " << iter->first << "[";
                    boost::shared_ptr<GeneratedCode> gc_temp = visit (iter->second->size);
                    if (gc_temp.get()->decls.str() != "") {
                        gc.get()->output << gc_temp.get()->decls.str();
                    }
                    if (gc_temp.get()->inits.str() != "") {
                        gc.get()->output << gc_temp.get()->inits.str();
                    }
                    if (gc_temp.get()->output.str() != "") {
                        gc.get()->output << gc_temp.get()->output.str();
                    }

                    gc.get()->output << "];" << std::endl;
                }
            }
        }
        for (std::vector<FunctionAST*>::iterator iter = ast->funs.begin(), end = ast->funs.end(); iter != end; ++iter) {
            boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter, stage);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->output << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }

        }

        gc.get()->output << ast->name << "() { }" << std::endl;

        //TODO: I'm not sure if I need a traditional copy constructor, or this pointer style
        //gc.get()->output << ast->name << "(const " << ast->name << "& p) {" << std::endl;
        gc.get()->output << ast->name << "(" << ast->name << "* p__) {" << std::endl;
        for (std::map<std::string, VariableInfo*>::iterator iter = ast->vars.begin(), end = ast->vars.end(); iter != end; ++iter) {
            if ((iter->second)->needsCopyDelete) {
                gc.get()->output << "if (" << (iter->second)->name << " != NULL) { delete " << (iter->second)->name << "; };" << std::endl;
                gc.get()->output << (iter->second)->name << " = new " << lookupAssocType((iter->second)->type) << "(p__->" << (iter->second)->name << ");" << std::endl;
            }
            else {
                gc.get()->output << (iter->second)->name << " = " << "p__->" << (iter->second)->name << ";" << std::endl;
            }
        }
        gc.get()->output << "}" << std::endl;
        gc.get()->output << "};" << std::endl;
        int unwindAmount = currentScopeCount.back();
        for (int i = 0; i < unwindAmount; ++i) {
            scopeStack.pop_back();
        }
        currentScopeCount.pop_back();

        //Get us back to seeing only the functions we could see before we entered the actor
        while (funStack.size() != currentFunGroup) {
            funStack.pop_back();
        }
    }
*/
    return gc;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ActorAST *ast, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode), gc_temp(new GeneratedCode);

    ActionAST *actast;
    FunctionAST *funast;
    VarDeclExprAST *varDecl;

    //DeclStage::Stage stage;
    if (stage == DeclStage::DECL) {
        currentScopeCount.push_back(0);
        scopeContainerId = currentScopeCount.size(); //remember where we started
        gc.get()->output << "class " << ast->name << " : public Actor {" << std::endl << "public: " << std::endl;
        currentFunGroup = funStack.size();

        //then push a variable that will be how we message ourselves
        std::string name("this");
        VariableInfo *vi = new VariableInfo(name, ast->name, ContainerType::Scalar, ScopeType::Actor);
        ++(currentScopeCount.back());
        scopeStack.push_back(vi);

        actors[ast->name] = ast;
    }
    for (std::vector<ASTNode*>::iterator iter = ast->children.begin(), end = ast->children.end(); iter != end; ++iter) {
        switch ((*iter)->type()) {
            case (NodeType::Function) :
                if (stage == DeclStage::DECL) {
                    funStack.push_back(dynamic_cast<PrototypeAST*>((*iter)->children[0]));
                }
                funast = dynamic_cast<FunctionAST*>(*iter);
                gc_temp = visit(funast, stage);
                break;
            case (NodeType::VarDecl) :
                varDecl = dynamic_cast<VarDeclExprAST*>(*iter);
                if (stage == DeclStage::DECL) {
                    varDecl->vi->scopeType = ScopeType::Actor;

                    ++(currentScopeCount.back());
                    scopeStack.push_back(varDecl->vi);
                    std::map<std::string, ActorAST*>::iterator finder = actors.find(varDecl->vi->type.declType);

                    if (finder != actors.end()) {
                        if (varDecl->vi->type.containerType == ContainerType::Scalar) {
                            gc.get()->output << "actorId_t " << varDecl->vi->name << ";" << std::endl;
                        }
                        else if (varDecl->vi->type.containerType == ContainerType::Array) {
                            gc.get()->output << "actorId_t " << varDecl->vi->name << "[";

                            boost::shared_ptr<GeneratedCode> gc_temp = visit (varDecl->vi->size);
                            if (gc_temp.get()->decls.str() != "") {
                                gc.get()->output << gc_temp.get()->decls.str();
                            }
                            if (gc_temp.get()->inits.str() != "") {
                                gc.get()->output << gc_temp.get()->inits.str();
                            }
                            if (gc_temp.get()->output.str() != "") {
                                gc.get()->output << gc_temp.get()->output.str();
                            }

                            gc.get()->output << "];" << std::endl;
                        }

                    }
                    else {
                        if (varDecl->vi->type.containerType == ContainerType::Scalar) {
                            gc.get()->output << lookupAssocType(varDecl->vi->type) << " " << varDecl->vi->name << ";" << std::endl;
                        }
                        else if (varDecl->vi->type.containerType == ContainerType::Array) {
                            gc.get()->output << lookupAssocType(varDecl->vi->type) << " " << varDecl->vi->name << "[";
                            boost::shared_ptr<GeneratedCode> gc_temp = visit (varDecl->vi->size);

                            if (gc_temp.get()->decls.str() != "") {
                                gc.get()->output << gc_temp.get()->decls.str();
                            }
                            if (gc_temp.get()->inits.str() != "") {
                                gc.get()->output << gc_temp.get()->inits.str();
                            }
                            if (gc_temp.get()->output.str() != "") {
                                gc.get()->output << gc_temp.get()->output.str();
                            }

                            gc.get()->output << "];" << std::endl;
                        }

                    }

                }
                break;
        }
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }

    }
    if (stage == DeclStage::DECL) {
        gc.get()->output << "};" << std::endl;
    }

    for (std::vector<ASTNode*>::iterator iter = ast->children.begin(), end = ast->children.end(); iter != end; ++iter) {
        switch ((*iter)->type()) {
            case (NodeType::Action) :
                actast = dynamic_cast<ActionAST*>(*iter);
                gc_temp = visit(actast, ast->name, stage);

                if (gc_temp.get()->decls.str() != "") {
                    gc.get()->output << gc_temp.get()->decls.str();
                }
                if (gc_temp.get()->inits.str() != "") {
                    gc.get()->output << gc_temp.get()->inits.str();
                }
                if (gc_temp.get()->output.str() != "") {
                    gc.get()->output << gc_temp.get()->output.str();
                }
                break;
        }
    }

    if (stage == DeclStage::IMPL) {
        int unwindAmount = currentScopeCount.back();
        for (int i = 0; i < unwindAmount; ++i) {
            scopeStack.pop_back();
        }
        currentScopeCount.pop_back();

        //Get us back to seeing only the functions we could see before we entered the actor
        while (funStack.size() != currentFunGroup) {
            funStack.pop_back();
        }
    }

    /*
    if (stage == DeclStage::FORWARD) {
        this->actors[ast->name] = ast;
        gc.get()->output << "class " << ast->name << ";" << std::endl;

    }
    else if (stage == DeclStage::DECL) {
        currentScopeCount.push_back(0);
        scopeContainerId = currentScopeCount.size(); //remember where we started

        gc.get()->output << "class " << ast->name << " : public Actor {" << std::endl << "public: " << std::endl;

        //then push a variable that will be how we message ourselves
        std::string name("this");
        VariableInfo *vi = new VariableInfo(name, ast->name, TypeType::Scalar, ScopeType::Actor);
        ++(currentScopeCount.back());
        scopeStack.push_back(vi);

        for (std::map<std::string, VariableInfo*>::iterator iter = ast->vars.begin(), end = ast->vars.end(); iter != end; ++iter) {
            ++(currentScopeCount.back());
            scopeStack.push_back(iter->second);
            std::map<std::string, ActorAST*>::iterator finder = actors.find(iter->second->type.declType);

            if (finder != actors.end()) {
                if (iter->second->type.typeType == TypeType::Scalar) {
                    gc.get()->output << "actorId_t " << iter->first << ";" << std::endl;
                }
                else if (iter->second->type.typeType == TypeType::Array) {
                    gc.get()->output << "actorId_t " << iter->first << "[";

                    boost::shared_ptr<GeneratedCode> gc_temp = visit (iter->second->size);
                    if (gc_temp.get()->decls.str() != "") {
                        gc.get()->output << gc_temp.get()->decls.str();
                    }
                    if (gc_temp.get()->inits.str() != "") {
                        gc.get()->output << gc_temp.get()->inits.str();
                    }
                    if (gc_temp.get()->output.str() != "") {
                        gc.get()->output << gc_temp.get()->output.str();
                    }

                    gc.get()->output << "];" << std::endl;
                }
            }
            else {
                if (iter->second->type.typeType == TypeType::Scalar) {
                    gc.get()->output << lookupAssocType(iter->second->type) << " " << iter->first << ";" << std::endl;
                }
                else if (iter->second->type.typeType == TypeType::Array) {
                    gc.get()->output << lookupAssocType(iter->second->type) << " " << iter->first << "[";
                    boost::shared_ptr<GeneratedCode> gc_temp = visit (iter->second->size);

                    if (gc_temp.get()->decls.str() != "") {
                        gc.get()->output << gc_temp.get()->decls.str();
                    }
                    if (gc_temp.get()->inits.str() != "") {
                        gc.get()->output << gc_temp.get()->inits.str();
                    }
                    if (gc_temp.get()->output.str() != "") {
                        gc.get()->output << gc_temp.get()->output.str();
                    }

                    gc.get()->output << "];" << std::endl;
                }
            }
        }
        for (std::vector<FunctionAST*>::iterator iter = ast->funs.begin(), end = ast->funs.end(); iter != end; ++iter) {
            boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter, stage);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->output << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }

        }
        gc.get()->output << "};" << std::endl;
        for (std::vector<ActionAST*>::iterator iter = ast->actions.begin(), end = ast->actions.end(); iter != end; ++iter) {
            boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter, ast->name, stage);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->output << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }

        }
    }
    else if (stage == DeclStage::IMPL) {
        currentFunGroup = funStack.size();
        for (std::vector<FunctionAST*>::iterator iter = ast->funs.begin(), end = ast->funs.end(); iter != end; ++iter) {
            funStack.push_back((*iter)->proto);
        }

        for (std::vector<FunctionAST*>::iterator iter = ast->funs.begin(), end = ast->funs.end(); iter != end; ++iter) {
            boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter, stage);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->output << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }
        }
        for (std::vector<ActionAST*>::iterator iter = ast->actions.begin(), end = ast->actions.end(); iter != end; ++iter) {
            boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter, ast->name, stage);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->output << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }

        }

        int unwindAmount = currentScopeCount.back();
        for (int i = 0; i < unwindAmount; ++i) {
            scopeStack.pop_back();
        }
        currentScopeCount.pop_back();

        //Get us back to seeing only the functions we could see before we entered the actor
        while (funStack.size() != currentFunGroup) {
            funStack.pop_back();
        }
    }
*/

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(PrototypeAST *ast, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    bool isExtern = checkIfExtern(ast->name);

    gc.get()->output << lookupAssocType(ast->returnType) << " " << ast->name << "(";
    for (int i = 0, j = ast->children.size(); i != j; ++i) {
        VarDeclExprAST *varDecl = dynamic_cast<VarDeclExprAST*>(ast->children[i]);
        if (stage == DeclStage::IMPL) {
            ++(currentScopeCount.back());
            scopeStack.push_back(varDecl->vi);
        }
        if (i != 0) {
            gc.get()->output << ", ";
        }

        gc.get()->output << lookupAssocType(varDecl->vi->type) << " " << varDecl->vi->name;
    }
    if (!isExtern) {
        if (ast->children.size() > 0) {
            gc.get()->output << ", ";
        }
        gc.get()->output << "Actor *actor__";
    }
    gc.get()->output << ")";

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(FunctionAST *ast, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    PrototypeAST *proto = dynamic_cast<PrototypeAST*>(ast->children[0]);

    if (stage == DeclStage::FORWARD) {
        if (ast->children[1]->children.size() == 0) {
            gc.get()->output << "extern ";
            //this->externFns.push_back(ast->proto->name);
        }
        boost::shared_ptr<GeneratedCode> gc_temp = visit (proto, stage);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->decls << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }

        gc.get()->output << ";" << std::endl;
    }
    else if (stage == DeclStage::IMPL) {
        if (ast->children[1]->children.size() == 0) {
            boost::shared_ptr<GeneratedCode> gc_temp(new GeneratedCode);
            return gc_temp;
        }
        else {
            scopeContainerId = currentScopeCount.size(); //remember where we started
            currentScopeCount.push_back(0);

            boost::shared_ptr<GeneratedCode> gc_temp = visit (proto, stage);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->decls << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->decls << gc_temp.get()->output.str();
            }

            setupDontCare(proto->returnType);

            currentContId = 0;

            gc.get()->decls << "{" << std::endl;

            gc.get()->output << "TypeUnion tmpTU__;" << std::endl;
            gc.get()->output << "unsigned int timeLeft__ = actor__->parentThread->timeSliceEndTime;" << std::endl;

            gc.get()->output << "actor__->parentThread->timeSliceEndTime = timeLeft__;" << std::endl;
            gc.get()->output << "int contId__ = 0;" << std::endl;
            gc.get()->output << "if (actor__->isResuming) {" << std::endl;
            gc.get()->output << "  contId__ = actor__->heapStack.back().Int32; actor__->heapStack.pop_back();" << std::endl;
            gc.get()->output << "  if (actor__->heapStack.size() == 0) {" << std::endl;
            gc.get()->output << "    actor__->isResuming = false;" << std::endl;
            gc.get()->output << "  }" << std::endl;
            gc.get()->output << "}" << std::endl;
            gc.get()->output << "actor__->parentThread->timeSliceEndTime = timeLeft__;" << std::endl;

            gc.get()->output << "switch(contId__) {" <<std::endl;
            gc.get()->output << "case(" << currentContId << "):" << std::endl;

            ++currentContId;

            for (std::vector<ASTNode*>::iterator iter = ast->children[1]->children.begin(),
                    end = ast->children[1]->children.end(); iter != end; ++iter) {
                boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter);
                if (gc_temp.get()->decls.str() != "") {
                    gc.get()->decls << gc_temp.get()->decls.str();
                }
                if (gc_temp.get()->inits.str() != "") {
                    gc.get()->output << gc_temp.get()->inits.str();
                }
                if (gc_temp.get()->output.str() != "") {
                    gc.get()->output << gc_temp.get()->output.str();
                }
                gc.get()->output << ";" << std::endl;

            }

            gc.get()->output << "  }" << std::endl;

            int unwindAmount = currentScopeCount.back();
            for (int i = 0; i < unwindAmount; ++i) {
                VariableInfo *vi = scopeStack.back();
                if (vi->type.containerType == ContainerType::Array) {
                    gc.get()->output << "if (" << vi->name << " != NULL) {" << std::endl;
                    if (isCopyDelete(vi->type)) {
                        gc.get()->output << "  for (int i__=0; i__ < " << vi->name << "->size(); ++i__) { if (" << vi->name << "[i__] != NULL) { delete " << vi->name << "[i__];} }" << std::endl;
                    }
                    gc.get()->output << "  " << vi->name << "->clear();" << std::endl;
                    gc.get()->output << "  delete(" << vi->name << ");" << std::endl;
                    gc.get()->output << "}" << std::endl;
                }
                else if ((vi->type.requiresCopyDelete())&&(!checkIfActor(vi->type.declType))) {
                    gc.get()->output << "if (" << vi->name << " != NULL) {" << std::endl;
                    gc.get()->output << "  delete(" << vi->name << ");" << std::endl;
                    gc.get()->output << "}" << std::endl;
                }
                scopeStack.pop_back();
            }
            currentScopeCount.pop_back();

            gc.get()->output << "}" << std::endl;

        }
    }
    return gc;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ActionAST *ast, std::string actorName, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);
    inAction = true;

    PrototypeAST *proto = dynamic_cast<PrototypeAST*>(ast->children[0]);

    if (stage == DeclStage::DECL) {
        gc.get()->output << "void " << actorName << "__" << proto->name << "_action(Actor *a__);" << std::endl;
    }
    else if (stage == DeclStage::IMPL) {
        gc.get()->decls << "void " << actorName << "__" << proto->name << "_action(Actor *a__) ";

        scopeContainerId = currentScopeCount.size(); //remember where we started
        currentScopeCount.push_back(0);

        currentContId = 0;
        //std::string retVal = "void";
        //setupDontCare(retVal);
        this->dontCareReturnVal = "";

        gc.get()->decls << "{" << std::endl;

        gc.get()->decls << actorName << " *actor__ = static_cast<" << actorName << "*>(a__);" << std::endl;
        gc.get()->decls << "TypeUnion tmpTU__;" << std::endl;
        gc.get()->decls << "unsigned int timeLeft__ = actor__->parentThread->timeSliceEndTime;" << std::endl;

        gc.get()->output << "int contId__ = 0;" << std::endl;
        gc.get()->output << "if (actor__->isResuming) {" << std::endl;
        gc.get()->output << "  contId__ = actor__->heapStack.back().Int32; actor__->heapStack.pop_back();" << std::endl;
        gc.get()->output << "  if (actor__->heapStack.size() == 0) {" << std::endl;
        gc.get()->output << "    actor__->isResuming = false;" << std::endl;
        gc.get()->output << "  }" << std::endl;
        gc.get()->output << "}" << std::endl;
        gc.get()->output << "else {" << std::endl;

        /*
        for (std::map<std::string, VariableInfo*>::reverse_iterator iter = ast->vars.rbegin(), end = ast->vars.rend(); iter != end; ++iter) {
            if (iter->second->scopeType == ScopeType::Prototype) {
                gc.get()->decls << lookupAssocType(iter->second->type) << " " << iter->second->name << ";" << std::endl;
                gc.get()->output << lookupPopForVar(iter->second) << ";" << std::endl;

                ++(currentScopeCount.back());
                scopeStack.push_back(iter->second);
            }
        }
        */
        for (std::vector<ASTNode*>::reverse_iterator iter = ast->children[0]->children.rbegin(), end = ast->children[0]->children.rend(); iter != end; ++iter) {
            VarDeclExprAST *varDecl = dynamic_cast<VarDeclExprAST*>(*iter);
            gc.get()->decls << lookupAssocType(varDecl->vi->type) << " " << varDecl->vi->name << ";" << std::endl;
            gc.get()->output << lookupPopForVar(varDecl->vi) << ";" << std::endl;

            ++(currentScopeCount.back());
            scopeStack.push_back(varDecl->vi);
        }
        gc.get()->output << "}" << std::endl;

        gc.get()->output << "switch(contId__) {" <<std::endl;
        gc.get()->output << "case(" << currentContId << "):" << std::endl;
        ++currentContId;

        for (std::vector<ASTNode*>::iterator iter = ast->children[1]->children.begin(),
                end = ast->children[1]->children.end(); iter != end; ++iter) {
            boost::shared_ptr<GeneratedCode> gc_temp = visit (*iter);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }

            gc.get()->output << ";" << std::endl;
        }

        gc.get()->output << "}" << std::endl;
        gc.get()->output << "--timeLeft__;" << std::endl;
        gc.get()->output << "actor__->parentThread->timeSliceEndTime = timeLeft__;" << std::endl;
        gc.get()->output << "actor__->actorState = ActorState::WAITING_FOR_ACTION;" << std::endl;

        int unwindAmount = currentScopeCount.back();
        for (int i = 0; i < unwindAmount; ++i) {
            VariableInfo *vi = scopeStack.back();
            if ((vi->type.requiresCopyDelete())&&(!checkIfActor(vi->type.declType))) {
                gc.get()->output << "if (" << vi->name << " != NULL) {" << std::endl;
                gc.get()->output << "  delete(" << vi->name << ");" << std::endl;
                gc.get()->output << "}" << std::endl;
            }
            scopeStack.pop_back();
        }
        currentScopeCount.pop_back();

        gc.get()->output << "}" << std::endl;

    }
    inAction = false;
    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(AppAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode), gc_temp(new GeneratedCode);

    ActionAST *actast;
    FunctionAST *funast;
    ActorAST *actorast;
    ClassAST *classast;

    ActionAST *mainAction = NULL;

    gc.get()->decls << "//automatically generated by Minnow->C++ codegen (0.1)" << std::endl;
    gc.get()->decls << "#include \"aquarium.hpp\"" << std::endl;

    gc.get()->decls << "inline int convertToInt(std::string s){std::istringstream i(s);int x;i >> x;return x;}" << std::endl;
    gc.get()->decls << "inline void puti(int i){std::cout << i << std::endl; }" << std::endl;
    gc.get()->decls << "inline void putstring(std::string s){std::cout << s << std::endl; }" << std::endl;

    DeclStage::Stage stage;
    for (int i = 0; i < 3; ++i) {
        switch(i) {
            case (0) : stage = DeclStage::FORWARD; break;
            case (1) : stage = DeclStage::DECL; break;
            case (2) : stage = DeclStage::IMPL; break;
        }
        for (std::vector<ASTNode*>::iterator iter = ast->children.begin(), end = ast->children.end(); iter != end; ++iter) {
            //std::cout << "CHILD: " << (*iter)->nodeType << " at " << (*iter)->filepos.lineNumber << " " << (*iter)->filepos.colStart << std::endl;
            switch ((*iter)->type()) {
                case (NodeType::Action) :
                    actast = dynamic_cast<ActionAST*>(*iter);
                    if (stage == DeclStage::FORWARD) {
                        if (dynamic_cast<PrototypeAST*>(actast->children[0])->name != "main") {
                            throw CompilerException("Only the 'main' action is allowed at the top level");
                        }
                        else {
                            mainAction = actast;
                        }
                    }
                    gc_temp = visit(actast, "Actor", stage);
                    break;
                case (NodeType::Function) :
                    if (stage == DeclStage::FORWARD) {
                        funStack.push_back(dynamic_cast<PrototypeAST*>((*iter)->children[0]));
                    }
                    funast = dynamic_cast<FunctionAST*>(*iter);
                    gc_temp = visit(funast, stage);
                    break;
                case (NodeType::Actor) :
                    actorast = dynamic_cast<ActorAST*>(*iter);
                    gc_temp = visit(actorast, stage);
                    break;
                case (NodeType::Class) :
                    classast = dynamic_cast<ClassAST*>(*iter);
                    gc_temp = visit(classast, stage);
                    break;
            }
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->output << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }

        }
    }
    /*
    for (std::vector<StructAST*>::iterator iter = ast->structs.begin(), end = ast->structs.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, DeclStage::FORWARD);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<ActorAST*>::iterator iter = ast->actors.begin(), end = ast->actors.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, DeclStage::FORWARD);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<FunctionAST*>::iterator iter = ast->functions.begin(), end = ast->functions.end(); iter != end; ++iter) {
        funStack.push_back((*iter)->proto);

        gc_temp = visit(*iter, DeclStage::FORWARD);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }

        //std::cout << "Pushing proto: " << (*iter)->proto->name << std::endl;
    }

    for (std::vector<ActionAST*>::iterator iter = ast->actions.begin(), end = ast->actions.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, "Actor", DeclStage::FORWARD);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<StructAST*>::iterator iter = ast->structs.begin(), end = ast->structs.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, DeclStage::DECL);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<ActorAST*>::iterator iter = ast->actors.begin(), end = ast->actors.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, DeclStage::DECL);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<FunctionAST*>::iterator iter = ast->functions.begin(), end = ast->functions.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, DeclStage::DECL);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<ActionAST*>::iterator iter = ast->actions.begin(), end = ast->actions.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, "Actor", DeclStage::DECL);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<StructAST*>::iterator iter = ast->structs.begin(), end = ast->structs.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, DeclStage::IMPL);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<ActorAST*>::iterator iter = ast->actors.begin(), end = ast->actors.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, DeclStage::IMPL);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<FunctionAST*>::iterator iter = ast->functions.begin(), end = ast->functions.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, DeclStage::IMPL);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }

    for (std::vector<ActionAST*>::iterator iter = ast->actions.begin(), end = ast->actions.end(); iter != end; ++iter) {
        gc_temp = visit(*iter, "Actor", DeclStage::IMPL);
        if (gc_temp.get()->decls.str() != "") {
            gc.get()->output << gc_temp.get()->decls.str();
        }
        if (gc_temp.get()->inits.str() != "") {
            gc.get()->output << gc_temp.get()->inits.str();
        }
        if (gc_temp.get()->output.str() != "") {
            gc.get()->output << gc_temp.get()->output.str();
        }
    }
*/

    gc.get()->output << "int main(int argc, char *argv[]){" << std::endl;

    if (mainAction == NULL) {
        throw CompilerException("Can not find 'main' action");
    }
    else {
        PrototypeAST* mainProto = dynamic_cast<PrototypeAST*>(mainAction->children[0]);

        if (mainProto->children.size() > 0) {
            gc.get()->output << "VM_Main(argc, argv, Actor__main_action, true);" << std::endl;
        }
        else {
            gc.get()->output << "VM_Main(argc, argv, Actor__main_action, false);" << std::endl;
        }
    }


    gc.get()->output << "return(0);" << std::endl;
    gc.get()->output << "};" << std::endl;
    return gc;
}

std::string CodegenCPPOutput::translate(ASTNode *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    gc = visit(ast);

    std::ostringstream output;
    if (gc.get()->decls.str() != "") {
        output << gc.get()->decls.str();
    }
    if (gc.get()->inits.str() != "") {
        output << gc.get()->inits.str();
    }
    if (gc.get()->output.str() != "") {
        output << gc.get()->output.str();
    }

    return output.str();
}
