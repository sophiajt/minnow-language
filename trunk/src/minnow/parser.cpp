#include <iostream>
#include <stdlib.h>

#include "lexer.hpp"
#include "parser.hpp"

#define EQUATE_PREC 2
#define MSG_PREC 4
#define COMMA_GROUP_PREC 5
#define SCOPE_PREC 6
#define COMPARE_PREC 10
#define ADD_SUB_PREC 20
#define MULT_DIV_PREC 30
#define POWER_PREC 40

/*
  This file has a decent amount of code inspired by Chris Lattner's Kaleidoscope LLVM example.  Some cool tricks.  Not sure
  how it will evolve over time, but definitely want to thank Chris for such clear examples.

  http://www.llvm.org/docs/tutorial/index.html
*/
int getPrecedence(const std::string &op) {
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
    else if ( (op == "<<") || (op == "::") ) {
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

ExpressionAST *parseParens(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    ExpressionAST *returnVal;
    ++iter;
    if (iter == end) {
        throw CompilerException("Unclosed parentheses", *(--iter));
    }
    else {
        returnVal = parseExpression(iter, end);
        returnVal->pos = (*iter)->pos;
        if ((*iter)->data != ")") {
            throw CompilerException("Unclosed parentheses", *(--iter));
        }
        else {
            ++iter;
        }
    }
    
    return returnVal;
}

ExpressionAST *parsePointcut(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    PointcutExprAST *returnVal;
    ++iter;
    if (iter == end) {
        throw CompilerException("Unfinished pointcut", *(--iter));
    }
    else {
        //parse pattern by looking for ! or {

        returnVal = new PointcutExprAST();
        returnVal->pos = (*iter)->pos;
        while ( ((*iter)->data != "!") && ((*iter)->data != "{") && (iter != end) ) {
            returnVal->pattern += ((*iter)->data);
            ++iter;
        }

        if (iter == end) {
            throw CompilerException("Unfinished pointcut", *(--iter));
        }

        if ((*iter)->data == "!") {
            returnVal->throwExOnFalse = true;
            ++iter;
        }

        if (iter == end) {
            throw CompilerException("Unfinished pointcut", *(--iter));
        }

        if ((*iter)->data != "{") {
            throw CompilerException("Unfinished pointcut", *(--iter));
        }
        else {
            ++iter;
            while ((iter != end) && ((*iter)->data != "}")) {
                //std::cout << "Tok: " << (*iter)->data << std::endl;
                returnVal->closure->body.push_back(parseExpression(iter, end));
            }
            if (iter == end) {
                throw CompilerException("Expected '}' to end pointcut block", *(--iter));
            }
        }
    }

    return returnVal;
}

ExpressionAST *parseArrayIndex(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    ExpressionAST *returnVal;
    ++iter;
    if (iter == end) {
        throw CompilerException("Unclosed square brackets", *(--iter));
    }
    else {
        returnVal = parseExpression(iter, end);
        returnVal->pos = (*iter)->pos;
        if ((*iter)->data != "]") {
            throw CompilerException("Unclosed square brackets", *(--iter));
        }
        else {
            ++iter;
        }
    }

    return returnVal;
}

ExpressionAST *parseBinaryOp(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end, int currPrec,
    ExpressionAST *expr) {

    //This uses Chris Lattner's cool precedence parsing trick
    int binPrec;
    //let's see if what we're sitting on is a math symbol
    while (iter != end) {
        std::string op = (*iter)->data;
        binPrec = getPrecedence(op);
        if (binPrec < currPrec) {
            //the next symbol in line is lower precedence than we need
            return expr;
        }
        ++iter;
        if (iter != end) {
            //check the precedence of the next hop
            ExpressionAST *rhs = parsePrimary(iter, end);
            if (rhs == NULL) {
                throw CompilerException("Incompete math equation", *iter);
            }

            //++iter;

            if (iter != end) {
                std::string op = (*iter)->data;
                int nextPrec = getPrecedence(op);

                //std::cout << "Outside dive: " << (*iter)->data << " " << binPrec << " " << nextPrec << std::endl;
                if (binPrec < nextPrec) {
                    //--iter;
                    //std::cout << "Diving in: " << (*iter)->data << std::endl;
                    rhs = parseBinaryOp(iter, end, binPrec+1, rhs);
                }
            }
            expr = new BinaryExprAST(op, expr, rhs);
            expr->pos = (*iter)->pos;
            
        }
        else {
            throw CompilerException("Incompete math equation", *(--iter));
        }
    }
    return expr;
}

ExpressionAST *parseExpression(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    ExpressionAST *lhs = parsePrimary(iter, end);
    if (lhs != NULL) {
        return parseBinaryOp(iter, end, 0, lhs);
    }
    else {
        return NULL;
    }
}

ExpressionAST *parseFunCall(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    ExpressionAST *returnVal = NULL;
    std::string name;
    std::vector<ExpressionAST*> args;

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
        args.push_back(parseExpression(iter, end));
        /*
        if ((*iter)->data == ",") {
            //skip to the next arg
            ++iter;
        }
        */
    }

    if (iter != end) {
        //skip the trailing ')'
        ++iter;
    }

    //what we have is probably a tree of args, but we want a flat list, so fix it
    if (args.size() > 0) {
        BinaryExprAST *beast = dynamic_cast<BinaryExprAST*>(args[0]);
        ExpressionAST *RHS;
        if ((beast != NULL) && (beast->op == ",")) {
            args.clear();
            while ((beast != NULL) && (beast->op == ",")) {
                args.push_back(beast->LHS);
                RHS = beast->RHS;
                beast = dynamic_cast<BinaryExprAST*>(RHS);
            }
            args.push_back(RHS);
        }
    }

    returnVal = new CallExprAST(name, args);
    returnVal->pos = (*iter)->pos;
    
    return returnVal;
}

ExpressionAST *parseIfBlock(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    IfExprAST *returnVal = new IfExprAST();
    returnVal->pos = (*iter)->pos;
    
    ++iter;
    if (iter == end) {
        throw CompilerException("Expected if block", *(--iter));
    }

    returnVal->Cond = parseExpression(iter, end);

    while ((iter != end) && ((*iter)->data != "end") && ((*iter)->data != "else")) {
        //std::cout << "Tok: " << (*iter)->data << std::endl;
        returnVal->Then.push_back(parseExpression(iter, end));
    }
    if (iter == end) {
        throw CompilerException("Expected 'end' to end if block", *(--iter));
    }

    if ((*iter)->data == "else") {
        ++iter;
        while ((iter != end) && ((*iter)->data != "end")) {
            //std::cout << "Else Tok: " << (*iter)->data << std::endl;
            returnVal->Else.push_back(parseExpression(iter, end));
        }
    }
    if (iter == end) {
        throw CompilerException("Expected 'end' to end if block", *(--iter));
    }
    else {
        ++iter;
    }

    return returnVal;
}

ExpressionAST *parseWhileBlock(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    WhileExprAST *returnVal = new WhileExprAST();
    returnVal->pos = (*iter)->pos;
    
    ++iter;
    if (iter == end) {
        throw CompilerException("Expected while block", *(--iter));
    }

    returnVal->Cond = parseExpression(iter, end);

    while ((iter != end) && ((*iter)->data != "end")) {
        //std::cout << "Tok: " << (*iter)->data << " " << (*iter)->tokenType << std::endl;
        ExpressionAST *ast = parseExpression(iter, end);
        //if (ast == NULL) {
        //    std::cout << "NULL EXPRESSION" << std::endl;
        //}
        returnVal->Loop.push_back(ast);
    }

    if (iter == end) {
        throw CompilerException("Expected 'end' to end if block", *(--iter));
    }
    else {
        ++iter;
    }

    return returnVal;
}

ExpressionAST *parseVarDecl(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    std::string name, type;
    bool isSpawn = false;
    bool isAlloc = false;
    bool isArray = false;

    ExpressionAST *returnVal = NULL;

    if ((*iter)->data == "spawn") {
        isSpawn = true;
    }

    if ((*iter)->data == "new") {
        isAlloc = true;
    }

    //the next two have to be ids.  the first is the type, the second the name.
    ++iter;
    if ((iter == end) || ((*iter)->tokenType != TokenType::Id)) {
        throw CompilerException("Expected type and name in variable declaration", *(--iter));
    }
    type = (*iter)->data;
    ++iter;
    if (iter == end) {
        throw CompilerException("Expected type and name in variable declaration", *(--iter));
    }
    if ((*iter)->data == "[") {
        //check to see if it's an array type
        ++iter;
        if ((iter == end) || ((*iter)->data != "]")) {
            throw CompilerException("Incomplete array variable declaration", *(--iter));
        }
        ++iter;
        isArray = true;
    }

    if ((iter == end) || ((*iter)->tokenType != TokenType::Id)) {
        throw CompilerException("Expected type and name in variable declaration", *(--iter));
    }
    
    name = (*iter)->data;
    ++iter;
    if (iter != end) {
        if ((*iter)->data == "[") {
            if (isArray == false) {
                throw CompilerException("Incorrect type for array declaration, use typename followed by [] to signify an array", *(--iter));
            }       
            ExpressionAST *size = parseArrayIndex(iter, end);
            returnVal = new ArrayDeclExprAST(name, type, size, isSpawn, isAlloc);
            returnVal->pos = (*iter)->pos;
        }
        else if (isArray == true) {
            returnVal = new ArrayDeclExprAST(name, type, NULL, isSpawn, isAlloc);
            returnVal->pos = (*iter)->pos;
        }
        else {
            returnVal = new VarDeclExprAST(name, type, isSpawn, isAlloc);
            returnVal->pos = (*iter)->pos;
        }
    }
    else {
        returnVal = new VarDeclExprAST(name, type, isSpawn, isAlloc);
        returnVal->pos = (*iter)->pos;
    }

    return returnVal;
 }

ExpressionAST *parsePrimary(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    ExpressionAST *returnVal = NULL;
    ExpressionAST *index = NULL;
    double val;
    bool boolval;

    std::string name, type;

    switch ((*iter)->tokenType) {
        case (TokenType::Num) :
            val = atof((*iter)->data.c_str());
            returnVal = new NumberExprAST(val);
            returnVal->pos = (*iter)->pos;
            ++iter;
            break;
        case (TokenType::String) :
            returnVal = new QuoteExprAST((*iter)->data);
            returnVal->pos = (*iter)->pos;
            ++iter;
            break;
        case (TokenType::Id) :
            if ((*iter)->data == "end") {
                returnVal = new EndExprAST();
                returnVal->pos = (*iter)->pos;
            }
            else if ((*iter)->data == "var") {
                returnVal = parseVarDecl(iter, end);
            }
            else if ((*iter)->data == "spawn") {
                returnVal = parseVarDecl(iter, end);
            }
            else if ((*iter)->data == "new") {
                returnVal = parseVarDecl(iter, end);
            }
            else if ((*iter)->data == "if") {
                returnVal = parseIfBlock(iter, end);
            }
            else if ((*iter)->data == "while") {
                returnVal = parseWhileBlock(iter, end);
            }
            else {
                name = (*iter)->data;
                ++iter;

                if (iter == end) {
                    returnVal = new VariableExprAST(name);
                    returnVal->pos = (*iter)->pos;
                }
                else {
                    if ((*iter)->data == "(") {
                        --iter;
                        returnVal = parseFunCall(iter, end);
                    }
                    else if ((*iter)->data == "[") {
                        index = parseArrayIndex(iter, end);
                        returnVal = new ArrayIndexedExprAST(name, index);
                        returnVal->pos = (*iter)->pos;
                    }
                    else {
                        returnVal = new VariableExprAST(name);
                        returnVal->pos = (*iter)->pos;
                    }
                }
            }
            break;
        case (TokenType::EOL) :
            ++iter;
            break;
        case (TokenType::Symbol) :
            if ((*iter)->data == "(") {
                returnVal = parseParens(iter, end);
            }
            else if ((*iter)->data == "@") {
                returnVal = parsePointcut(iter, end);
            }
            else {
                std::cout << "Element: " << (*iter)->data << std::endl;
                throw CompilerException("Unknown symbol", *iter);
            }
            break;
        case (TokenType::Bool) :
            
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
            returnVal->pos = (*iter)->pos;
            ++iter;
            
            break;
        default:
            std::ostringstream msg;
            msg << "Unknown token type:" << (*iter)->tokenType;
            throw CompilerException(msg.str(), *iter);
    }

    return returnVal;
}

PrototypeAST *parsePrototype(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end, CodeHolder *holder) {
    PrototypeAST *proto = NULL;

    if ((iter != end) && ((*iter)->tokenType == TokenType::Id)) {
        proto = new PrototypeAST();

        //FIXME: Add type parsing here, so we can allow things like arrays/dictionaries/etc
        
        proto->type.declType = (*iter)->data;
        proto->type.typeType = TypeType::Scalar;

        ++iter;

        if (iter == end) {
            throw CompilerException("Incomplete function definition", *(--iter));
        }
        if ((*iter)->data == "[") {
            ++iter;
            if (iter == end) {
                throw CompilerException("Incomplete function definition", *(--iter));
            }
            if ((*iter)->data != "]") {
                throw CompilerException("Incomplete function definition", *(--iter));
            }
            ++iter;
            proto->type.typeType = TypeType::Array;
        }
        
        if (iter == end) {
            throw CompilerException("Expected function definition", *(--iter));
        }

        if ((*iter)->data != "(") {
            proto->name = (*iter)->data;
            ++iter;
            if (iter == end) {
                throw CompilerException("Expected function definition", *(--iter));
            }
        }
        else {
            proto->name = proto->type.declType;
            proto->type.declType = "void";
        }

        //current version: def <name>

        if ((*iter)->data == "(") {
            //we've got some parms

            ++iter;
            if (iter == end) {
                throw CompilerException("Expecting ')' for function definition", *iter);
            }

            std::string name, type;

            while (((*iter)->data != ")") && (iter != end)) {
                bool isArray = false;
                if ((*iter)->tokenType != TokenType::Id) {
                    throw CompilerException("Expecting type in function definition", *iter);
                }

                type = (*iter)->data;
                ++iter;

                if (iter == end) {
                    throw CompilerException("Incomplete function definition", *(--iter));
                }
                if ((*iter)->data == "[") {
                    ++iter;
                    if (iter == end) {
                        throw CompilerException("Incomplete function definition", *(--iter));
                    }
                    if ((*iter)->data != "]") {
                        throw CompilerException("Incomplete function definition", *(--iter));
                    }
                    ++iter;
                    isArray = true;
                }
                if ((*iter)->tokenType != TokenType::Id) {
                    throw CompilerException("Expecting name in function definition", *iter);
                }
                name = (*iter)->data;
           
                VariableInfo *vi;
                if (isArray) {
                    vi = new VariableInfo(name, type, TypeType::Array, ScopeType::Prototype);
                }
                else {
                    vi = new VariableInfo(name, type, TypeType::Scalar, ScopeType::Prototype);
                }

                //FIXME: How much is CodeHolder used?
                if (holder->vars.find(name) == holder->vars.end()) {
                    holder->vars[name] = vi;
                }
                else {
                    throw CompilerException("Duplicated variable name in function", *iter);
                }
                //proto->argNames.push_back(name);
                //proto->argTypes.push_back(type);
                proto->args.push_back(vi);
                ++iter;
                if ((iter != end) && ((*iter)->data == ",")) {
                    //skip to the next arg
                    ++iter;
                }
            }
            if (iter != end) {
                if ((*iter)->data == ")") {
                    //skip over ')'
                    ++iter;
                }
            }
        }
    }

    return proto;
}

ActionAST *parseAction(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    ActionAST *returnVal = NULL;

    if ((iter != end) && ((*iter)->data == "action")) {
        //std::cout << "Found def..." << std::endl;
        //we may be at a definition
        ++iter;
        if (iter != end) {
            returnVal = new ActionAST();
            PrototypeAST *proto = parsePrototype(iter, end, returnVal);
            if (proto == NULL) {
                throw CompilerException("Expecting action definition", *iter);
            }
            returnVal->proto = proto;

            ExpressionAST *ast = parseExpression(iter, end);
            while (iter != end) {
                //std::cout << "Checking expressions..." << (*iter)->data << " " << (*iter)->tokenType << std::endl;
                if (ast != NULL) {
                    //std::cout << "..." << ast->type() << std::endl;

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
        }
    }
    else {
        //std::cout << "Found nothing..." << std::endl;
    }

    return returnVal;
}

FunctionAST *parseFunction(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    FunctionAST *returnVal = NULL;

    if ((iter != end) && ((*iter)->data == "extern")) {
        //std::cout << "Found extern..." << std::endl;
        ++iter;
        if (iter != end) {
            returnVal = new FunctionAST();
            PrototypeAST *proto = parsePrototype(iter, end, returnVal);
            if (proto == NULL) {
                throw CompilerException("Expecting external function definition", *iter);
            }
            returnVal->proto = proto;

        }
    }
    else if ((iter != end) && ((*iter)->data == "def")) {
        //std::cout << "Found def..." << std::endl;
        //we may be at a definition
        ++iter;
        if (iter != end) {
            returnVal = new FunctionAST();
            PrototypeAST *proto = parsePrototype(iter, end, returnVal);
            if (proto == NULL) {
                throw CompilerException("Expecting function definition", *iter);
            }
            returnVal->proto = proto;

            ExpressionAST *ast = parseExpression(iter, end);
            while (iter != end) {
                //std::cout << "Checking expressions..." << (*iter)->data << " " << (*iter)->tokenType << std::endl;
                if (ast != NULL) {
                    //std::cout << "..." << ast->type() << std::endl;

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
        }
    }
    else {
        //std::cout << "Found nothing..." << std::endl;
    }

    return returnVal;
}
StructAST *parseStruct(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    StructAST *returnVal = NULL;

    if ((iter != end) && ((*iter)->data == "class")) {

        //std::cout << "Found extern..." << std::endl;
        ++iter;
        if (iter != end) {
            returnVal = new StructAST();
            returnVal->name = (*iter)->data;
            //FIXME: Check if name is valid

            ++iter;
            while ((iter != end) && ((*iter)->data != "end")) {
                if ((*iter)->data == "var") {
                    ExpressionAST *var = parseVarDecl(iter, end);
                    if (var != NULL) {
                        VarDeclExprAST *vdeast = dynamic_cast<VarDeclExprAST*>(var);
                        ArrayDeclExprAST *adeast = dynamic_cast<ArrayDeclExprAST*>(var);
                        if (vdeast != NULL) {
                            VariableInfo *vi = new VariableInfo(vdeast->name, vdeast->declType, TypeType::Scalar, ScopeType::Struct);
                            if (returnVal->vars.find(vdeast->name) != returnVal->vars.end()) {
                                throw CompilerException("Duplicate variable definition", *(--iter));
                            }
                            else {
                                returnVal->vars[vdeast->name] = vi;
                            }
                        }
                        else if (adeast != NULL) {
                            VariableInfo *vi = new VariableInfo(adeast->name, adeast->declType, TypeType::Array, adeast->size, ScopeType::Struct);
                            if (returnVal->vars.find(adeast->name) != returnVal->vars.end()) {
                                throw CompilerException("Duplicate variable definition", *(--iter));
                            }
                            else {
                                returnVal->vars[adeast->name] = vi;
                            }
                        }
                        
                    }
                }
                else if ((*iter)->data == "def") {
                    FunctionAST *fun = parseFunction(iter, end);
                    if (fun != NULL) {
                        returnVal->funs.push_back(fun);
                    }
                }
                else {
                    if (iter != end)
                        ++iter;
                }
            }

            if (iter == end) {
                throw CompilerException("Expected 'end' at end of struct", *(--iter));
            }

        }

    }

    return returnVal;
}

ActorAST *parseActor(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    ActorAST *returnVal = NULL;
    bool isIsolated = false;
    
    if ((iter != end) && ((*iter)->data == "isolated")) {
        isIsolated = true;
        ++iter;
    }
    if ((iter != end) && ((*iter)->data == "actor")) {
        //std::cout << "Found extern..." << std::endl;
        ++iter;
        if (iter != end) {
            returnVal = new ActorAST(isIsolated);
            returnVal->name = (*iter)->data;
            //FIXME: Check if name is valid

            ++iter;
            while ((iter != end) && ((*iter)->data != "end")) {
                
                if ((*iter)->data == "var") {
                    /*
                    VarDeclExprAST *var = parseVarDecl(iter, end);
                    if (var != NULL) {
                        if (returnVal->vars.find(var->name) != returnVal->vars.end()) {
                            throw CompilerException("Duplicate variable definition", *(--iter));
                        }
                        else {
                            returnVal->vars[var->name] = var->declType;
                        }
                    }*/
                    /*
                    ExpressionAST *var = parseVarDecl(iter, end);
                    if (var != NULL) {
                        returnVal->vars.push_back(var);
                    }
                    */
                    ExpressionAST *var = parseVarDecl(iter, end);
                    if (var != NULL) {
                        VarDeclExprAST *vdeast = dynamic_cast<VarDeclExprAST*>(var);
                        ArrayDeclExprAST *adeast = dynamic_cast<ArrayDeclExprAST*>(var);
                        if (vdeast != NULL) {
                            VariableInfo *vi = new VariableInfo(vdeast->name, vdeast->declType, TypeType::Scalar, ScopeType::Actor);
                            if (returnVal->vars.find(vdeast->name) != returnVal->vars.end()) {
                                throw CompilerException("Duplicate variable definition", *(--iter));
                            }
                            else {
                                returnVal->vars[vdeast->name] = vi;
                            }
                        }
                        else if (adeast != NULL) {
                            VariableInfo *vi = new VariableInfo(adeast->name, adeast->declType, TypeType::Array, adeast->size, ScopeType::Actor);
                            if (returnVal->vars.find(adeast->name) != returnVal->vars.end()) {
                                throw CompilerException("Duplicate variable definition", *(--iter));
                            }
                            else {
                                returnVal->vars[adeast->name] = vi;
                            }
                        }
                    }
                }
                else if ((*iter)->data == "def") {
                    FunctionAST *fun = parseFunction(iter, end);
                    if (fun != NULL) {
                        returnVal->funs.push_back(fun);
                    }
                }
                else if ((*iter)->data == "action") {
                    ActionAST *act = parseAction(iter, end);
                    if (act != NULL) {
                        returnVal->actions.push_back(act);
                    }
                }
                else {
                    if (iter != end)
                        ++iter;
                }
            }

            if (iter == end) {
                throw CompilerException("Expected 'end' at end of struct", *(--iter));
            }

        }

    }

    return returnVal;
}


AppAST *parseApp(std::vector<Token*>::iterator &iter, std::vector<Token*>::iterator &end) {
    AppAST *app = new AppAST();
    //StructAST *struc = parseStruct(iter, end);
    //std::cout << "ParseApp" << std::endl;
    //FunctionAST *fun = parseFunction(iter, end);
    StructAST *struc = NULL;
    FunctionAST *fun = NULL;
    ActorAST *act = NULL;
    ActionAST *mainact = NULL;
    while (iter != end) {
        fun = parseFunction(iter, end);
        if (fun != NULL) {
            app->functions.push_back(fun);
        }
        struc = parseStruct(iter, end);
        if (struc != NULL) {
            app->structs.push_back(struc);
        }
        act = parseActor(iter, end);
        if (act != NULL) {
            app->actors.push_back(act);
        }
        mainact = parseAction(iter, end);
        if (mainact != NULL) {
            app->actions.push_back(mainact);
        }

        if ( (fun == NULL) && (struc == NULL) && (act == NULL) && (mainact == NULL) ) {
            ++iter;
        }
    }

    return app;
}
