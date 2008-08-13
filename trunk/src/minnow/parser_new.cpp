#include <boost/shared_ptr.hpp>

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

VariableInfo* parse_variable_decl(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	VariableInfo *vi = new VariableInfo();

	vi->type = parse_typesig(iter, end);
	if ((*iter)->tokenType != TokenType::Id) {
		throw CompilerException("Invalid variable declaration", *(iter));
	}
	vi->name = (*iter)->data;
	++iter;

	return vi;
}

PrototypeAST* parse_prototype(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {
	PrototypeAST* returnVal = new PrototypeAST();

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

	return returnVal;
}

FunctionAST* parse_function(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	FunctionAST* returnVal = NULL;

    if ((iter != end) && ((*iter)->data == "extern")) {
        ++iter;
        if (iter != end) {
            returnVal = new FunctionAST();

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
ASTNode* parse(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	ASTNode *returnVal, *child;

	returnVal = new AppAST();

	while (iter != end) {
		child = parse_function(iter, end);
		if (child != NULL) {
			//FunctionAST* fast = dynamic_cast<FunctionAST*>(child);
			returnVal->children.push_back(child);
			continue;
		}
	}

	return returnVal;
}
