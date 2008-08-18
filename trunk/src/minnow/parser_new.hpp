#ifndef PARSER_NEW_HPP_
#define PARSER_NEW_HPP_

#include "lexer.hpp"

class ScopeType {
public:
    enum Type { CodeBlock, Struct, Actor, Prototype };
};

class ContainerType {
public:
    enum Type { Scalar, Array };
};

class TypeInfo {
public:
    std::string declType;
    ContainerType::Type containerType;
    std::vector<TypeInfo> containedTypes;

    bool requiresCopyDelete() {
    	bool needsCopyDelete = false; //default, now check for exceptions
        if (containerType == ContainerType::Array) {
            needsCopyDelete = true;
        }
        //FIXME: This is _ugly_
        else if ((declType != "void") && (declType != "bool") &&
        		(declType != "int") && (declType != "double") &&
        		(declType != "string")) {
            needsCopyDelete = true;
        }

        return needsCopyDelete;
    }

    TypeInfo(const std::string &decl, ContainerType::Type &type,
    		const std::vector<TypeInfo> &contained) :
    	declType(decl), containerType(type), containedTypes(contained) {

    }

    TypeInfo(const std::string &decl, ContainerType::Type type) :
    	declType(decl), containerType(type) {

    }

    TypeInfo() {
    	declType = "void";
    	containerType = ContainerType::Scalar;
    }
};

class NodeType {
public:
    enum Type { Number, Boolean, Variable, ArrayIndexed, Binary, Quote, Call,
    	End, VarDecl, ArrayDecl, If, While, Pointcut, Advice, Aspect, Lambda,
    	Recv, Msg, Block, Prototype, Function, Action, Class, Actor, Namespace,
    	FileStart, FileEnd, App };
};

class ASTNode {
public:
	FilePos filepos;
	NodeType::Type nodeType;
	TypeInfo resultType;
	std::vector<ASTNode*> children;

	virtual NodeType::Type type() = 0;
};

class VariableInfo {
public:
    std::string name;
    TypeInfo type;
    ASTNode *size;
    ScopeType::Type scopeType;

    VariableInfo(std::string &vname, std::string &decl, ContainerType::Type ty,
    		ScopeType::Type scope) :
        name(vname), type(decl, ty), scopeType(scope)
    { }
    VariableInfo(std::string &vname, std::string &decl, ContainerType::Type ty,
    		ASTNode *sizeast, ScopeType::Type scope) :
        name(vname), type(decl, ty), size(sizeast), scopeType(scope)
	{ }

    VariableInfo() {
    	scopeType = ScopeType::CodeBlock;
    }
};

class BlockAST : public ASTNode{
public:
	//container for other nodes (keeps groups clean)

    virtual NodeType::Type type() { return NodeType::Block; }
};

class NumberExprAST : public ASTNode {
public:
    double val;

    NumberExprAST(double nVal) : val(nVal) {}

    virtual NodeType::Type type() { return NodeType::Number; }
};

class BooleanExprAST : public ASTNode {
public:
    bool val;

    BooleanExprAST(bool bVal) : val(bVal) {}

    virtual NodeType::Type type() { return NodeType::Boolean; }
};

class QuoteExprAST : public ASTNode {
public:
    std::string val;

    QuoteExprAST(std::string &nVal) : val(nVal) {}

    virtual NodeType::Type type() { return NodeType::Quote; }
};

class VariableExprAST : public ASTNode {
public:
    std::string name;
    explicit VariableExprAST(const std::string vName) : name(vName) {}

    virtual NodeType::Type type() { return NodeType::Variable; }
};

class VarDeclExprAST : public ASTNode {
public:
	VariableInfo *vi;
    bool isSpawn;
    bool isAlloc;

    explicit VarDeclExprAST(VariableInfo *varInfo, bool spawn, bool alloc) :
    	vi(varInfo), isSpawn(spawn), isAlloc(alloc)
    {}

    virtual NodeType::Type type() { return NodeType::VarDecl; }
};

class ArrayIndexedExprAST : public ASTNode {
public:
    std::string name;
    explicit ArrayIndexedExprAST(const std::string vName, ASTNode* vIndex) :
        name(vName) {children.push_back(vIndex); }

    virtual NodeType::Type type() { return NodeType::ArrayIndexed; }
};

//FIXME: Check for possible leaks
class EndExprAST : public ASTNode {
public:
    explicit EndExprAST() {}

    virtual NodeType::Type type() { return NodeType::End; }
};

class IfExprAST : public ASTNode {
public:
    explicit IfExprAST() {}
    //three nodes: first is the condition of the if, the second is block for the
    //then, and the third the block for the else.

    virtual NodeType::Type type() { return NodeType::If; }
};

class WhileExprAST : public ASTNode {
public:
    explicit WhileExprAST() {}
    //two nodes: the cond and the loop block

    virtual NodeType::Type type() { return NodeType::While; }
};

class BinaryExprAST : public ASTNode {
public:
    std::string op; //, binaryType;

    explicit BinaryExprAST(const std::string bOp, ASTNode* lhs, ASTNode* rhs)
        : op(bOp) { children.push_back(lhs); children.push_back(rhs); }

    virtual NodeType::Type type() { return NodeType::Binary; }
};

class CallExprAST : public ASTNode {
public:
    std::string name;
    //children are the args to the call

    CallExprAST(const std::string &fName)
        : name(fName) {}

    virtual NodeType::Type type() { return NodeType::Call; }
};

class LambdaExprAST : public ASTNode{
public:
	//children is one child, the block of nodes

    virtual NodeType::Type type() { return NodeType::Lambda; }
};

class PointcutExprAST : public ASTNode {
public:
    std::string pattern;
    bool throwExOnFalse;

    //one child, the lambda that will be used as the closure

    PointcutExprAST () : throwExOnFalse(false) {
    	children.push_back(new LambdaExprAST());
	}
    virtual NodeType::Type type() { return NodeType::Pointcut; }
};

class PrototypeAST : public ASTNode {
public:
    std::string name;
    bool isExtern;
    TypeInfo returnType;
    //children are the variable decls that will be args

    virtual NodeType::Type type() { return NodeType::Prototype; }
};

class DataMsgExprAST : public ASTNode {
public:
    //two children: the pattern to match (a prototype) and
	//the block of asts for this closure.
    //FIXME: use a lambda?

    explicit DataMsgExprAST() {}

    virtual NodeType::Type type() { return NodeType::Msg; }
};

class DataRecvExprAST : public ASTNode {
public:
	//children are the msg matches for the receive

    explicit DataRecvExprAST() {}

    virtual NodeType::Type type() { return NodeType::Recv; }
};

class FunctionAST : public ASTNode {
public:
    //prototype is first child, block is second
    virtual NodeType::Type type() { return NodeType::Function; }
};

class ActionAST : public ASTNode {
public:
	//prototype is first child, block is second
    virtual NodeType::Type type() { return NodeType::Action; }
};

class ClassAST : public ASTNode {
  public:
    std::string name;

    virtual NodeType::Type type() { return NodeType::Class; }
};

class ActorAST : public ASTNode {
  public:
    std::string name;
    bool isIsolated;
    ActorAST(bool isolated) : isIsolated(isolated) {}

    virtual NodeType::Type type() { return NodeType::Actor; }
};

class AppAST : public ASTNode {
    virtual NodeType::Type type() { return NodeType::App; }
};

TypeInfo parse_typesig(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

VarDeclExprAST* parse_variable_decl(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

ASTNode *parse_parens(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

ASTNode *parse_array_index(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

ASTNode *parse_primary(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

ASTNode *parse_binary_op(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end, int currPrec, ASTNode *expr);

ASTNode *parse_expression(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

PrototypeAST* parse_prototype(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

FunctionAST* parse_function(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

ActionAST* parse_action(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

ClassAST* parse_class(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

ActorAST* parse_actor(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);

ASTNode* parse(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end);


#endif /* PARSER_NEW_HPP_ */
