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
    std::map<std::string, ClassAST*> classes;
    //std::vector<std::string> builtins;
    std::vector<VariableInfo*> scopeStack;
    std::vector<PrototypeAST*> funStack;
    //std::vector<std::string> externFns;
    //std::vector<PrototypeAST*> externFns;
    std::string dontCareReturnVal;
    long startOfLinePos;
    int scopeContainerId;
    std::vector<int> currentScopeCount; //counts the number of elems local to the current scope
    unsigned int currentFunGroup;
    int currentContId;
    int tempNumber;

    bool inAction;

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

    //FIXME: This won't work with function overloading

    bool checkIfExtern(const std::string &fn) {
        for (std::vector<PrototypeAST*>::reverse_iterator iter = funStack.rbegin(), end = funStack.rend(); iter != end; ++iter) {
            if ((*iter)->name == fn) {
                return (*iter)->isExtern;
            }
        }

        return false;

//         if (find(externFns.begin(), externFns.end(), fn) != externFns.end()) {
//             return true;
//         }
//         else {
//             return false;
//         }
    }

    bool checkIfActor(const std::string &s) {
        std::map<std::string, ActorAST*>::iterator finder = actors.find(s);

        return (finder != actors.end());
    }

    //void setupDontCare(const std::string &typeName) {
    void setupDontCare(const TypeInfo &ti) {
        //FIXME: Add in the position this happened

        if (ti.containerType == ContainerType::Array) {
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

    bool isCopyDelete(const TypeInfo &ti) {
        bool answer = true;
        if (ti.declType == "void") {
            answer = false;
        }
        else if (ti.declType == "bool") {
            answer = false;
        }
        else if (ti.declType == "int") {
            answer = false;
        }
        else if (ti.declType == "double") {
            answer = false;
        }
        else if (ti.declType == "string") {
            answer = false;
        }

        return answer;
    }

    std::string lookupAssocType(const TypeInfo &ti) {
        //std::map<std::string, ActorAST*>::iterator finder = actors.find(ti.declType);

        //if (finder != actors.end()) {
        if (checkIfActor(ti.declType)) {
            if (ti.containerType == ContainerType::Array) {
                return "std::vector<actorId_t>*";
            }
            else {
                return "actorId_t";
            }
        }
        else {
            std::ostringstream arrayType;
            if (ti.containerType == ContainerType::Array) {
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

    //std::string lookupPushForTypeAndBlock(const boost::shared_ptr<TypeInfo> ti, const std::string &block) {
    std::string lookupPushForTypeAndBlock(TypeInfo &ti, const std::string &block) {
        std::ostringstream o;
        //std::map<std::string, ActorAST*>::iterator finder = actors.find(ti.get()->declType);
        //if (finder != actors.end()) {
        if (checkIfActor(ti.declType)) {
            o << "  tmpTU__.UInt32 = " << block << ";" << std::endl;
        }
        else if (ti.containerType == ContainerType::Array) {
            //o << "  tmpTU__.VoidPtr = " << block << ";" << std::endl;
            std::string allocType = lookupAssocType(ti);
            allocType = allocType.erase(allocType.size()-1);


            o << "  tmpTU__.VoidPtr = new " << allocType << "();" << std::endl;
            if (ti.containedTypes[0].requiresCopyDelete()) {
                o << "  for (int i__=0; i__ < " << block << "->size(); ++i__) { (("
                    << lookupAssocType(ti) << ")(tmpTU__.VoidPtr))->push_back(new "
                    << ti.declType << "((*" << block << ")[i__])); }" << std::endl;
            }
            else {
                o << "  for (int i__=0; i__ < " << block << "->size(); ++i__) { (("
                    << lookupAssocType(ti) << ")(tmpTU__.VoidPtr))->push_back((*"
                    << block << ")[i__]); }" << std::endl;
            }
        }
        else if (ti.declType == "int") {
            o << "  tmpTU__.UInt32 = " << block << ";" << std::endl;
        }
        else if (ti.declType == "string") {
            o << "  tmpTU__.VoidPtr = strcpy(new char[" << block << ".size() + 1], " << block << ".c_str());" << std::endl;
        }
        else if (ti.declType == "double") {
            o << "  tmpTU__.Double = " << block << ";" << std::endl;
        }
        else if (ti.declType == "float") {
            o << "  tmpTU__.Float = " << block << ";" << std::endl;
        }
        else if (ti.declType == "bool") {
            o << "  tmpTU__.Bool = " << block << ";" << std::endl;
        }
        else {
            //TRIM
            std::string allocType = lookupAssocType(ti);
            allocType = allocType.erase(allocType.size()-1);
            o << "  tmpTU__.VoidPtr = new " << allocType << "(" << block << ");" << std::endl;
        }
        return o.str();
    }

    std::string lookupPushForVar(const VariableInfo *vi) {
        std::ostringstream o;
        if (vi->type.containerType == ContainerType::Array) {
            o << "  tmpTU__.VoidPtr = " << vi->name << ";" << std::endl;
        }
        else if (vi->type.declType == "int") {
            o << "  tmpTU__.UInt32 = " << vi->name << ";" << std::endl;
        }
        else if (vi->type.declType == "string") {
            o << "  tmpTU__.VoidPtr = strcpy(new char[" << vi->name << ".size() + 1], " << vi->name << ".c_str());" << std::endl;
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
            //std::map<std::string, ActorAST*>::iterator finder = actors.find(vi->type.declType);

            //if (finder != actors.end()) {
            if (checkIfActor(vi->type.declType)) {
                o << "  tmpTU__.UInt32 = " << vi->name << ";" << std::endl;
            }
            else {
                o << "  tmpTU__.VoidPtr = " << vi->name << ";" << std::endl;
            }

        }
        o << "  actor__->heapStack.push_back(tmpTU__);" << std::endl;
        return o.str();
    }

    std::string lookupPopForVar(const VariableInfo *vi) {
        std::ostringstream o;
        if (vi->type.containerType == ContainerType::Array) {
            o << "  " << vi->name << " = (" << lookupAssocType(vi->type) << ")(actor__->heapStack.back().VoidPtr); actor__->heapStack.pop_back();" << std::endl;
        }
        else if (vi->type.declType == "int") {
            o << "  " << vi->name << " = actor__->heapStack.back().UInt32; actor__->heapStack.pop_back();" << std::endl;
        }
        else if (vi->type.declType == "string") {
            o << "  " << vi->name << ".assign((char *)(actor__->heapStack.back().VoidPtr)); actor__->heapStack.pop_back(); delete((char *)(tmpTU__.VoidPtr));" << std::endl;
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
            //std::map<std::string, ActorAST*>::iterator finder = actors.find(vi->type.declType);

            //if (finder != actors.end()) {
            if (checkIfActor(vi->type.declType)) {
                o << "  " << vi->name << " = (" << lookupAssocType(vi->type) << ")(actor__->heapStack.back().UInt32); actor__->heapStack.pop_back();" << std::endl;
            }
            else {
                o << "  " << vi->name << " = (" << lookupAssocType(vi->type) << ")(actor__->heapStack.back().VoidPtr); actor__->heapStack.pop_back();" << std::endl;
            }

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

    TypeInfo lookupReturnTypeInfo(const CallExprAST *ast) {
        //std::string returnVal("int");

        for (std::vector<PrototypeAST*>::reverse_iterator iter = funStack.rbegin(),
                end = funStack.rend(); iter != end; ++iter) {
            //FIXME: This is insufficient for overloaded functions
            if ((*iter)->name == ast->name) {
                //TypeInfo ti((*iter)->type, TypeType::Scalar);
                return (*iter)->returnType;
            }
        }

        /*
        if (find(externFns.begin(), externFns.end(), ast->name) == externFns.end()) {
            std::ostringstream msg;
            msg << "Can not find function '" << ast->name << "'";
            std::string outmsg = msg.str();
            throw CompilerException(outmsg, ast->pos);
        }
        else {
            TypeInfo ti;
            return ti;
        }
        */
        //return "";
        //return returnVal;

        std::ostringstream msg;
        msg << "Can not find function '" << ast->name << "'";
        std::string outmsg = msg.str();
        throw CompilerException(outmsg, ast->filepos);
    }

    std::string lookupReturnType(const CallExprAST *ast, const std::string &container) {
        //std::string returnVal("int");

        if (container == "") {
            for (std::vector<PrototypeAST*>::reverse_iterator iter = funStack.rbegin(), end = funStack.rend(); iter != end; ++iter) {
                //FIXME: This is insufficient for overloaded functions
                if ((*iter)->name == ast->name) {
                    //TypeInfo ti((*iter)->type, TypeType::Scalar);
                    return lookupAssocType((*iter)->returnType);
                }
            }
        }
        //TODO: Re-enable this

        else {
            for (std::vector<ASTNode*>::iterator iter = classes[container]->children.begin(),
                    end = classes[container]->children.end(); iter != end; ++iter) {

                FunctionAST *fast = dynamic_cast<FunctionAST*>(*iter);
                if (fast != NULL) {
                    //FIXME: This is insufficient for overloaded functions
                    PrototypeAST *proto = dynamic_cast<PrototypeAST*>(fast->children[0]);
                    if (proto->name == ast->name) {
                        //TypeInfo ti((*iter)->type, TypeType::Scalar);
                        return lookupAssocType(proto->returnType);
                    }
                }
            }
        }

        std::ostringstream msg;
        msg << "Can not find function '" << ast->name << "'";
        std::string outmsg = msg.str();
        throw CompilerException(outmsg, ast->filepos);

        /*
        if (find(externFns.begin(), externFns.end(), ast->name) == externFns.end()) {
            std::ostringstream msg;
            msg << "Can not find function '" << ast->name << "'";
            std::string outmsg = msg.str();
            throw CompilerException(outmsg, ast->pos);
            return "";
        }
        else {
            return "";
        }
        */
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

    boost::shared_ptr<TypeInfo> resolveType(ASTNode *ast);
    boost::shared_ptr<GeneratedCode> handleCall(CallExprAST *ast, const std::string &container, const std::string &container_name);

    boost::shared_ptr<GeneratedCode> visit(ASTNode *ast);  //catch all that will dispatch out to others
    boost::shared_ptr<GeneratedCode> visit(NumberExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(BooleanExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(QuoteExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(VariableExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(VarDeclExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(ArrayIndexedExprAST *ast);
    //boost::shared_ptr<GeneratedCode> visit(ArrayDeclExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(EndExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(IfExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(WhileExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(BinaryExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(CallExprAST *ast);
    boost::shared_ptr<GeneratedCode> visit(ClassAST *ast, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(ActorAST *ast, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(PrototypeAST *ast, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(ActionAST *ast, std::string actorName, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(FunctionAST *ast, DeclStage::Stage stage);
    boost::shared_ptr<GeneratedCode> visit(AppAST *ast);
public:
    std::string translate(ASTNode *ast);

    CodegenCPPOutput() {
        //builtins.push_back("int");
        tempNumber = 0;
        inAction = false;
    };
};

#endif
