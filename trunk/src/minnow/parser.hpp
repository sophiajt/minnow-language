#ifndef MINNOW_PARSER_HPP
#define MINNOW_PARSER_HPP

#include <iostream>
#include <map>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include "lexer.hpp"

class TypeType {
public:
    enum Type { Scalar, Array };
};

class ScopeType {
    public:
        enum Type { CodeBlock, Struct, Actor, Prototype };
};

class ExpressionType {
public:
    enum Type { Number, Variable, ArrayIndexed, Binary, Quote, Call, End, VarDecl, ArrayDecl, If, While, Pointcut, Lambda };
};

class ExpressionAST {
public:
    virtual ~ExpressionAST() {}
    FilePos pos;
    virtual ExpressionType::Type type() = 0;
    virtual std::string resultType() = 0;
};

class VariableInfo {
public:
    std::string name;
    std::string declType;
    TypeType::Type typeType;
    ExpressionAST *size;
    ScopeType::Type scopeType;
    
    VariableInfo(std::string &vname, std::string &decl, TypeType::Type type, ScopeType::Type scope) : 
        name(vname), declType(decl), typeType(type), scopeType(scope) { }
    VariableInfo(std::string &vname, std::string &decl, TypeType::Type type, ExpressionAST *sizeexpr, ScopeType::Type scope) : 
        name(vname), declType(decl), typeType(type), size(sizeexpr), scopeType(scope) { }
};

class CodeHolder {
public:
    std::map <std::string, VariableInfo*> vars;
};

class NumberExprAST : public ExpressionAST {
public:
    double val;

    NumberExprAST(double nVal) : val(nVal) {}

    virtual ExpressionType::Type type() { return ExpressionType::Number; }
    virtual std::string resultType() { return "num"; };
};

class QuoteExprAST : public ExpressionAST {
public:
    std::string val;

    QuoteExprAST(std::string &nVal) : val(nVal) {}

    virtual ExpressionType::Type type() { return ExpressionType::Quote; }
    virtual std::string resultType() { return "string"; };
};

class VariableExprAST : public ExpressionAST {
public:
    std::string name, varType;
    explicit VariableExprAST(const std::string vName) : name(vName) {}

    virtual ExpressionType::Type type() { return ExpressionType::Variable; }
    virtual std::string resultType() { return varType; };
};

class VarDeclExprAST : public ExpressionAST {
public:
    std::string name, declType;
    bool isSpawn;
    bool isAlloc;
    explicit VarDeclExprAST(const std::string vName, const std::string vType, bool spawn, bool alloc) : name(vName), declType(vType), isSpawn(spawn), isAlloc(alloc) {}

    virtual ExpressionType::Type type() { return ExpressionType::VarDecl; }
    virtual std::string resultType() { return declType; };
};

class ArrayIndexedExprAST : public ExpressionAST {
public:
    std::string name;
    ExpressionAST *index;
    explicit ArrayIndexedExprAST(const std::string vName, ExpressionAST *vIndex) :
        name(vName), index(vIndex) {}

    virtual ExpressionType::Type type() { return ExpressionType::ArrayIndexed; }
    virtual std::string resultType() { return ""; };  //FIXME
};

class ArrayDeclExprAST : public ExpressionAST {
public:
    std::string name, declType;
    ExpressionAST *size;
    bool isSpawn;
    bool isAlloc;
    explicit ArrayDeclExprAST(const std::string vName, const std::string vType, ExpressionAST *vSize, bool spawn, bool alloc) :
        name(vName), declType(vType), size(vSize), isSpawn(spawn), isAlloc(alloc) {}

    virtual ExpressionType::Type type() { return ExpressionType::ArrayDecl; }
    virtual std::string resultType() { return ""; };     //FIXME
};

//FIXME: Check for possible leaks
class EndExprAST : public ExpressionAST {
public:
    explicit EndExprAST() {}

    virtual ExpressionType::Type type() { return ExpressionType::End; }
    virtual std::string resultType() { return ""; };
};

class IfExprAST : public ExpressionAST {
public:
    explicit IfExprAST() {}
    ExpressionAST *Cond;
    std::vector<ExpressionAST*> Then;
    std::vector<ExpressionAST*> Else;

    virtual ExpressionType::Type type() { return ExpressionType::If; }
    virtual std::string resultType() { return "bool"; };
};

class WhileExprAST : public ExpressionAST {
public:
    explicit WhileExprAST() {}
    ExpressionAST *Cond;
    std::vector<ExpressionAST*> Loop;

    virtual ExpressionType::Type type() { return ExpressionType::While; }
    virtual std::string resultType() { return "bool"; };
};

class BinaryExprAST : public ExpressionAST {
public:
    std::string op, binaryType;
    ExpressionAST *LHS, *RHS;

    explicit BinaryExprAST(const std::string bOp, ExpressionAST *lhs, ExpressionAST *rhs)
     : op(bOp), LHS(lhs), RHS(rhs) {}

    virtual ExpressionType::Type type() { return ExpressionType::Binary; }
    virtual std::string resultType() { return binaryType; };
};

class CallExprAST : public ExpressionAST {
public:
    std::string name;
    std::vector<ExpressionAST*> args;

    CallExprAST(const std::string &fName, std::vector<ExpressionAST*> &fArgs)
     : name(fName), args(fArgs) {}

    virtual ExpressionType::Type type() { return ExpressionType::Call; }

    virtual std::string resultType() { return ""; }; //FIXME
};

class LambdaExprAST : public ExpressionAST{
public:
    std::vector<ExpressionAST*> body;

    virtual ExpressionType::Type type() { return ExpressionType::Lambda; }

    virtual std::string resultType() { return ""; }; //FIXME
};

class PointcutExprAST : public ExpressionAST {
public:
    std::string pattern;
    bool throwExOnFalse;
    LambdaExprAST *closure;
    PointcutExprAST () : throwExOnFalse(false) { closure = new LambdaExprAST();}

    virtual ExpressionType::Type type() { return ExpressionType::Pointcut; }

    virtual std::string resultType() { return ""; }; //FIXME
};

class PrototypeAST {
public:
    std::string name;
    std::string type;
    std::vector<std::string> argNames;
    std::vector<std::string> argTypes;
};

class FunctionAST : public CodeHolder {
public:
    PrototypeAST *proto;
    std::vector<ExpressionAST*> body;
    FilePos pos;
};

class ActionAST : public CodeHolder {
public:
    PrototypeAST *proto;
    std::vector<ExpressionAST*> body;
    FilePos pos;
};

class StructAST : public CodeHolder {
public:
    std::string name;
    std::vector<FunctionAST*> funs;
    //std::vector<ExpressionAST*> vars;
};

class ActorAST : public CodeHolder {
public:
    std::string name;
    //std::vector<ExpressionAST*> vars;
    std::vector<FunctionAST*> funs;
    std::vector<ActionAST*> actions;
    bool isIsolated;
    ActorAST(bool isolated) : isIsolated(isolated) {}
};

class AppAST {
public:
    std::vector<FunctionAST*> functions;
    std::vector<StructAST*> structs;
    std::vector<ActorAST*> actors;
    std::vector<ActionAST*> actions;
};

ExpressionAST *parsePrimary(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end);
ExpressionAST *parseExpression(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end);
FunctionAST *parseFunction(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end);
AppAST *parseApp(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end);

#endif //MINNOW_PARSER_HPP
