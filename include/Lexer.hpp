#pragma once

#include <iostream>
#include <string>

namespace db
{

namespace lexer
{

enum TokenType
{
    TOK_CREATE = 0,
    TOK_TABLE = 1,
    TOK_INSERT = 2,
    TOK_SELECT = 3,
    TOK_UPDATE = 4,
    TOK_DELETE = 5,
    TOK_INDEX = 6,
    TOK_JOIN = 7,
    TOK_ON = 8,
    TOK_SET = 9,
    TOK_FROM = 10,
    TOK_WHERE = 11,
    TOK_TRUE = 12,
    TOK_FALSE = 13,
    TOK_TO = 14,
    TOK_BY = 15,
    TOK_ORDERED = 16,

    TOK_INT32 = 17,
    TOK_STRING = 18,
    TOK_BYTES = 19,
    TOK_BOOL = 20,
    TOK_INT_LITERAL = 21,
    TOK_STRING_LITERAL = 22,
    TOK_HEX_LITERAL = 23,  
    TOK_BOOL_LITERAL = 24,

    TOK_ATT_UNIQUE = 25,
    TOK_ATT_AUTOINCREMENT = 26,
    TOK_ATT_KEY = 27,

    TOK_IDENTIFIER = 28,
    TOK_DOT = 29,           // .
    TOK_LPAREN = 30,        // (
    TOK_RPAREN = 31,        // )
    TOK_COMMA = 32,         // ,
    TOK_COLON = 33,         // :
    TOK_LBRACE = 34,        // {
    TOK_RBRACE = 35,        // }
    TOK_LBRACKET = 36,      // [
    TOK_RBRACKET = 37,      // ]
    TOK_EQUAL = 38,         // =
    TOK_PLUS = 39,          // +
    TOK_MINUS = 40,         // -
    TOK_MULTIPLY = 41,      // *
    TOK_DIVIDE = 42,        // /
    TOK_MODULO = 43,        // %
    TOK_LESS = 44,          // <
    TOK_GREATER = 45,       // >
    TOK_LESS_EQUAL = 46,    // <=
    TOK_GREATER_EQUAL = 47, // >=
    TOK_NOT_EQUAL = 48,     // !=
    TOK_AND = 49,           // &&
    TOK_OR = 50,            // ||
    TOK_XOR = 51,           // ^^
    TOK_NOT = 52,           // !
    TOK_BITWISE_OR = 53,    // |
    TOK_EOF = 54,
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
#ifdef DEBUG
        std::cout << "Lexer input: " << input << std::endl;
#endif
    }

    Token getNextToken();

private:
    std::string input;
    size_t pos = 0;
    int line = 0;
    int column = 0;

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
