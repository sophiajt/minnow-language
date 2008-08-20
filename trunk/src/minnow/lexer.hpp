#ifndef MINNOW_LEXER_HPP
#define MINNOW_LEXER_HPP

#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

class TokenType {
public:
    enum Type { Id, Num, Char, String, Def, Bool, Symbol, EOL};
};

struct FilePos {
    std::string filename;
    unsigned int lineNumber, colStart, colEnd;

    FilePos& operator=( const FilePos& newPos ) {
        filename = newPos.filename;
        lineNumber = newPos.lineNumber;
        colStart = newPos.colStart;
        colEnd = newPos.colEnd;

        return *this;
    }
};

struct Token {
    TokenType::Type tokenType;
    std::string data;
    FilePos pos;

    Token () { }
    Token(TokenType::Type tType, std::string tData) : tokenType(tType), data(tData) { }
    Token(TokenType::Type tType, std::string tData, std::string tFilename, int tLineNumber, int tColStart, int tColEnd) :
            tokenType(tType), data(tData) {
        pos.filename = tFilename;
        pos.lineNumber = tLineNumber;
        pos.colStart = tColStart;
        pos.colEnd = tColEnd;
    }
};

class CompilerException : public std::runtime_error {
public:
    CompilerException(const std::string &msg, const int lineNumber, const int colStart, const int colEnd, const std::string &filename)
            : std::runtime_error(build_message(msg, lineNumber, colStart, colEnd, filename)) {
    }

    CompilerException(const std::string &msg, const Token *token)
            : std::runtime_error(build_message(msg, token->pos.lineNumber, token->pos.colStart, token->pos.colEnd, token->pos.filename)) {
    }

    CompilerException(const std::string &msg, const FilePos &pos)
    : std::runtime_error(build_message(msg, pos.lineNumber, pos.colStart, pos.colEnd, pos.filename)) {
    }

    CompilerException(const std::string &msg)
    : std::runtime_error(msg) {
    }

    std::string build_message(const std::string &msg, const int lineNumber, const int colStart, const int colEnd, const std::string &filename) const {
        std::ostringstream msg_builder;

        msg_builder << msg << " at line " << lineNumber << " column " << colStart << " in '" << filename << "'";

        return msg_builder.str();
    }
};

std::vector<Token*> tokenize(std::string sourceText, std::string filename);

#endif //MINNOW_LEXER_HPP
