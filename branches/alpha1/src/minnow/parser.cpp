#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"

const int EQUATE_PREC = 2;
const int MSG_PREC = 4;
const int COMMA_GROUP_PREC = 5;
const int SCOPE_PREC = 6;
const int COMPARE_PREC = 10;
const int ADD_SUB_PREC = 20;
const int MULT_DIV_PREC = 30;
const int POWER_PREC = 40;

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

        TypeInfo containedType;
        containedType.declType = ti.declType;
        containedType.containerType = ContainerType::Scalar;
        ti.containedTypes.push_back(containedType);
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
	    std::ostringstream msg;
	    msg << "Invalid variable declaration for '" << (*iter)->data << "'";
	    throw CompilerException(msg.str(), *iter);
		//throw CompilerException("Invalid variable declaration", *(iter));
	}
	vi->name = (*iter)->data;

	returnVal = new VarDeclExprAST(vi, isSpawn, isAlloc);
	returnVal->filepos = (*iter)->pos;
	++iter;

	if (iter != end) {
	    if ((*iter)->data == "[") {
	        ++iter;
	        if (iter == end) {
	            throw CompilerException("Incomplete default size", *(--iter));
	        }
	        vi->size = parse_expression(iter, end);
	        /*
	        vi->type.containerType = ContainerType::Array;

	        TypeInfo containedInfo;
	        containedInfo.containerType = ContainerType::Scalar;
	        containedInfo.declType = vi->type.declType;
	        vi->type.containedTypes.push_back(containedInfo);
            */
	        if ((*iter)->data != "]") {
	            throw CompilerException("Incomplete default size", *(--iter));
	        }
	        ++iter;
	    }
	}

	return returnVal;
}

int get_precedence(const std::string &op) {
    int binPrec = -1;
    if (( op == "<") || ( op == ">") || (op == ">=") || (op == "<=") ||
        (op == "==") || (op == "!=") ) {

        binPrec = COMPARE_PREC;
    }
    else if (( op == "+") || ( op == "-")) {
        binPrec = ADD_SUB_PREC;
    }
    else if (( op == "*") || ( op == "/")) {
        binPrec = MULT_DIV_PREC;
    }
    else if ( op == "^") {
        binPrec = POWER_PREC;
    }
    else if ( op == "=") {
        binPrec = EQUATE_PREC;
    }
    else if ( (op == "!") || (op == "::") ) {
        binPrec = MSG_PREC;
    }
    else if ( op == ".") {
        binPrec = SCOPE_PREC;
    }
    else if ( op == ",") {
        binPrec = COMMA_GROUP_PREC;
    }

    return binPrec;
}

ASTNode *parse_parens(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {
	ASTNode *returnVal;
    ++iter;
    if (iter == end) {
        throw CompilerException("Unclosed parentheses", *(--iter));
    }
    else {
        returnVal = parse_expression(iter, end);
        returnVal->filepos = (*iter)->pos;
        if ((*iter)->data != ")") {
            throw CompilerException("Unclosed parentheses", *(--iter));
        }
        else {
            ++iter;
        }
    }

    return returnVal;
}

ASTNode *parse_array_index(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {
    ASTNode *returnVal;
    ++iter;
    if (iter == end) {
        throw CompilerException("Unclosed square brackets", *(--iter));
    }
    else {
        returnVal = parse_expression(iter, end);
        returnVal->filepos = (*iter)->pos;
        if ((*iter)->data != "]") {
            throw CompilerException("Unclosed square brackets", *(--iter));
        }
        else {
            ++iter;
        }
    }

    return returnVal;
}

ASTNode *parse_fun_call(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {
    ASTNode *returnVal = NULL;
    std::string name;
    std::vector<ASTNode*> args;
    std::vector<ASTNode*> unwinder;

    name = (*iter)->data;
    ++iter;
    if ((iter == end) || ((*iter)->data != "(")) {
        --iter;
        throw CompilerException("Internal integrity error relating to function calls", *iter);
    }
    ++iter;
    if (iter == end) {
        throw CompilerException("Expecting ')' for function call", *iter);
    }
    while (((*iter)->data != ")") && (iter != end)) {
        args.push_back(parse_expression(iter, end));
    }

    if (iter != end) {
        //skip the trailing ')'
        ++iter;
    }

    //what we have is probably a tree of args, but we want a flat list, so fix it
    if (args.size() == 1) {
        BinaryExprAST *beast = dynamic_cast<BinaryExprAST*>(args[0]);
        ASTNode *LHS;
        if ((beast != NULL) && (beast->op == ",")) {
            while ((beast != NULL) && (beast->op == ",")) {
                unwinder.push_back(beast->children[1]);
                LHS = beast->children[0];
                beast = dynamic_cast<BinaryExprAST*>(LHS);
            }
            unwinder.push_back(LHS);
        }
        else {
            unwinder.push_back(args[0]);
        }
    }
    args.clear();
    for (std::vector<ASTNode*>::reverse_iterator riter=unwinder.rbegin(),
    		rend=unwinder.rend(); riter != rend; ++riter) {
        args.push_back(*riter);
    }

    returnVal = new CallExprAST(name);
    returnVal->children.swap(args);

    returnVal->filepos = (*iter)->pos;

    return returnVal;
}

ASTNode *parse_if_block(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    IfExprAST *returnVal = new IfExprAST();
    returnVal->filepos = (*iter)->pos;

    ++iter;
    if (iter == end) {
        throw CompilerException("Expected if block", *(--iter));
    }

    returnVal->children.push_back(parse_expression(iter, end));

    BlockAST *thenBlock = new BlockAST();
    while ((iter != end) && ((*iter)->data != "end") && ((*iter)->data != "else")) {
        thenBlock->children.push_back(parse_expression(iter, end));
    }
    returnVal->children.push_back(thenBlock);

    if (iter == end) {
        throw CompilerException("Expected 'end' to end if block", *(--iter));
    }

    BlockAST *elseBlock = new BlockAST();
    if ((*iter)->data == "else") {
        ++iter;
        while ((iter != end) && ((*iter)->data != "end")) {
        	elseBlock->children.push_back(parse_expression(iter, end));
        }
    }
    returnVal->children.push_back(elseBlock);

    if (iter == end) {
        throw CompilerException("Expected 'end' to end if block", *(--iter));
    }
    else {
        ++iter;
    }

    return returnVal;
}

ASTNode *parse_while_block(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

    WhileExprAST *returnVal = new WhileExprAST();
    returnVal->filepos = (*iter)->pos;

    ++iter;
    if (iter == end) {
        throw CompilerException("Expected while block", *(--iter));
    }

    returnVal->children.push_back(parse_expression(iter, end));

    BlockAST *loopBlock = new BlockAST();
    while ((iter != end) && ((*iter)->data != "end")) {
        ASTNode *ast = parse_expression(iter, end);
        loopBlock->children.push_back(ast);
    }
    returnVal->children.push_back(loopBlock);

    if (iter == end) {
        throw CompilerException("Expected 'end' to end while block", *(--iter));
    }
    else {
        ++iter;
    }

    return returnVal;
}


ASTNode *parse_primary(std::vector<Token*>::iterator &iter,
        std::vector<Token*>::iterator &end) {
    ASTNode *returnVal = NULL;
    ASTNode *index = NULL;
    double val;
    bool boolval;

    std::string name, type;

    switch ((*iter)->tokenType) {
        case (TokenType::Num):
            val = atof((*iter)->data.c_str());
            returnVal = new NumberExprAST(val);
            returnVal->filepos = (*iter)->pos;
            ++iter;
            break;
        case (TokenType::String):
            returnVal = new QuoteExprAST((*iter)->data);
            returnVal->filepos = (*iter)->pos;
            ++iter;
            break;
        case (TokenType::Id):
            if ((*iter)->data == "end") {
                returnVal = new EndExprAST();
                returnVal->filepos = (*iter)->pos;
            }
            else if ((*iter)->data == "var") {
                returnVal = parse_variable_decl(iter, end);
            }
            else if ((*iter)->data == "spawn") {
                returnVal = parse_variable_decl(iter, end);
            }
            else if ((*iter)->data == "new") {
                returnVal = parse_variable_decl(iter, end);
            }
            else if ((*iter)->data == "if") {
                returnVal = parse_if_block(iter, end);
            }
            else if ((*iter)->data == "while") {
                returnVal = parse_while_block(iter, end);
            }
            /*
             else if ((*iter)->data == "receive") {
             returnVal = parseDataRecvBlock(iter, end);
             }
             */
            else {
                name = (*iter)->data;
                ++iter;

                if (iter == end) {
                    returnVal = new VariableExprAST(name);
                    returnVal->filepos = (*iter)->pos;
                }
                else {
                    if ((*iter)->data == "(") {
                        --iter;
                        returnVal = parse_fun_call(iter, end);
                    }
                    else if ((*iter)->data == "[") {
                        index = parse_array_index(iter, end);
                        returnVal = new ArrayIndexedExprAST(name, index);
                        returnVal->filepos = (*iter)->pos;
                    }
                    else {
                        returnVal = new VariableExprAST(name);
                        returnVal->filepos = (*iter)->pos;
                    }
                }
            }
            break;
        case (TokenType::EOL):
            ++iter;
            break;
        case (TokenType::Symbol):
            if ((*iter)->data == "(") {
                returnVal = parse_parens(iter, end);
            }
            /*
             else if ((*iter)->data == "@") {
             returnVal = parsePointcut(iter, end);
             }
             */
            else {
                //std::cout << "Element: " << (*iter)->data << std::endl;
                throw CompilerException("Unknown symbol", *iter);
            }
            break;
        case (TokenType::Bool):
            if ((*iter)->data == "true") {
                boolval = true;
            }
            else if ((*iter)->data == "false") {
                boolval = false;
            }
            else {
                throw CompilerException("Unknown boolean value", *iter);
            }
            returnVal = new BooleanExprAST(boolval);
            returnVal->filepos = (*iter)->pos;
            ++iter;

            break;
        default:
            std::ostringstream msg;
            msg << "Unknown token type:" << (*iter)->tokenType;
            throw CompilerException(msg.str(), *iter);
    }

    return returnVal;
}

ASTNode *parse_binary_op(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end, int currPrec, ASTNode *expr) {

    //This uses Chris Lattner's cool precedence parsing trick
    //For more information see his kaleidoscope tutorial on the llvm website
    int binPrec;
    //let's see if what we're sitting on is a math symbol
    while (iter != end) {
        std::string op = (*iter)->data;
        binPrec = get_precedence(op);
        if (binPrec < currPrec) {
            //the next symbol in line is lower precedence than we need
            return expr;
        }
        ++iter;
        if (iter != end) {
            //check the precedence of the next hop
            ASTNode *rhs = parse_primary(iter, end);
            if (rhs == NULL) {
                throw CompilerException("Incomplete math equation", *iter);
            }

            //++iter;

            if (iter != end) {
                std::string op = (*iter)->data;
                int nextPrec = get_precedence(op);

                //std::cout << "Outside dive: " << (*iter)->data << " " << binPrec << " " << nextPrec << std::endl;
                if (binPrec < nextPrec) {
                    //--iter;
                    //std::cout << "Diving in: " << (*iter)->data << std::endl;
                    rhs = parse_binary_op(iter, end, binPrec+1, rhs);
                }
            }
            expr = new BinaryExprAST(op, expr, rhs);
            expr->filepos = (*iter)->pos;

        }
        else {
            throw CompilerException("Incomplete math equation", *(--iter));
        }
    }
    return expr;
}

ASTNode *parse_expression(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
	ASTNode *lhs = parse_primary(iter, end);
    if (lhs != NULL) {
        return parse_binary_op(iter, end, 0, lhs);
    }
    else {
        return NULL;
    }

    while (((*iter)->tokenType == TokenType::EOL) && (iter != end))  {
        //skip EOL chars
        ++iter;
    }
}

PrototypeAST* parse_prototype(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {
	PrototypeAST* returnVal = new PrototypeAST();
	returnVal->filepos = (*iter)->pos;

	//parse the return type
	returnVal->returnType = parse_typesig(iter, end);

    if ((*iter)->data == "(") {
        returnVal->name = returnVal->returnType.declType;
        returnVal->returnType.declType = "void";
    }
    else {
        returnVal->name = (*iter)->data;
        ++iter;
        if (iter == end) {
            throw CompilerException("Expected prototype definition", *(--iter));
        }
    }

    if ((*iter)->data == "(") {
        ++iter;
    }
    else {
    	throw CompilerException("Incomplete function definition", *iter);
    }

    if (iter == end) {
    	throw CompilerException("Incomplete function definition", *iter);
    }

    while (((*iter)->data != ")") && (iter != end)) {
    	returnVal->children.push_back(parse_variable_decl(iter, end));
    	if ((iter != end) && ((*iter)->data == ",")) {
    		++iter;
    	}
    }

    if (iter != end) {
        //skip the trailing ')'
        ++iter;
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
            returnVal->children.push_back(new BlockAST()); //add the code empty code block for consistency
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

            BlockAST *bodyBlock = new BlockAST();
            ASTNode *ast = parse_expression(iter, end);
            while (iter != end) {
                if (ast != NULL) {
                    if (ast->type() == NodeType::End) {
                        ++iter;
                        break;
                    }
                    else {
                        bodyBlock->children.push_back(ast);
                    }
                }
                ast = parse_expression(iter, end);
            }
            returnVal->children.push_back(bodyBlock);
        }
    }

	return returnVal;
}

ActionAST* parse_action(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	ActionAST* returnVal = NULL;

    if ((iter != end) && ((*iter)->data == "action")) {
        //we may be at a definition
        ++iter;
        if (iter != end) {
            returnVal = new ActionAST();
            returnVal->filepos = (*iter)->pos;

            PrototypeAST *proto = parse_prototype(iter, end);
            if (proto == NULL) {
                throw CompilerException("Expecting function definition", *iter);
            }
            proto->isExtern = false;
            returnVal->children.push_back(proto);

            BlockAST *bodyBlock = new BlockAST();
            ASTNode *ast = parse_expression(iter, end);
            while (iter != end) {
                if (ast != NULL) {
                    if (ast->type() == NodeType::End) {
                        ++iter;
                        break;
                    }
                    else {
                        bodyBlock->children.push_back(ast);
                    }
                }
                ast = parse_expression(iter, end);
            }
            returnVal->children.push_back(bodyBlock);
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
            returnVal->filepos = (*iter)->pos;

            ++iter;
            while ((iter != end) && ((*iter)->data != "end")) {
                if ((*iter)->data == "def") {
                    FunctionAST *fun = parse_function(iter, end);
                    if (fun != NULL) {
                        returnVal->children.push_back(fun);
                    }
                }
                else if ((*iter)->tokenType == TokenType::Id) {
                	VarDeclExprAST *varDecl = parse_variable_decl(iter, end);
                    if (varDecl != NULL) {
                        //FIXME: Check for duplicate definitions
                    	returnVal->children.push_back(varDecl);
					}
                    else {
                    	throw CompilerException("Unknown element in class", *(--iter));
                    }
                }
                else {
                    //FIXME: Not sure if this is correct
                    if (iter != end) {
                        ++iter;
                    }
                }
            }

            if (iter == end) {
                throw CompilerException("Expected 'end' at end of class", *(--iter));
            }
            else {
                ++iter;
            }
        }
    }

	return returnVal;
}

ActorAST* parse_actor(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	ActorAST *returnVal = NULL;
    bool isIsolated = false;

    if ((iter != end) && ((*iter)->data == "isolated")) {
        isIsolated = true;
        ++iter;
    }

    if ((iter != end) && ((*iter)->data == "actor")) {
        ++iter;
        if (iter != end) {
            returnVal = new ActorAST(isIsolated);
            returnVal->filepos = (*iter)->pos;
            returnVal->name = (*iter)->data;

            ++iter;
            while ((iter != end) && ((*iter)->data != "end")) {
                if ((*iter)->data == "def") {
                    FunctionAST *fun = parse_function(iter, end);
                    if (fun != NULL) {
                        returnVal->children.push_back(fun);
                    }
                }
                else if ((*iter)->data == "action") {
                    ActionAST *act = parse_action(iter, end);
                    if (act != NULL) {
                        returnVal->children.push_back(act);
                    }
                }
                else if ((*iter)->tokenType == TokenType::Id){
                    //std::cout << "Var decl enter" << std::endl;
                	VarDeclExprAST *varDecl = parse_variable_decl(iter, end);
                    //std::cout << "Var decl leave" << std::endl;
                    if (varDecl != NULL) {
                        //FIXME: Check for duplicate definitions
                    	returnVal->children.push_back(varDecl);
					}
                    else {
                    	throw CompilerException("Unknown element in class", *(--iter));
                    }
                }
                else {
                    //FIXME: Not sure if this is correct
                    if (iter != end) {
                        ++iter;
                    }
                }
            }

            if (iter == end) {
                throw CompilerException("Expected 'end' at end of class", *(--iter));
            }
            else {
                ++iter; //skip over "end"
            }
        }
    }

	return returnVal;
}

AppAST* parse(std::vector<Token*>::iterator &iter,
		std::vector<Token*>::iterator &end) {

	AppAST *returnVal;
	ASTNode *child;

	returnVal = new AppAST();
	returnVal->filepos = (*iter)->pos;

	while (iter != end) {
		child = parse_function(iter, end);
		if (child != NULL) {
            //std::cout << "Pushing function" << std::endl;
			returnVal->children.push_back(child);
			continue;
		}
		child = parse_class(iter, end);
		if (child != NULL) {
            //std::cout << "Pushing class" << std::endl;
			returnVal->children.push_back(child);
			continue;
		}
		child = parse_actor(iter, end);
		if (child != NULL) {
            //std::cout << "Pushing actor" << std::endl;
			returnVal->children.push_back(child);
			continue;
		}
		child = parse_action(iter, end);
		if (child != NULL) {
		    //std::cout << "Pushing action" << std::endl;
			returnVal->children.push_back(child);
			continue;
		}

		if ((*iter)->tokenType == TokenType::EOL) {
		    ++iter;
		}
		else {
            std::ostringstream msg;
            msg << "Unknown element '" << (*iter)->data << "' of type " << (*iter)->tokenType;
            throw CompilerException(msg.str(), *iter);
		}
	}

	return returnVal;
}
