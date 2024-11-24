#pragma once

#include <string>

namespace db
{

namespace lexer
{

enum TokenType
{
    TOK_CREATE,
    TOK_TABLE,
    TOK_INSERT,
    TOK_SELECT,
    TOK_UPDATE,
    TOK_DELETE,
    TOK_INDEX,
    TOK_JOIN,
    TOK_ON,
    TOK_SET,
    TOK_FROM,
    TOK_WHERE,
    TOK_TRUE,
    TOK_FALSE,
    TOK_TO,
    TOK_BY,
    TOK_ORDERED,

    TOK_INT32,
    TOK_STRING,
    TOK_BYTES,
    TOK_BOOL,
    TOK_INT_LITERAL,
    TOK_STRING_LITERAL,
    TOK_BOOL_LITERAL,

    TOK_ATT_UNIQUE,
    TOK_ATT_AUTOINCREMENT,
    TOK_ATT_KEY,

    TOK_IDENTIFIER,
    TOK_DOT,           // .
    TOK_LPAREN,        // (
    TOK_RPAREN,        // )
    TOK_COMMA,         // ,
    TOK_COLON,         // :
    TOK_LBRACE,        // {
    TOK_RBRACE,        // }
    TOK_LBRACKET,       // [
    TOK_RBRACKET,       // ]
    TOK_EQUAL,         // =
    TOK_PLUS,          // +
    TOK_MINUS,         // -
    TOK_MULTIPLY,      // *
    TOK_DIVIDE,        // /
    TOK_MODULO,        // %
    TOK_LESS,          // <
    TOK_GREATER,       // >
    TOK_LESS_EQUAL,    // <=
    TOK_GREATER_EQUAL, // >=
    TOK_NOT_EQUAL,     // !=
    TOK_AND,           // &&
    TOK_OR,            // ||
    TOK_XOR,           // ^^
    TOK_NOT,           // !
    TOK_BITWISE_OR,    // |
    TOK_HEX_LITERAL,   // 0x...
    TOK_EOF,
};

struct Token
{
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

class Lexer
{
public:
    Lexer(const std::string& input)
        : input(input)
    {
    }

    Token getNextToken();

private:
    std::string input;
    size_t pos;
    int line;
    int column;

    char peek() const;
    char get();
    void skipWhitespace();
    Token identifierOrKeyword();
    Token hexNumber();
    Token number();
    Token stringLiteral();
    Token parseOperator();
};

} // namespace lexer

} // namespace db
