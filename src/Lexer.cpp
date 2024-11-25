#include "Lexer.hpp"

#include "DataBaseException.hpp"
#include <algorithm>

namespace db
{

namespace lexer
{

char Lexer::peek() const
{
    if (pos >= input.size())
    {
        return '\0';
    }
    return input[pos];
}

char Lexer::get()
{
    char c = peek();
    pos++;
    column++;
    return c;
}

void Lexer::skipWhitespace()
{
    while (isspace(peek()))
    {
        get();
    }
}

Token Lexer::identifierOrKeyword()
{
    size_t start = pos;
    while (isalnum(peek()) || peek() == '_')
    {
        get();
    }
    std::string lexeme = input.substr(start, pos - start);
    std::string upperLexeme = lexeme;
    std::transform(upperLexeme.begin(), upperLexeme.end(), upperLexeme.begin(),
                   ::toupper);

    if (upperLexeme == "CREATE")
        return Token{ TOK_CREATE, lexeme, line, column };
    if (upperLexeme == "TABLE")
        return Token{ TOK_TABLE, lexeme, line, column };
    if (upperLexeme == "INSERT")
        return Token{ TOK_INSERT, lexeme, line, column };
    if (upperLexeme == "SELECT")
        return Token{ TOK_SELECT, lexeme, line, column };
    if (upperLexeme == "UPDATE")
        return Token{ TOK_UPDATE, lexeme, line, column };
    if (upperLexeme == "DELETE")
        return Token{ TOK_DELETE, lexeme, line, column };
    if (upperLexeme == "INDEX")
        return Token{ TOK_INDEX, lexeme, line, column };
    if (upperLexeme == "JOIN")
        return Token{ TOK_JOIN, lexeme, line, column };
    if (upperLexeme == "ON")
        return Token{ TOK_ON, lexeme, line, column };
    if (upperLexeme == "SET")
        return Token{ TOK_SET, lexeme, line, column };
    if (upperLexeme == "FROM")
        return Token{ TOK_FROM, lexeme, line, column };
    if (upperLexeme == "WHERE")
        return Token{ TOK_WHERE, lexeme, line, column };
    if (upperLexeme == "TRUE")
        return Token{ TOK_TRUE, lexeme, line, column };
    if (upperLexeme == "FALSE")
        return Token{ TOK_FALSE, lexeme, line, column };
    if (upperLexeme == "TO")
        return Token{ TOK_TO, lexeme, line, column };
    if (upperLexeme == "BY")
        return Token{ TOK_BY, lexeme, line, column };
    if (upperLexeme == "ORDERED")
        return Token{ TOK_ORDERED, lexeme, line, column };
    // if (upperLexeme == "AUTOINCREMENT")
    //     return Token{ TOK_ATT_AUTOINCREMENT, lexeme, line, column };
    // if (upperLexeme == "KEY")
    //     return Token{ TOK_ATT_KEY, lexeme, line, column };
    // if (upperLexeme == "UNIQUE")
    //     return Token{ TOK_ATT_UNIQUE, lexeme, line, column };
    if (upperLexeme == "INT32")
        return Token{ TOK_INT32, lexeme, line, column };
    if (upperLexeme == "STRING")
        return Token{ TOK_STRING, lexeme, line, column };
    if (upperLexeme == "BYTES")
        return Token{ TOK_BYTES, lexeme, line, column };
    if (upperLexeme == "BOOL")
        return Token{ TOK_BOOL, lexeme, line, column };
    return Token{ TOK_IDENTIFIER, lexeme, line, column };
}

Token Lexer::parseOperator()
{
    char currentChar = get();
    switch (currentChar)
    {
    case '+':
        return Token{ TOK_PLUS, "+", line, column };
    case '-':
        return Token{ TOK_MINUS, "-", line, column };
    case '*':
        return Token{ TOK_MULTIPLY, "*", line, column };
    case '/':
        return Token{ TOK_DIVIDE, "/", line, column };
    case '%':
        return Token{ TOK_MODULO, "%", line, column };
    case '<':
        if (peek() == '=')
        {
            get();
            return Token{ TOK_LESS_EQUAL, "<=", line, column };
        }
        else
        {
            return Token{ TOK_LESS, "<", line, column };
        }
    case '>':
        if (peek() == '=')
        {
            get();
            return Token{ TOK_GREATER_EQUAL, ">=", line, column };
        }
        else
        {
            return Token{ TOK_GREATER, ">", line, column };
        }
    case '=':
        if (peek() == '=')
        {
            get();
            return Token{ TOK_EQUAL, "==", line, column };
        }
        else
        {
            return Token{ TOK_EQUAL, "=", line, column };
        }
    case '!':
        if (peek() == '=')
        {
            get();
            return Token{ TOK_NOT_EQUAL, "!=", line, column };
        }
        else
        {
            return Token{ TOK_NOT, "!", line, column };
        }
    case '&':
        if (peek() == '&')
        {
            get();
            return Token{ TOK_AND, "&&", line, column };
        }
        else
        {
            throw DatabaseException("Unexpected character '&'");
        }
    case '|':
        return Token{ TOK_BITWISE_OR, "|", line, column };
    case '^':
        if (peek() == '^')
        {
            get();
            return Token{ TOK_XOR, "^^", line, column };
        }
        else
        {
            throw DatabaseException("Unexpected character '^'");
        }
    case '(':
        return Token{ TOK_LPAREN, "(", line, column };
    case ')':
        return Token{ TOK_RPAREN, ")", line, column };
    case ',':
        return Token{ TOK_COMMA, ",", line, column };
    case ':':
        return Token{ TOK_COLON, ":", line, column };
    case '{':
        return Token{ TOK_LBRACE, "{", line, column };
    case '}':
        return Token{ TOK_RBRACE, "}", line, column };
    case '[':
        return Token{ TOK_LBRACKET, "[", line, column };
    case ']':
        return Token{ TOK_RBRACKET, "]", line, column };
    case '.':
        return Token{ TOK_DOT, ".", line, column };
    default:
        throw DatabaseException(
            std::string("Unexpected character '") + currentChar + "' at line " +
            std::to_string(line) + ", column " + std::to_string(column));
    }
}

Token Lexer::getNextToken()
{
    skipWhitespace();

    if (pos >= input.size())
    {
        return Token{ TOK_EOF, "", line, column };
    }

    char currentChar = peek();

    if (isalpha(currentChar) || currentChar == '_')
    {
        return identifierOrKeyword();
    }
    else if (isdigit(currentChar))
    {
        return number();
    }
    else if (currentChar == '"')
    {
        return stringLiteral();
    }
    else
    {
        return parseOperator();
    }
}

Token Lexer::number()
{
    if (peek() == '0' && (input[pos + 1] == 'x' || input[pos + 1] == 'X'))
    {
        return hexNumber();
    }
    size_t start = pos;
    while (isdigit(peek()))
    {
        get();
    }
    std::string lexeme = input.substr(start, pos - start);
    return Token{ TOK_INT_LITERAL, lexeme, line, column };
}

Token Lexer::hexNumber()
{
    size_t start = pos;
    get(); // Consume '0'
    get(); // Consume 'x' or 'X'
    while (isxdigit(peek()))
    {
        get();
    }
    std::string lexeme = input.substr(start, pos - start);
    return Token{ TOK_HEX_LITERAL, lexeme, line, column };
}

Token Lexer::stringLiteral()
{
    get(); // Consume opening quote
    size_t start = pos;
    while (peek() != '"' && peek() != '\0')
    {
        get();
    }
    if (peek() == '\0')
    {
        throw DatabaseException("Unterminated string literal");
    }
    std::string lexeme = input.substr(start, pos - start);
    get(); // Consume closing quote
    return Token{ TOK_STRING_LITERAL, lexeme, line, column };
}

} // namespace lexer

} // namespace db
