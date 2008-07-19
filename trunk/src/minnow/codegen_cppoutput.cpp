#include "codegen_cppoutput.hpp"

#include <boost/shared_ptr.hpp>
#include <string>

//catch all that will dispatch out to others
boost::shared_ptr<TypeInfo> CodegenCPPOutput::resolveType(ExpressionAST *ast) {
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
        case (ExpressionType::Number) :
            neast = dynamic_cast<NumberExprAST*>(ast);
            if (neast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            ti.get()->declType = "int";
            ti.get()->typeType = TypeType::Scalar;
            break;
        case (ExpressionType::Boolean) :
            boeast = dynamic_cast<BooleanExprAST*>(ast);
            if (boeast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            ti.get()->declType = "bool";
            ti.get()->typeType = TypeType::Scalar;
            break;
        case (ExpressionType::Variable) :
            veast = dynamic_cast<VariableExprAST*>(ast);
            if (veast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            vi = findVarInScope(veast->name);
            if (vi != NULL) {
                ti.get()->declType = vi->type.declType;
                ti.get()->typeType = vi->type.typeType;
            }
            else {
                std::ostringstream msg;
                msg << "Can not find variable '" << veast->name << "'";
                throw CompilerException(msg.str(), ast->pos);
            }
            break;
        case (ExpressionType::ArrayIndexed) :
            aieast = dynamic_cast<ArrayIndexedExprAST*>(ast);
            if (aieast == NULL) {
                printf("FIXME: Array indexed compiler exception\n");
            }
            vi = findVarInScope(aieast->name);
            if (vi != NULL) {
                ti.get()->declType = vi->type.declType;
                ti.get()->typeType = vi->type.typeType;
            }
            else {
                std::ostringstream msg;
                msg << "Can not find variable '" << aieast->name << "'";
                throw CompilerException(msg.str(), ast->pos);
            }
            break;
        case (ExpressionType::Binary) :
            beast = dynamic_cast<BinaryExprAST*>(ast);
            if (beast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            ti = resolveType(beast->LHS);
            break;
        case (ExpressionType::Quote) :
            qeast = dynamic_cast<QuoteExprAST*>(ast);
            if (qeast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            ti.get()->declType = "quote";
            ti.get()->typeType = TypeType::Scalar;
            break;
        case (ExpressionType::Call) :
            ceast = dynamic_cast<CallExprAST*>(ast);
            if (ceast == NULL) {
                printf("FIXME: Call compiler exception\n");
            }
            typeInfo = lookupReturnTypeInfo(ceast);
            ti.get()->declType = typeInfo.declType;
            ti.get()->typeType = typeInfo.typeType;
            //gc = visit(ceast);
            break;
        default:
            throw CompilerException("Can't resolve type during lookup", ast->pos);
    }

    return ti;
}

//catch all that will dispatch out to others
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ExpressionAST *ast) {
    boost::shared_ptr<GeneratedCode> gc;
    
    NumberExprAST *neast;
    BooleanExprAST *boeast;
    QuoteExprAST *qeast;
    VariableExprAST *veast;
    VarDeclExprAST *vdeast;
    ArrayIndexedExprAST *aieast;
    ArrayDeclExprAST *adeast;
    EndExprAST *eeast;
    IfExprAST *ieast;
    WhileExprAST *weast;
    BinaryExprAST *beast;
    CallExprAST *ceast;

    if (ast == NULL) {
        //NOP
        gc = boost::shared_ptr<GeneratedCode>(new GeneratedCode);
        return gc;
    }
    //Number, Variable, ArrayIndexed, Binary, Quote, Call, DefFun, End, VarDecl, ArrayDecl, If, While
    switch (ast->type()) {
        case (ExpressionType::Number) :
            neast = dynamic_cast<NumberExprAST*>(ast);
            if (neast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            gc = visit(neast);
            break;
        case (ExpressionType::Boolean) :
            boeast = dynamic_cast<BooleanExprAST*>(ast);
            if (boeast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            gc = visit(boeast);
            break;
        case (ExpressionType::Variable) :
            veast = dynamic_cast<VariableExprAST*>(ast);
            if (veast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            gc = visit(veast);
            break;
        case (ExpressionType::ArrayIndexed) :
            aieast = dynamic_cast<ArrayIndexedExprAST*>(ast);
            if (aieast == NULL) {
                printf("FIXME: Array indexed compiler exception\n");
            }
            gc = visit(aieast);
            break;
        case (ExpressionType::Binary) :
            beast = dynamic_cast<BinaryExprAST*>(ast);
            if (beast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            gc = visit(beast);
            break;
        case (ExpressionType::Quote) :
            qeast = dynamic_cast<QuoteExprAST*>(ast);
            if (qeast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            gc = visit(qeast);
            break;
        case (ExpressionType::Call) :
            ceast = dynamic_cast<CallExprAST*>(ast);
            if (ceast == NULL) {
                printf("FIXME: Call compiler exception\n");
            }
            gc = visit(ceast);
            break;
        case (ExpressionType::End) :
            eeast = dynamic_cast<EndExprAST*>(ast);
            if (eeast == NULL) {
                printf("FIXME: End compiler exception\n");
            }
            gc = visit(eeast);
            break;
        case (ExpressionType::VarDecl) :
            vdeast = dynamic_cast<VarDeclExprAST*>(ast);
            if (vdeast == NULL) {
                printf("FIXME: VarDecl compiler exception\n");
            }
            gc = visit(vdeast);
            break;
        case (ExpressionType::ArrayDecl) :
            adeast = dynamic_cast<ArrayDeclExprAST*>(ast);
            if (adeast == NULL) {
                printf("FIXME: ArrayDecl compiler exception\n");
            }
            gc = visit(adeast);
            break;
        case (ExpressionType::If) :
            ieast = dynamic_cast<IfExprAST*>(ast);
            if (ieast == NULL) {
                printf("FIXME: If compiler exception\n");
            }
            
            gc = visit(ieast);
            break;
        case (ExpressionType::While) :
            weast = dynamic_cast<WhileExprAST*>(ast);
            if (weast == NULL) {
                printf("FIXME: While compiler exception\n");
            }
            gc = visit(weast);
            break;
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

    if ((ast->name == "return") || (ast->name == "done")) {
        if (inAction) {
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
        throw CompilerException(oss.str(), ast->pos);
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
    
    VariableInfo *vi = new VariableInfo(ast->name, ast->declType, TypeType::Scalar, ScopeType::CodeBlock);
    ++(currentScopeCount.back());
    scopeStack.push_back(vi);
    
    std::map<std::string, ActorAST*>::iterator finder = actors.find(ast->declType);

    if (finder != actors.end()) {
        if (ast->isSpawn) {
            std::string actorTemp = nextTemp();
            gc.get()->decls << "actorId_t " << ast->name << ";" << std::endl;
            gc.get()->inits << ast->declType << " *" << actorTemp << ";" << std::endl;
            gc.get()->output << "{ " << actorTemp << " = new " << ast->declType << "();" << std::endl;
            if (finder->second->isIsolated) {
                gc.get()->output << "  actor__->parentThread->ScheduleNewIsolatedActor(" << actorTemp << ");" << std::endl;
            }
            else {
                gc.get()->output << "  actor__->parentThread->ScheduleNewActor(" << actorTemp << ");" << std::endl;
            }
            gc.get()->output << "  " << ast->name << " = " << actorTemp << "->actorId; }" << std::endl;
        }
        else {
            gc.get()->decls << "actorId_t " << ast->name << ";" << std::endl;
        }
    }
    else {
        gc.get()->decls << lookupAssocType(vi->type) << " " << ast->name << ";" << std::endl;
        if (ast->isAlloc) {
            gc.get()->output << ast->name << " = new " << ast->declType << "();" << std::endl;
        }
        gc.get()->output << ast->name;
    }
    
    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ArrayIndexedExprAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);
    
    VariableInfo *vi = findVarInScope(ast->name);
    if (vi == NULL) {
        std::ostringstream oss;
        oss << "Unknown variable '" << ast->name << "'";
        throw CompilerException(oss.str(), ast->pos);
    }
    else if (vi->type.typeType != TypeType::Array) {
        std::ostringstream oss;
        oss << "Variable '" << ast->name << "' is not of an array type";
        throw CompilerException(oss.str(), ast->pos);
    }
    if (vi->scopeType == ScopeType::Actor) {
        gc.get()->output << "(*(actor__->" << ast->name << "))";
    }
    else {
        gc.get()->output << "(*" << ast->name << ")";
    }
    gc.get()->output << "[";
    boost::shared_ptr<GeneratedCode> gc_temp = visit (ast->index);
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
        allocType = allocType.erase(allocType.size()-1); //trim off the trailing '*'
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

    gc_cond = visit (ast->Cond);
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
    
    for (std::vector<ExpressionAST*>::iterator iter = ast->Then.begin(), end = ast->Then.end(); iter != end; ++iter) {

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
    if (ast->Else.size() > 0) {
        gc.get()->output << "else {" << std::endl;
        for (std::vector<ExpressionAST*>::iterator iter = ast->Else.begin(), end = ast->Else.end(); iter != end; ++iter) {
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

    gc_cond = visit (ast->Cond);
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
    for (std::vector<ExpressionAST*>::iterator iter = ast->Loop.begin(), end = ast->Loop.end(); iter != end; ++iter) {
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
    gc_cond = visit (ast->Cond);
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
    
    if (ast->op == "::") {
        CallExprAST *ceast = dynamic_cast<CallExprAST*>(ast->RHS);
        VariableExprAST *veast = dynamic_cast<VariableExprAST*>(ast->LHS);
        ArrayIndexedExprAST *aieast = dynamic_cast<ArrayIndexedExprAST*>(ast->LHS);
        
        VariableInfo *vi;

        if (ceast == NULL) {
            //std::cout << "Can't build message, is type: " << ast->RHS->type() << std::endl;
            //exit(1);
            std::ostringstream msg;
            msg << "Can't build message, right hand side is type: " << ast->RHS->type();
            throw CompilerException(msg.str(), ast->pos);
        }
        
        if ((veast == NULL) && (aieast == NULL)) {
            std::ostringstream msg;
            msg << "Can't build message, left hand side is type: " << ast->LHS->type();
            throw CompilerException(msg.str(), ast->pos);
        }
        else if (veast != NULL) {
            vi = findVarInScope(veast->name);
        }
        else { //if (aieast != NULL) {
            vi = findVarInScope(aieast->name);
        }   
        
        //int argSize = ceast->args.size();
        std::string msgName = nextTemp();
        std::string actorIfLocal = nextTemp();

        gc.get()->decls << "Actor *" << actorIfLocal << ";" << std::endl;
        
        gc.get()->output << actorIfLocal << " = actor__->parentThread->ActorIfLocal(";
        gc_temp = visit (ast->LHS);
        
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

        gc_temp = visit (ast->LHS);
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
        gc.get()->output << "  " << msgName << ".numArgs = " << ceast->args.size() << ";" << std::endl;
        gc.get()->output << "  " << msgName << ".messageType = MessageType::ACTION_MESSAGE;" << std::endl;
        gc.get()->output << "  " << msgName << ".task = &" << vi->type.declType << "__" << ceast->name << "_action;" << std::endl;
        
        for (unsigned int i = 0; i < ceast->args.size(); ++i) {
            boost::shared_ptr<TypeInfo> ti = resolveType(ceast->args[i]);
            
            boost::shared_ptr<GeneratedCode> gc_temp = visit (ceast->args[i]);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                //gc.get()->output << gc_temp.get()->output.str();
                gc.get()->output << "  " << lookupPushForTypeAndBlock(ti, gc_temp.get()->output.str());
                gc.get()->output << "  " << msgName << ".arg[" << i << "] = tmpTU__;" << std::endl;
            }

            //gc.get()->output << lookupPushForVar(ceast->args[i]);
            //gc.get()->output << ";" << std::endl;
        }

        gc.get()->output << "actor__->parentThread->SendMessage(" << msgName << ");" << std::endl;
        gc.get()->output << "}" << std::endl;
        gc.get()->output << "else {" << std::endl;

        for (unsigned int i = 0; i < ceast->args.size(); ++i) {
            boost::shared_ptr<TypeInfo> ti = resolveType(ceast->args[i]);
            //gc.get()->output << "tmpTU__.UInt32 = ";

            boost::shared_ptr<GeneratedCode> gc_temp = visit (ceast->args[i]);
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
        VariableExprAST *veast = dynamic_cast<VariableExprAST*>(ast->RHS);
        CallExprAST *ceast = dynamic_cast<CallExprAST*>(ast->RHS);
        if ((veast == NULL) && (ceast == NULL)) {
            std::cout << "Can't call to method, is type: " << ast->RHS->type() << std::endl;
            exit(1);
        }
        else {
            gc_temp = visit (ast->LHS);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }
            gc.get()->output << "->";
            gc_temp = visit (ast->RHS);
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
    else {
        gc_temp = visit (ast->LHS);
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
        gc_temp = visit (ast->RHS);
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
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode), gc_temp;
    std::vector<boost::shared_ptr<GeneratedCode> > gc_args;
    bool isExtern = checkIfExtern(ast->name);

    for (int i = 0, j = ast->args.size(); i != j; ++i) {
        gc_args.push_back(visit (ast->args[i]));
    }
    
    for (int i = 0, j = ast->args.size(); i != j; ++i) {
        //if (i != 0)
        //    gc.get()->inits << ", ";
        if (gc_args[i].get()->decls.str() != "") {
            gc.get()->decls << gc_args[i].get()->decls.str();
        }
        if (gc_args[i].get()->inits.str() != "") {
            gc.get()->inits << gc_args[i].get()->inits.str();
        }            
    }
  
    if (!isExtern) {
        std::string tmpName = nextTemp();
        std::string retType = lookupReturnType(ast);
        gc.get()->decls << retType << " " << tmpName << ";" << std::endl;
        
        gc.get()->inits << "case(" << currentContId << "):" << std::endl;
        ++currentContId;
    
        gc.get()->inits << outputResumeBlock();
    
        gc.get()->inits << "actor__->parentThread->timeSliceEndTime = timeLeft__;" << std::endl;
        gc.get()->inits << tmpName << " = " << ast->name << "(";
        for (int i = 0, j = ast->args.size(); i != j; ++i) {
            if (i != 0)
                gc.get()->inits << ", ";
            
            if (gc_args[i].get()->output.str() != "") {
                gc.get()->inits << gc_args[i].get()->output.str();
            }
        }
        if (!isExtern) {
            if (ast->args.size() > 0) {
                gc.get()->inits << ", ";
            }
            gc.get()->inits << "actor__";
        }
        gc.get()->inits << ");" << std::endl;
        gc.get()->inits << "timeLeft__ = actor__->parentThread->timeSliceEndTime;" << std::endl;
        gc.get()->inits << outputPauseBlock(false);
        
        gc.get()->output << tmpName;
    }
    else {
        gc.get()->output << ast->name << "(";
        for (int i = 0, j = ast->args.size(); i != j; ++i) {
            if (i != 0)
                gc.get()->output << ", ";
            /*
            gc_temp = visit (ast->args[i]);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            */
            if (gc_args[i].get()->output.str() != "") {
                gc.get()->output << gc_args[i].get()->output.str();
            }
        }
        gc.get()->output << ")";
    }

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(StructAST *ast, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    if (stage == DeclStage::FORWARD) {
        this->structs[ast->name] = ast;
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
                        gc.get()->decls << gc_temp.get()->decls.str();
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
                        gc.get()->decls << gc_temp.get()->decls.str();
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
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->output << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->output << gc_temp.get()->output.str();
            }

        }
        
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

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ActorAST *ast, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

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

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(PrototypeAST *ast, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    bool isExtern = checkIfExtern(ast->name);
    
    gc.get()->output << lookupAssocType(ast->type) << " " << ast->name << "(";
    for (int i = 0, j = ast->args.size(); i != j; ++i) {
        //VariableInfo *vi = new VariableInfo(ast->argNames[i], ast->argTypes[i], TypeType::Scalar, ScopeType::CodeBlock);
        if (stage == DeclStage::IMPL) {
            ++(currentScopeCount.back());
            scopeStack.push_back(ast->args[i]);
        }
        if (i != 0) {
            gc.get()->output << ", ";
        }
        
        gc.get()->output << lookupAssocType(ast->args[i]->type) << " " << ast->args[i]->name;
        /*
        if (stage != DeclStage::IMPL) {
            delete vi;
        }
        */
    }
    if (!isExtern) {
        if (ast->args.size() > 0) {
            gc.get()->output << ", ";
        }
        gc.get()->output << "Actor *actor__";
    }
    gc.get()->output << ")";

    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(FunctionAST *ast, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);

    if (stage == DeclStage::FORWARD) {
        if (ast->body.size() == 0) {
            gc.get()->output << "extern ";
            this->externFns.push_back(ast->proto->name);
        }
        boost::shared_ptr<GeneratedCode> gc_temp = visit (ast->proto, stage);
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
        if (ast->body.size() == 0) {
            boost::shared_ptr<GeneratedCode> gc_temp(new GeneratedCode);
            return gc_temp;
        }
        else {
            scopeContainerId = currentScopeCount.size(); //remember where we started
            currentScopeCount.push_back(0);
            
            boost::shared_ptr<GeneratedCode> gc_temp = visit (ast->proto, stage);
            if (gc_temp.get()->decls.str() != "") {
                gc.get()->decls << gc_temp.get()->decls.str();
            }
            if (gc_temp.get()->inits.str() != "") {
                gc.get()->decls << gc_temp.get()->inits.str();
            }
            if (gc_temp.get()->output.str() != "") {
                gc.get()->decls << gc_temp.get()->output.str();
            }
            
            setupDontCare(ast->proto->type);

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
            
            for (std::vector<ExpressionAST*>::iterator iter = ast->body.begin(), end = ast->body.end(); iter != end; ++iter) {
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
            gc.get()->output << "}" << std::endl;

            int unwindAmount = currentScopeCount.back();
            for (int i = 0; i < unwindAmount; ++i) {
                scopeStack.pop_back();
            }
            currentScopeCount.pop_back();
        }
    }
    return gc;
}

boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(ActionAST *ast, std::string actorName, DeclStage::Stage stage) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode);
    inAction = true;

    if (stage == DeclStage::DECL) {
        gc.get()->output << "void " << actorName << "__" << ast->proto->name << "_action(Actor *a__);" << std::endl;
    }
    else if (stage == DeclStage::IMPL) {
        gc.get()->decls << "void " << actorName << "__" << ast->proto->name << "_action(Actor *a__) ";
        
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

        for (std::map<std::string, VariableInfo*>::reverse_iterator iter = ast->vars.rbegin(), end = ast->vars.rend(); iter != end; ++iter) {
            if (iter->second->scopeType == ScopeType::Prototype) {
                gc.get()->decls << lookupAssocType(iter->second->type) << " " << iter->second->name << ";" << std::endl;
                gc.get()->output << lookupPopForVar(iter->second) << ";" << std::endl;

                ++(currentScopeCount.back());
                scopeStack.push_back(iter->second);
            }
        }
        gc.get()->output << "}" << std::endl;

        gc.get()->output << "switch(contId__) {" <<std::endl;
        gc.get()->output << "case(" << currentContId << "):" << std::endl;
        ++currentContId;
        
        for (std::vector<ExpressionAST*>::iterator iter = ast->body.begin(), end = ast->body.end(); iter != end; ++iter) {
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
        gc.get()->output << "}" << std::endl;

        int unwindAmount = currentScopeCount.back();
        for (int i = 0; i < unwindAmount; ++i) {
            scopeStack.pop_back();
        }
        currentScopeCount.pop_back();
    }
    inAction = false;
    return gc;
}
boost::shared_ptr<GeneratedCode> CodegenCPPOutput::visit(AppAST *ast) {
    boost::shared_ptr<GeneratedCode> gc(new GeneratedCode), gc_temp(new GeneratedCode);

    gc.get()->decls << "//automatically generated by Minnow->C++ codegen (0.1)" << std::endl;
    gc.get()->decls << "#include \"aquarium.hpp\"" << std::endl;
    gc.get()->decls << "inline int convertToInt(const std::string& s){std::istringstream i(s);int x;i >> x;return x;}" << std::endl;
    gc.get()->decls << "inline void puti(int i){std::cout << i << std::endl; }" << std::endl;
    gc.get()->decls << "inline void putstring(std::string &s){std::cout << s << std::endl; }" << std::endl;
    gc.get()->decls << "extern void exit(int i);" << std::endl;
    gc.get()->decls << "extern int puts(char *s);" << std::endl;

    externFns.push_back("convertToInt");
    externFns.push_back("puti");
    externFns.push_back("exit");
    externFns.push_back("puts");
    externFns.push_back("putstring");
    externFns.push_back("return");
    
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

        funStack.push_back((*iter)->proto);
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

    gc.get()->output << "int main(int argc, char *argv[]){" << std::endl;
    if ((ast->actions.size() != 1)||(ast->actions[0]->proto->name != "main")) {
        throw CompilerException("Can not find 'main' action");
    }
    
    else if (ast->actions[0]->proto->args.size() > 0) {
        gc.get()->output << "VM_Main(argc, argv, Actor__main_action, true);" << std::endl;
    }
    else {
        gc.get()->output << "VM_Main(argc, argv, Actor__main_action, false);" << std::endl;
    }
    
    gc.get()->output << "return(0);" << std::endl;
    gc.get()->output << "};" << std::endl;
    
    return gc;
}

std::string CodegenCPPOutput::translate(AppAST *ast) {
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
