#include <iostream>

#include "lexer.hpp"
#include "parser_new.hpp"

TypeInfo parse_typesig(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	TypeInfo ti;
    if (iter == end) {
        throw CompilerException("Incomplete type signature", *(--iter));
    }
    ti.declType = (*iter)->data;

    ++iter;

    if ((*iter)->data == "[") {
        ++iter;
        if (iter == end) {
            throw CompilerException("Incomplete type signature", *(--iter));
        }
        if ((*iter)->data != "]") {
            throw CompilerException("Unsupported type signature", *(--iter));
        }
        ++iter;
        ti.containerType = ContainerType::Array;
    }

    if (iter == end) {
        throw CompilerException("Incomplete type signature", *(--iter));
    }

	return ti;
}

VarDeclExprAST* parse_variable_decl(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	bool isSpawn = false;
	bool isAlloc = false;

	VariableInfo *vi = new VariableInfo();
	VarDeclExprAST *returnVal = NULL;

	if (iter == end) {
		throw CompilerException("Expected variable declaration", *(--iter));
	}

	if ((*iter)->data == "var") {
		//nothing
		++iter;
	}
	//FIXME: this is the _old_ style
	else if ((*iter)->data == "spawn") {
		isSpawn = true;
		++iter;
	}
	else if ((*iter)->data == "new") {
		isAlloc = true;
		++iter;
	}

	vi->type = parse_typesig(iter, end);
	if ((*iter)->tokenType != TokenType::Id) {
		throw CompilerException("Invalid variable declaration", *(iter));
	}
	vi->name = (*iter)->data;
	returnVal = new VarDeclExprAST(vi, isSpawn, isAlloc);
	returnVal->filepos = (*iter)->pos;
	++iter;

	return returnVal;
}

PrototypeAST* parse_prototype(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {
	PrototypeAST* returnVal = new PrototypeAST();
	returnVal->filepos = (*iter)->pos;

	//parse the return type
	returnVal->returnType = parse_typesig(iter, end);

    if ((*iter)->data != "(") {
        returnVal->name = (*iter)->data;
        ++iter;
        if (iter == end) {
            throw CompilerException("Expected prototype definition", *(--iter));
        }
    }
    else {
    	returnVal->name = returnVal->returnType.declType;
    	returnVal->returnType.declType = "void";
    }

    if ((*iter)->data != "(") {
    	throw CompilerException("Incomplete function definition", *iter);
    }

    ++iter;
    if (iter == end) {
    	throw CompilerException("Incomplete function definition", *iter);
    }

    while (((*iter)->data != ")") && (iter != end)) {
    	returnVal->children.push_back(parse_variable_decl(iter, end));
    	if ((iter != end) && ((*iter)->data == ",")) {
    		++iter;
    	}
    }
	return returnVal;
}

FunctionAST* parse_function(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	FunctionAST* returnVal = NULL;

    if ((iter != end) && ((*iter)->data == "extern")) {
        ++iter;
        if (iter != end) {
            returnVal = new FunctionAST();
            returnVal->filepos = (*iter)->pos;
            PrototypeAST *proto = parse_prototype(iter, end);
            if (proto == NULL) {
                throw CompilerException("Expecting external function definition", *iter);
            }
            proto->isExtern = true;
            returnVal->children.push_back(proto);
        }
    }
    else if ((iter != end) && ((*iter)->data == "def")) {
        //we may be at a definition
        ++iter;
        if (iter != end) {
            returnVal = new FunctionAST();
            returnVal->filepos = (*iter)->pos;

            PrototypeAST *proto = parse_prototype(iter, end);
            if (proto == NULL) {
                throw CompilerException("Expecting function definition", *iter);
            }
            proto->isExtern = false;
            returnVal->children.push_back(proto);
            /*
            ExpressionAST *ast = parseExpression(iter, end);
            while (iter != end) {
                if (ast != NULL) {
                    if (ast->type() == ExpressionType::End) {
                        ++iter;
                        break;
                    }
                    else {
                        returnVal->body.push_back(ast);
                    }
                }
                ast = parseExpression(iter, end);
            }
            */
        }
    }

	return returnVal;
}

ClassAST* parse_class(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	ClassAST *returnVal = NULL;

    if ((iter != end) && ((*iter)->data == "class")) {
        ++iter;
        if (iter != end) {
            returnVal = new ClassAST();
            returnVal->name = (*iter)->data;

            ++iter;
            while ((iter != end) && ((*iter)->data != "end")) {
                if ((*iter)->data == "def") {
                    FunctionAST *fun = parse_function(iter, end);
                    if (fun != NULL) {
                        returnVal->children.push_back(fun);
                    }
                }
                else {
                	VarDeclExprAST *varDecl = parse_variable_decl(iter, end);
                    if (varDecl != NULL) {
                        //FIXME: Check for duplicate definitions
                    	returnVal->children.push_back(varDecl);
					}
                    else {
                    	throw CompilerException("Unknown element in class", *(--iter));
                    }
                }
            }

            if (iter == end) {
                throw CompilerException("Expected 'end' at end of class", *(--iter));
            }
        }
    }

	return returnVal;
}

ASTNode* parse(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	ASTNode *returnVal, *child;

	returnVal = new AppAST();

	while (iter != end) {
		child = parse_function(iter, end);
		if (child != NULL) {
			returnVal->children.push_back(child);
			continue;
		}
		child = parse_class(iter, end);
		if (child != NULL) {
			returnVal->children.push_back(child);
			continue;
		}
	}

	return returnVal;
}
