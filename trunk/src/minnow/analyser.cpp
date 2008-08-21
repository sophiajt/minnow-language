#include "analyser.hpp"
#include "parser.hpp"

ASTNode *Analyser::analyseScopeAndTypes(ASTNode* input) {
    VariableInfo *vi;

    NumberExprAST *neast;
    BooleanExprAST *boeast;
    QuoteExprAST *qeast;
    VariableExprAST *veast;
    ArrayIndexedExprAST *aieast;
    BinaryExprAST *beast;
    CallExprAST *ceast;

    if (input == NULL) {
        //NOP
        throw CompilerException("Internal error: AST node is NULL.");
    }
    //Number, Variable, ArrayIndexed, Binary, Quote, Call, DefFun, End, VarDecl, ArrayDecl, If, While
    switch (input->type()) {
        case (NodeType::Number) :
            neast = dynamic_cast<NumberExprAST*>(input);
            if (neast == NULL) {
                printf("FIXME: Number compiler exception\n");
            }
            //FIXME: Check for dot for floats
            input->programmaticType.declType = "int";
            input->programmaticType.containerType = ContainerType::Scalar;
            break;
        case (NodeType::Boolean) :
            boeast = dynamic_cast<BooleanExprAST*>(input);
            if (boeast == NULL) {
                printf("FIXME: Boolean compiler exception\n");
            }
            input->programmaticType.declType = "bool";
            input->programmaticType.containerType = ContainerType::Scalar;
            break;

        case (NodeType::Variable) :
            veast = dynamic_cast<VariableExprAST*>(input);
            if (veast == NULL) {
                printf("FIXME: Variable compiler exception\n");
            }
            vi = findVarInScope(veast->name);
            if (vi != NULL) {
                input->programmaticType = vi->type;
            }
            else {
                std::ostringstream msg;
                msg << "Can not find variable '" << veast->name << "'";
                throw CompilerException(msg.str(), input->filepos);
            }
            break;
        case (NodeType::ArrayIndexed) :
            aieast = dynamic_cast<ArrayIndexedExprAST*>(input);
            if (input->children.size() == 0) {
                throw CompilerException("Incomplete array index", input->filepos);
            }
            analyseScopeAndTypes(input->children[0]);
            if (input->children[0]->programmaticType.declType != "int") {
                throw CompilerException("Non-integer array index", input->children[0]->filepos);
            }
            if (aieast == NULL) {
                printf("FIXME: Array indexed compiler exception\n");
            }
            vi = findVarInScope(aieast->name);
            if (vi != NULL) {
                if (vi->type.containerType != ContainerType::Array) {
                    throw CompilerException("Indexing of non-array variable", input->filepos);
                }
                input->programmaticType = vi->type.containedTypes[0];
            }
            else {
                std::ostringstream msg;
                msg << "Can not find variable '" << aieast->name << "'";
                throw CompilerException(msg.str(), input->filepos);
            }
            break;
        case (NodeType::Binary) :
            beast = dynamic_cast<BinaryExprAST*>(input);
            if (beast == NULL) {
                printf("FIXME: Binary compiler exception\n");
            }
            if (input->children.size() != 2) {
                throw CompilerException("Internal Error: incorrect number of binary children",
                        input->filepos);
            }
            analyseScopeAndTypes(input->children[0]);
            analyseScopeAndTypes(input->children[1]);

            if (beast->op == "::") {
                input->programmaticType.declType = "void";
                input->programmaticType.containerType = ContainerType::Scalar;
            }
            else if (beast->op == "."){

            }
            break;
            /*
        case (NodeType::Quote) :
            qeast = dynamic_cast<QuoteExprAST*>(ast);
            if (qeast == NULL) {
                printf("FIXME: Quote compiler exception\n");
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
            */
        default:
            throw CompilerException("Can't resolve type during lookup", input->filepos);
    }

    return input;
}
