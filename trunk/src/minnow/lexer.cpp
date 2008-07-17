#include <string>

#include <stdio.h>

#include "lexer.hpp"

#define INC_LOC ++strIter; ++colNumber;

inline bool ispunctother(char c) {
    if (ispunct(c) && (c != '"') && (c != '\'') && (c != '{') && (c != '}') && (c != '[') && (c != ']') && (c != '(') &&
        (c != ')'))
    {
        return true;
    }
    else {
        return false;
    }
}

std::vector<Token *> tokenize(std::string sourceText, std::string filename) {
    std::string::iterator strIter = sourceText.begin(), strEnd = sourceText.end();
    std::vector<Token*> tokens;
    int lineNumber = 1;
    int colNumber = 0;
    Token *tok;
    bool extendLine = false;

    while (strIter != strEnd) {
        if (isspace(*strIter)) {
            if (*strIter == '\r') {
                int colStart = colNumber;

                if (*strIter == '\n') {
                    ++strIter;
                }

                if (!extendLine) {
                    tok = new Token(TokenType::EOL, "", filename, lineNumber, colStart, colNumber);
                    tokens.push_back(tok);
                }
                extendLine = false;

                ++lineNumber;
                colNumber = 0;
                ++strIter;
            }
            else if (*strIter == '\n') {
                int colStart = colNumber;

                if (!extendLine) {
                    tok = new Token(TokenType::EOL, "", filename, lineNumber, colStart, colNumber);
                    tokens.push_back(tok);
                }
                extendLine = false;

                ++lineNumber;
                colNumber = 0;
                ++strIter;
            }
            else {
                INC_LOC;
            }
        }
        else if (isalpha(*strIter)) {
            //we're either in a reserved word or an identifier
            std::string id;
            int colStart = colNumber;
            while ((isalnum(*strIter) || (*strIter == '_')) && (strIter != strEnd)) {
                id += *strIter;
                INC_LOC;
            }
            if (id == "def") {
                tok = new Token(TokenType::Def, id, filename, lineNumber, colStart, colNumber);
            }
            else if ((id == "true") || (id == "false")) {
                tok = new Token(TokenType::Bool, id, filename, lineNumber, colStart, colNumber);
            }
            else {
                tok = new Token(TokenType::Id, id, filename, lineNumber, colStart, colNumber);
            }
            tokens.push_back(tok);
        }
        else if (isdigit(*strIter)) {
            std::string num;
            int colStart = colNumber;
            bool hasDot = false;
            while ((isdigit(*strIter) || (*strIter == '.')) && (strIter != strEnd)) {
                if (*strIter == '.') {
                    if (hasDot)
                        throw CompilerException("Too many decimal places in number", lineNumber, colStart, colNumber);
                    else
                        hasDot = true;
                }
                num += *strIter;
                INC_LOC;
            }
            tok = new Token(TokenType::Num, num, filename, lineNumber, colStart, colNumber);
            //we're in a number
            tokens.push_back(tok);
        }
        else if (*strIter == '"') {
            //quoted string
            std::string strVal;
            int colStart = colNumber;
            INC_LOC;
            while ((strIter != strEnd) && (*strIter != '"')) {
                if (*strIter == '\\') {
                    ++strIter;
                    if (strIter != strEnd) {
                        switch (*strIter) {
                            default:
                               strVal += *strIter;
                        }
                    }
                }
                else {
                    strVal += *strIter;
                }
                INC_LOC;
            }

            if (*strIter == '"') ++strIter;

            tok = new Token(TokenType::String, strVal, filename, lineNumber, colStart, colNumber);
            //we're in a number
            tokens.push_back(tok);
        }
        else if (*strIter == '\'') {
            //quoted string
            std::string chrVal;
            int colStart = colNumber;
            INC_LOC;
            while ((strIter != strEnd) && (*strIter != '\'')) {
                if (*strIter == '\\') {
                    ++strIter;
                    if (strIter != strEnd) {
                        switch (*strIter) {
                            default:
                               chrVal += *strIter;
                        }
                    }
                }
                else {
                    chrVal += *strIter;
                }
                INC_LOC;
            }

            if (*strIter == '\'') ++strIter;

            tok = new Token(TokenType::Char, chrVal, filename, lineNumber, colStart, colNumber);
            //we're in a number
            tokens.push_back(tok);
        }
        else if ((*strIter == '(') || (*strIter == ')') || (*strIter == '[') || (*strIter == ']') || (*strIter == '{') ||
            (*strIter == '}')) {

            //Handle the "one off" type of symbols, those that don't clump and have to be handled separately
            std::string oneoff;
            oneoff += *strIter;
            int colStart = colNumber;
            tok = new Token(TokenType::Symbol, oneoff, filename, lineNumber, colStart, colNumber);
            tokens.push_back(tok);
            INC_LOC;
        }
        else if (ispunctother(*strIter)) {
            std::string punct;
            int colStart = colNumber;
            while ((ispunctother(*strIter)) && (strIter != strEnd)) {
                punct += *strIter;
                INC_LOC;
            }
            if (punct == "...") {
                //this means the line continues to the next line, so let's eat that carriage return and keep going
                extendLine = true;
            }
            else if (punct == "/*") {
                INC_LOC;
                while (!((*strIter == '*')&&(*(strIter+1) == '/')) && (strIter != strEnd)) {
                    INC_LOC;
                }
                if (strIter == strEnd) {
                    throw CompilerException("Unclosed comment", lineNumber, colStart, colStart);
                }
                else {
                    //if we're okay, skip over the end of the comment
                    INC_LOC;
                    INC_LOC;
                }
            }
            else if (punct == "//") {
                INC_LOC;
                while ((*strIter != '\r')&&(*strIter != '\n') && (strIter != strEnd)) {
                    INC_LOC;
                }
            }
            else {
                tok = new Token(TokenType::Symbol, punct, filename, lineNumber, colStart, colNumber);
                tokens.push_back(tok);
            }
        }
        else {
            throw CompilerException("Unknown character", lineNumber, colNumber, colNumber);
        }
    }

    return tokens;
}
