#ifndef CODEGEN_CPPOUTPUT_HPP
#define CODEGEN_CPPOUTPUT_HPP

#include <vector>
#include <string>
#include <sstream>
#include <map>

#include <boost/shared_ptr.hpp>

#include "parser.hpp"

class DeclStage { public: enum Stage {FORWARD, DECL, IMPL}; };

struct GeneratedCode {
    std::ostringstream decls;
    std::ostringstream inits;
    std::ostringstream output;
};

class CodegenCPPOutput {    
    std::map<std::string, ActorAST*> actors;
    std::map<std::string, StructAST*> structs;
    std::vector<std::string> builtins;
    std::vector<VariableInfo*> scopeStack;
    std::vector<PrototypeAST*> funStack;
    std::vector<std::string> externFns;
    std::string dontCareReturnVal;
    long startOfLinePos;
    int scopeContainerId;
    std::vector<int> currentScopeCount; //counts the number of elems local to the current scope
    unsigned int currentFunGroup;
    int currentContId;
    int tempNumber;

    std::string nextTemp() {
        char buffer[30];
        sprintf(buffer, "tmp%i__", tempNumber);
        ++tempNumber;
        return std::string(buffer);
    }
    
    VariableInfo* findVarInScope(const std::string &name) {
        for (std::vector<VariableInfo*>::reverse_iterator iter = scopeStack.rbegin(), end = scopeStack.rend(); iter != end; ++iter) {
            if ((*iter)->name == name) {
                return *iter;
            }
        }
        
        return NULL;
    }

    bool checkIfExtern(const std::string &fn) {
        if (find(externFns.begin(), externFns.end(), fn) != externFns.end()) {
            return true;
        }
        else {
            return false;
        }
    }

    //void setupDontCare(const std::string &typeName) {
    void setupDontCare(const TypeInfo &ti) {
        //FIXME: Add in the position this happened

        if (ti.typeType == TypeType::Array) {
            dontCareReturnVal = "NULL";
        }
        else if (ti.declType == "void") {
            dontCareReturnVal = "";
        }
        else if (ti.declType == "bool") {
            dontCareReturnVal = "false";
        }
        else if (ti.declType == "int") {
            dontCareReturnVal = "0";
        }
        else if (ti.declType == "double") {
            dontCareReturnVal = "0.0";
        }
        else if (ti.declType == "string") {
            dontCareReturnVal = "\"\"";
        }
        else {
            std::ostringstream msg;
            msg << "Unknown typename '" << ti.declType << "'";

            throw CompilerException(msg.str());
        }
    }
    
    std::string lookupAssocType(const TypeInfo &ti) {
        std::map<std::string, ActorAST*>::iterator finder = actors.find(ti.declType);

        if (finder != actors.end()) {
            if (ti.typeType == TypeType::Array) {
                return "std::vector<actorId_t>*";
            }
            else {
                return "actorId_t";
            }
        }
        else {
            std::ostringstream arrayType;
            if (ti.typeType == TypeType::Array) {
                if ((ti.declType == "int") || (ti.declType == "float") || (ti.declType == "bool") || (ti.declType == "double") || (ti.declType == "void")) {
                    arrayType << "std::vector<" << ti.declType << ">*";
                }
                else if (ti.declType == "string") {
                    arrayType << "std::vector<std::string>*";
                }
                else {
                    arrayType << "std::vector<" << ti.declType << "*>*";
                }
                return arrayType.str();
            }
            else {
                if ((ti.declType == "int") || (ti.declType == "float") || (ti.declType == "bool") || (ti.declType == "double") || (ti.declType == "void")) {
                    arrayType << ti.declType;
                }
                else if (ti.declType == "string") {
                    arrayType << "std::string";
                }
                else {
                    arrayType << ti.declType << "*";
                }
                return arrayType.str();
            }
        }
    }

    std::string lookupPushForVar(const VariableInfo *vi) {
        std::ostringstream o;
        if (vi->type.typeType == TypeType::Array) {
            o << "  tmpTU__.VoidPtr = " << vi->name << ";" << std::endl;
        }   
        else if (vi->type.declType == "int") {
            o << "  tmpTU__.UInt32 = " << vi->name << ";" << std::endl;
        }
        else if (vi->type.declType == "double") {
            o << "  tmpTU__.Double = " << vi->name << ";" << std::endl;
        }
        else if (vi->type.declType == "float") {
            o << "  tmpTU__.Float = " << vi->name << ";" << std::endl;
        }
        else if (vi->type.declType == "bool") {
            o << "  tmpTU__.Bool = " << vi->name << ";" << std::endl;
        }
        else {
            o << "  tmpTU__.VoidPtr = " << vi->name << ";" << std::endl;
        }
        o << "  actor__->heapStack.push_back(tmpTU__);" << std::endl;
        return o.str();
    }
    
    std::string lookupPopForVar(const VariableInfo *vi) {
        std::ostringstream o;
        if (vi->type.typeType == TypeType::Array) {
            o << "  " << vi->name << " = (" << lookupAssocType(vi->type) << ")(actor__->heapStack.back().VoidPtr); actor__->heapStack.pop_back();" << std::endl;
        }   
        else if (vi->type.declType == "int") {
            o << "  " << vi->name << " = actor__->heapStack.back().UInt32; actor__->heapStack.pop_back();" << std::endl;
        }
        else if (vi->type.declType == "double") {
            o << "  " << vi->name << " = actor__->heapStack.back().Double; actor__->heapStack.pop_back();" << std::endl;
        }
        else if (vi->type.declType == "float") {
            o << "  " << vi->name << " = actor__->heapStack.back().Float; actor__->heapStack.pop_back();" << std::endl;
        }
        else if (vi->type.declType == "bool") {
            o << "  " << vi->name << " = actor__->heapStack.back().Bool; actor__->heapStack.pop_back();" << std::endl;
        }
        else {
            o << "  " << vi->name << " = (" << lookupAssocType(vi->type) << ")(actor__->heapStack.back().VoidPtr); actor__->heapStack.pop_back();" << std::endl;
        }
        return o.str();
    }

    /*
    std::string lookupInternalType(const VariableInfo *vi) {
        std::map<std::string, ActorAST*>::iterator finder = actors.find(vi->type.declType);

        if (finder != actors.end()) {
            if (vi->type.typeType == TypeType::Array) {
                return "std::vector<actorId_t>*";
            }
            else {
                return "actorId_t";
            }
        }
        else {
            if (vi->type.typeType == TypeType::Array) {
                std::ostringstream arrayType;
                arrayType << "std::vector<" << lookupAssocType(vi->type.declType) << ">*";
                return arrayType.str();
            }
            else {
                return lookupAssocType(vi->type.declType);
            }
        }
    }
    */

    std::string lookupReturnType(const CallExprAST *ast) {
        //std::string returnVal("int");

        for (std::vector<PrototypeAST*>::reverse_iterator iter = funStack.rbegin(), end = funStack.rend(); iter != end; ++iter) {
            //FIXME: This is insufficient for overloaded functions
            if ((*iter)->name == ast->name) {
                //TypeInfo ti((*iter)->type, TypeType::Scalar);
                return lookupAssocType((*iter)->type);
            }
        }

        std::ostringstream msg;
        msg << "Can not find function '" << ast->name << "'";
        std::string outmsg = msg.str();
        throw CompilerException(outmsg, ast->pos);
        return "";
        //return returnVal;
    }
    
    std::string outputResumeBlock() {
        std::ostringstream output;
        
        int resumeCount = 0;
        int scopeStackSize = scopeStack.size() - 1;
        for (unsigned int i = scopeContainerId; i < currentScopeCount.size(); ++i) {
            resumeCount += currentScopeCount[i];
        }
        
        output << "if (actor__->isResuming) {" << std::endl;
        for (int i = 0; i < resumeCount; ++i) {
            VariableInfo *vi = scopeStack[scopeStackSize-i];
            output << lookupPopForVar(vi);
        }
        output << "  if (actor__->heapStack.size() == 0) {" << std::endl;
        output << "    actor__->isResuming = false;" << std::endl;
        output << "  }" << std::endl;
        output << "}" << std::endl;

        return output.str();
    }
    
    std::string outputPauseBlock(bool decrement) {
        std::ostringstream output;

        int resumeCount = 0;
        int scopeStackSize = scopeStack.size() - 1;
        for (unsigned int i = scopeContainerId; i < currentScopeCount.size(); ++i) {
            resumeCount += currentScopeCount[i];
        }
        
        if (decrement) {
            output << "if (timeLeft__ > 0) {" << std::endl;
            output << "  --timeLeft__;" << std::endl;
            output << "}" << std::endl;
            output << "else {" << std::endl;
        }
        else {
            output << "if (timeLeft__ == 0) {" << std::endl;
        }

        output << "  actor__->parentThread->timeSliceEndTime = timeLeft__;" << std::endl;
        
        for (int i = scopeStackSize-resumeCount + 1; i <= scopeStackSize; ++i) {
            VariableInfo *vi = scopeStack[i];
            output << lookupPushForVar(vi);
        }
        output << "  tmpTU__.UInt32 = " << (currentContId-1) << ";" << std::endl;
        output << "  actor__->heapStack.push_back(tmpTU__);" << std::endl;
        output << "  return " << dontCareReturnVal << ";" << std::endl;
        output << "}" << std::endl;

        return output.str();
    }
    
    boost::shared_ptr<GeneratedCode> visit(ExpressionAST *ast);  //catch all that will dispatch out to others
    boost::shared_ptr<GeneratedCode> visit(NumberExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(BooleanExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(QuoteExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(VariableExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(VarDeclExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(ArrayIndexedExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(ArrayDeclExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(EndExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(IfExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(WhileExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(BinaryExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(CallExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(StructAST *ast, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(ActorAST *ast, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(PrototypeAST *ast, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(ActionAST *ast, std::string actorName, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(FunctionAST *ast, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(AppAST *ast);
public:
    std::string translate(AppAST *ast);

    CodegenCPPOutput() {
        builtins.push_back("int");
        tempNumber = 0;
    };
};

#endif
