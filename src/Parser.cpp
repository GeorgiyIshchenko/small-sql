#include "Parser.hpp"
#include "Column.hpp"
#include "Lexer.hpp"
#include <cctype>
#include <memory>
#include <unordered_map>

namespace db
{

namespace parser
{

void Parser::advance()
{
    previousToken_ = currentToken_;
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

std::unique_ptr<expression::Expression> Parser::parseCondition()
{
    return parseExpression();
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

columns::BaseColumn::value_type getActualValue(lexer::TokenType dataType,
                                               std::string stringVal)
{
    columns::BaseColumn::value_type actualValue;
    if (dataType == lexer::TOK_INT32)
    {
        actualValue = stoi(stringVal);
    }
    else if (dataType == lexer::TOK_STRING)
    {
        actualValue = std::move(stringVal);
    }
    else if (dataType == lexer::TOK_BYTES)
    {
        columns::Bytes::value_type bytes;
        for (auto&& num : stringVal)
        {
            if (std::isdigit(num))
            {
                bytes.push_back(std::atoi(&num) - 30);
            }
            else
            {
                bytes.push_back(std::atoi(&num) - 41);
            }
        }
        actualValue = std::move(bytes);
    }
    else if (dataType == lexer::TOK_BOOL)
    {
        actualValue = stringVal == "true" ? true : false;
    }
    return actualValue;
}

void Parser::parseColumnDefinitions(std::vector<Table::ColumnType>& columns)
{
    do
    {
        std::unordered_map<lexer::TokenType, bool> attributes = {
            { lexer::TOK_ATT_AUTOINCREMENT, false },
            { lexer::TOK_ATT_UNIQUE, false },
            { lexer::TOK_ATT_KEY, false }
        };
        if (match(lexer::TOK_LBRACE))
        {
            while (!match(lexer::TOK_RBRACE))
            {
                expect(lexer::TOK_IDENTIFIER);
                attributes[currentToken_.type] = true;
                advance();
                match(lexer::TOK_COMMA);
            }
        }

        expect(lexer::TOK_IDENTIFIER);
        std::string columnName = currentToken_.lexeme;
        advance();

        expect(lexer::TOK_COLON);

        expect(lexer::TOK_IDENTIFIER);
        lexer::TokenType dataType = currentToken_.type;
        advance();

        int maxLen = 0;
        if (match(lexer::TOK_LBRACKET))
        {
            expect(lexer::TOK_INT_LITERAL);
            maxLen = std::stoi(currentToken_.lexeme);
            advance();
            expect(lexer::TOK_RBRACKET);
        }

        std::string defaultValueString;
        if (match(lexer::TOK_EQUAL))
        {
            expect(lexer::TOK_IDENTIFIER);
            defaultValueString = currentToken_.lexeme;
            advance();
        }

        columns::BaseColumn::value_type defaultValue;
        Table::ColumnType column;
        if (dataType == lexer::TOK_INT32)
        {
            column = std::make_shared<columns::Integer>(
                columnName,
                std::get<columns::Integer::value_type>(getActualValue(lexer::TOK_INT32, std::move(defaultValueString))),
                false, attributes[lexer::TOK_ATT_UNIQUE],
                attributes[lexer::TOK_ATT_KEY],
                attributes[lexer::TOK_ATT_UNIQUE]);
        }
        else if (dataType == lexer::TOK_STRING)
        {
            column = std::make_shared<columns::String>(
                columnName, maxLen,
                std::get<columns::String::value_type>(getActualValue(lexer::TOK_STRING,
                               std::move(defaultValueString))),
                false, attributes[lexer::TOK_ATT_UNIQUE],
                attributes[lexer::TOK_ATT_KEY]);
        }
        else if (dataType == lexer::TOK_BYTES)
        {
            column = std::make_shared<columns::Bytes>(
                columnName, maxLen,
                std::get<columns::Bytes::value_type>(getActualValue(lexer::TOK_BYTES, std::move(defaultValueString))),
                false, attributes[lexer::TOK_ATT_UNIQUE],
                attributes[lexer::TOK_ATT_KEY]);
        }
        else if (dataType == lexer::TOK_BOOL)
        {
            column = std::make_shared<columns::Bool>(
                columnName,
                std::get<columns::Bool::value_type>(getActualValue(lexer::TOK_BOOL, std::move(defaultValueString))),
                false, attributes[lexer::TOK_ATT_UNIQUE],
                attributes[lexer::TOK_ATT_KEY]);
        }
        columns.push_back(std::move(column));
    } while (match(lexer::TOK_COMMA));
}

std::unique_ptr<commands::Insert> Parser::parseInsert()
{
    expect(lexer::TOK_INSERT);
    expect(lexer::TOK_LPAREN);

    // Parse assignments
    Table::InsertType valuesMap;
    parseAssignments(valuesMap);

    expect(lexer::TOK_RPAREN);
    expect(lexer::TOK_TO);

    // Table name
    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = currentToken_.lexeme;
    advance();

    // Create and return the command object
    auto command =
        std::make_unique<commands::Insert>(tableName, std::move(valuesMap));
    return command;
}

void Parser::parseAssignments(Table::InsertType& valuesMap)
{
    do
    {
        // Key
        expect(lexer::TOK_IDENTIFIER);
        std::string key = currentToken_.lexeme;
        advance();

        expect(lexer::TOK_EQUAL);

        expect(lexer::TOK_STRING_LITERAL);
        std::string value = currentToken_.lexeme;
        advance();

        valuesMap[key] = getActualValue(currentToken_.type, std::move(value));

    } while (match(lexer::TOK_COMMA));
}

std::unique_ptr<commands::Select> Parser::parseSelect()
{
    expect(lexer::TOK_SELECT);

    // Parse select list
    std::vector<std::string> selectList;
    if (match(lexer::TOK_MULTIPLY))
    {
        // SELECT *
        selectList.push_back("*");
    }
    else
    {
        do
        {
            expect(lexer::TOK_IDENTIFIER);
            std::string column = currentToken_.lexeme;
            advance();

            // Handle qualified names
            if (match(lexer::TOK_DOT))
            {
                expect(lexer::TOK_IDENTIFIER);
                column += "." + currentToken_.lexeme;
                advance();
            }

            selectList.push_back(column);

        } while (match(lexer::TOK_COMMA));
    }

    expect(lexer::TOK_FROM);

    // table name
    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = previousToken_.lexeme;

    // TODO: join, possible interface:
    std::vector<JoinClause> joins;
    while (match(lexer::TOK_JOIN)) {
        joins.push_back(parseJoinClause());
    }

    // where
    std::unique_ptr<expression::Expression> whereCondition = nullptr;
    if (match(lexer::TOK_WHERE))
    {
        whereCondition = parseCondition();
    }

    // Create and return the command object
    auto command = std::make_unique<commands::Select>();
    // TODO: command->joins = std::move(joins);
    // command->tableName = tableName;
    // command->selectList = selectList;
    // command->whereCondition = std::move(whereCondition);

    return command;

    return command;
}

Parser::JoinClause Parser::parseJoinClause()
{
    expect(lexer::TOK_JOIN);

    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = previousToken_.lexeme;

    expect(lexer::TOK_ON);

    auto onCondition = parseCondition();

    return { tableName, std::move(onCondition) };
}

std::unique_ptr<commands::Update> Parser::parseUpdate()
{
    expect(lexer::TOK_UPDATE);

    // table name
    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = currentToken_.lexeme;
    advance();

    expect(lexer::TOK_SET);

    // assigments
    Table::InsertType assignments;
    parseAssignments(assignments);

    // where
    // Implement parseCondition() if needed

    // Create and return the command object
    auto command = std::make_unique<commands::Update>();
    // init command!!
    // command->tableName = tableName;
    // command->assignments = assignments;

    return command;
}

std::unique_ptr<commands::Delete> Parser::parseDelete()
{
    expect(lexer::TOK_DELETE);

    // Table name
    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = currentToken_.lexeme;
    advance();

    // where
    std::unique_ptr<expression::Expression> whereCondition = nullptr;
    if (match(lexer::TOK_WHERE))
    {
        whereCondition = parseCondition();
    }

    // Create and return the command object
    auto command = std::make_unique<commands::Delete>();
    // init command!!
    // command->tableName = tableName;
    // command->whereCondition = std::move(whereCondition);

    return command;
}

std::unique_ptr<expression::Expression> Parser::parseExpression()
{
    return parseLogicalOrExpression();
}

std::unique_ptr<expression::Expression> Parser::parseLogicalOrExpression()
{
    auto left = parseLogicalAndExpression();

    while (match(lexer::TOK_OR))
    {
        lexer::Token op = previousToken_;
        auto right = parseLogicalAndExpression();
        left = std::make_unique<expression::BinaryExpression>(
            op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<expression::Expression> Parser::parseLogicalAndExpression()
{
    auto left = parseEqualityExpression();

    while (match(lexer::TOK_AND))
    {
        lexer::Token op = previousToken_;
        auto right = parseEqualityExpression();
        left = std::make_unique<expression::BinaryExpression>(
            op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<expression::Expression> Parser::parseRelationalExpression()
{
    auto left = parseAdditiveExpression();

    while (match(lexer::TOK_LESS) || match(lexer::TOK_LESS_EQUAL) ||
           match(lexer::TOK_GREATER) || match(lexer::TOK_GREATER_EQUAL))
    {
        lexer::Token op = previousToken_;
        auto right = parseAdditiveExpression();
        left = std::make_unique<expression::BinaryExpression>(
            op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<expression::Expression> Parser::parseAdditiveExpression()
{
    auto left = parseMultiplicativeExpression();

    while (match(lexer::TOK_PLUS) || match(lexer::TOK_MINUS))
    {
        lexer::Token op = previousToken_;
        auto right = parseMultiplicativeExpression();
        left = std::make_unique<expression::BinaryExpression>(
            op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<expression::Expression> Parser::parseMultiplicativeExpression()
{
    auto left = parseUnaryExpression();

    while (match(lexer::TOK_MULTIPLY) || match(lexer::TOK_DIVIDE) ||
           match(lexer::TOK_MODULO))
    {
        lexer::Token op = previousToken_;
        auto right = parseUnaryExpression();
        left = std::make_unique<expression::BinaryExpression>(
            op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<expression::Expression> Parser::parseUnaryExpression()
{
    if (match(lexer::TOK_NOT))
    {
        lexer::Token op = previousToken_;
        auto operand = parseUnaryExpression();
        return std::make_unique<expression::UnaryExpression>(
            op, std::move(operand));
    }
    else
    {
        return parsePrimaryExpression();
    }
}

std::unique_ptr<expression::Expression> Parser::parsePrimaryExpression()
{
    if (match(lexer::TOK_LPAREN))
    {
        auto expr = parseExpression();
        expect(lexer::TOK_RPAREN);
        return expr;
    }
    else if (match(lexer::TOK_BITWISE_OR))
    {
        // string length notation |identifier|
        expect(lexer::TOK_IDENTIFIER);
        std::string identifier = previousToken_.lexeme;
        expect(lexer::TOK_BITWISE_OR);
        return std::make_unique<expression::StringLengthExpression>(identifier);
    }
    else if (match(lexer::TOK_IDENTIFIER))
    {
        // column name
        std::string name = previousToken_.lexeme;

        if (match(lexer::TOK_DOT))
        {
            expect(lexer::TOK_IDENTIFIER);
            name += "." + previousToken_.lexeme;
        }

        return std::make_unique<expression::IdentifierExpression>(name);
    }
    else if (match(lexer::TOK_INT_LITERAL) ||
             match(lexer::TOK_STRING_LITERAL) || match(lexer::TOK_HEX_LITERAL))
    {
        return std::make_unique<expression::LiteralExpression>(previousToken_);
    }
    else if (match(lexer::TOK_TRUE) || match(lexer::TOK_FALSE))
    {
        return std::make_unique<expression::LiteralExpression>(previousToken_);
    }
    else
    {
        throw std::runtime_error("Unexpected token in expression at line " +
                                 std::to_string(currentToken_.line));
    }
}

} // namespace parser

} // namespace db