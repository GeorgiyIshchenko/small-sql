#include "Parser.hpp"
#include "Column.hpp"
#include "Lexer.hpp"
#include <memory>
#include <unordered_map>

namespace db
{

namespace parser
{

void Parser::advance()
{
    currentToken_ = lexer_.getNextToken();
}

void Parser::expect(lexer::TokenType type)
{
    if (currentToken_.type != type)
    {
        throw std::runtime_error("Expected token type " + std::to_string(type) +
                                 ", but got " +
                                 std::to_string(currentToken_.type));
    }
    advance();
}

bool Parser::match(lexer::TokenType type)
{
    if (currentToken_.type == type)
    {
        advance();
        return true;
    }
    return false;
}

std::unique_ptr<commands::BaseCommand> Parser::parseCommand()
{
    advance();

    switch (currentToken_.type)
    {
    case lexer::TOK_CREATE:
        return parseCreateTable();
    case lexer::TOK_INSERT:
        return parseInsert();
    case lexer::TOK_SELECT:
        return parseSelect();
    case lexer::TOK_UPDATE:
        return parseUpdate();
    case lexer::TOK_DELETE:
        return parseDelete();
    default:
        throw std::runtime_error("Unknown command: " + currentToken_.lexeme);
    }
}

std::unique_ptr<commands::CreateTable> Parser::parseCreateTable()
{
    expect(lexer::TOK_CREATE);
    expect(lexer::TOK_TABLE);

    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = currentToken_.lexeme;
    advance();

    expect(lexer::TOK_LPAREN);

    std::vector<Table::ColumnType> columns;
    parseColumnDefinitions(columns);

    expect(lexer::TOK_RPAREN);

    auto command = std::make_unique<commands::CreateTable>(tableName, columns);

    return command;
}

void Parser::parseColumnDefinitions(std::vector<Table::ColumnType>& columns)
{
    do
    {
        std::unordered_map<lexem::TokenType, bool> attributes = {
            { lexem::TOK_ATT_AUTOINCREMENT, false },
            { lexem::TOK_ATT_UNIQUE, flase },
            { lexem::TOK_ATT_KEY, false }
        };
        if (match(lexer::TOK_LBRACE))
        {
            while (!match(lexer::TOK_RBRACE))
            {
                expect(lexer::TOK_IDENTIFIER);
                attributes.insert(currentToken_.lexem);
                advance();
                match(lexer::TOK_COMMA); // Handle commas between attributes
            }
        }

        // Column name
        expect(lexer::TOK_IDENTIFIER);
        std::string columnName = currentToken_.lexeme;
        advance();

        expect(lexer::TOK_COLON);

        expect(lexer::TOK_IDENTIFIER);
        lexer::TokenType dataType = currentToken_.type;
        advance();

        int size = 0;
        if (match(lexer::TOK_LBRACKET))
        {
            expect(lexer::TOK_INT_LITERAL);
            size = std::stoi(currentToken_.lexeme);
            advance();
            expect(lexer::TOK_RBRACKET);
        }

        std::string defaultValueString;
        if (match(lexer::TOK_EQUAL))
        {
            expect(lexer::TOK_IDENTIFIER); // Simplify for now
            defaultValueString = currentToken_.lexeme;
            advance();
        }

        // Add to columns vector
        columns::BaseColumn::value_type defaultValue;
        Table::ColumnType column;
        if (dataType == lexer::TOK_INT32)
        {
            defaultValue = stoi(defaultValueString);
            column = std::make_shared<columns::Integer>(columnName);
        }
        else if (dataType == lexer::TOK_STRING)
        {
        }
        else if (dataType == lexer::TOK_BYTES)
        {
        }
        else if (dataType == lexer::TOK_BOOL)
        {
        }
        columns.push_back(std::move(column));

    } while (match(lexer::TOK_COMMA));
}

} // namespace parser

} // namespace db