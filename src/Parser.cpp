#include "Parser.hpp"
#include "Column.hpp"
#include "DataBaseException.hpp"
#include "Filter.hpp"
#include "Lexer.hpp"
#include <cctype>
#include <iostream>
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
#if DEBUG
    std::cout << currentToken_.lexeme << " ";
#endif
}

void Parser::expect(lexer::TokenType type)
{
    if (currentToken_.type != type)
    {
        throw DatabaseException("Expected token type " + std::to_string(type) +
                                ", but got " +
                                std::to_string(currentToken_.type) + " (" +
                                currentToken_.lexeme + ")");
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
#ifdef DEBUG
    std::cout << "Parsed: ";
#endif
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
        throw DatabaseException("Unknown command: " + currentToken_.lexeme);
    }
}

std::unique_ptr<commands::CreateTable> Parser::parseCreateTable()
{
    expect(lexer::TOK_CREATE);
    expect(lexer::TOK_TABLE);

    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = previousToken_.lexeme;

    expect(lexer::TOK_LPAREN); // (

    std::vector<Table::ColumnType> columns;
    parseColumnDefinitions(columns);

    expect(lexer::TOK_RPAREN); // )

#ifdef DEBUG
    std::cout << "// Parsing create table " + tableName + " command is ended!"
              << std::endl;
#endif

    return std::make_unique<commands::CreateTable>(tableName, columns);
}

columns::BaseColumn::value_type
Parser::getActualValue(lexer::TokenType dataType, std::string stringVal)
{
    columns::BaseColumn::value_type actualValue;
    if (dataType == lexer::TOK_INT_LITERAL || dataType == lexer::TOK_INT32)
    {
        actualValue = std::stoi(stringVal);
    }
    else if (dataType == lexer::TOK_STRING_LITERAL ||
             dataType == lexer::TOK_STRING)
    {
        actualValue = std::move(stringVal);
    }
    else if (dataType == lexer::TOK_HEX_LITERAL || dataType == lexer::TOK_BYTES)
    {
        columns::Bytes::value_type bytes;
        for (auto&& num : stringVal)
        {
            if (std::isdigit(num))
            {
                bytes.push_back(static_cast<uint8_t>(num));
            }
            else
            {
                bytes.push_back(static_cast<uint8_t>(num));
            }
        }
        actualValue = std::move(bytes);
    }
    else if (dataType == lexer::TOK_TRUE || dataType == lexer::TOK_FALSE ||
             dataType == lexer::TOK_BOOL)
    {
        actualValue = stringVal == "true";
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
                advance();
                attributes[currentToken_.type] = true;
                advance();
                match(lexer::TOK_COMMA);
            }
        }

        expect(lexer::TOK_IDENTIFIER);
        std::string columnName = previousToken_.lexeme;

        expect(lexer::TOK_COLON);

        lexer::TokenType dataType = currentToken_.type;
        advance();

        int maxLen = 0;
        if (match(lexer::TOK_LBRACKET))
        {
            expect(lexer::TOK_INT_LITERAL);
            maxLen = std::stoi(previousToken_.lexeme);
            expect(lexer::TOK_RBRACKET);
        }

        std::string defaultValueString;
        if (match(lexer::TOK_EQUAL))
        {
            advance();
            defaultValueString = currentToken_.lexeme;
        }

        columns::BaseColumn::value_type defaultValue;
        Table::ColumnType column;
        if (dataType == lexer::TOK_INT32)
        {
            column = std::make_shared<columns::Integer>(
                columnName,
                std::get<columns::Integer::value_type>(getActualValue(
                    lexer::TOK_INT32, std::move(defaultValueString))),
                false, attributes[lexer::TOK_ATT_UNIQUE],
                attributes[lexer::TOK_ATT_KEY],
                attributes[lexer::TOK_ATT_UNIQUE]);
        }
        else if (dataType == lexer::TOK_STRING)
        {
            column = std::make_shared<columns::String>(
                columnName, maxLen,
                std::get<columns::String::value_type>(getActualValue(
                    lexer::TOK_STRING, std::move(defaultValueString))),
                false, attributes[lexer::TOK_ATT_UNIQUE],
                attributes[lexer::TOK_ATT_KEY]);
        }
        else if (dataType == lexer::TOK_BYTES)
        {
            column = std::make_shared<columns::Bytes>(
                columnName, maxLen,
                std::get<columns::Bytes::value_type>(getActualValue(
                    lexer::TOK_BYTES, std::move(defaultValueString))),
                false, attributes[lexer::TOK_ATT_UNIQUE],
                attributes[lexer::TOK_ATT_KEY]);
        }
        else if (dataType == lexer::TOK_BOOL)
        {
            column = std::make_shared<columns::Bool>(
                columnName,
                std::get<columns::Bool::value_type>(getActualValue(
                    lexer::TOK_BOOL, std::move(defaultValueString))),
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
    std::string tableName = previousToken_.lexeme;
    advance();

    // Create and return the command object
    auto command =
        std::make_unique<commands::Insert>(tableName, std::move(valuesMap));

#ifdef DEBUG
    std::cout << "// Parsing insert to table " + tableName +
                     " command is ended!"
              << std::endl;
#endif

    return command;
}

void Parser::parseAssignments(Table::InsertType& valuesMap)
{
    do
    {
        // Key
        expect(lexer::TOK_IDENTIFIER);
        std::string key = previousToken_.lexeme;

        expect(lexer::TOK_EQUAL);

        std::string value = currentToken_.lexeme;

        valuesMap[key] = getActualValue(currentToken_.type, std::move(value));

        advance();

    } while (match(lexer::TOK_COMMA));
}

std::unique_ptr<commands::Select> Parser::parseSelect()
{
    expect(lexer::TOK_SELECT);

    // Parse select list
    std::vector<std::string> selectList;
    if (match(lexer::TOK_MULTIPLY))
    {
        // PASS
    }
    else
    {
        do
        {
            expect(lexer::TOK_IDENTIFIER);
            std::string column = previousToken_.lexeme;

            // Handle qualified names
            if (match(lexer::TOK_DOT))
            {
                expect(lexer::TOK_IDENTIFIER);
                column = previousToken_.lexeme; // as we dont have joins
            }

            selectList.push_back(column);

        } while (match(lexer::TOK_COMMA));
    }

    expect(lexer::TOK_FROM);

    // table name
    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = previousToken_.lexeme;

    std::vector<JoinClause> joins;
    while (match(lexer::TOK_JOIN))
    {
        joins.push_back(parseJoinClause());
    }

    // where
    std::unique_ptr<filters::Filter> whereCondition = nullptr;
    if (match(lexer::TOK_WHERE))
    {
        whereCondition = parseWhere();
    }

    // Create and return the command object
    auto command = std::make_unique<commands::Select>(
        tableName, selectList, std::move(whereCondition));

#ifdef DEBUG
    std::cout << "// Parsing select to table " + tableName +
                     " command is ended!"
              << std::endl;
#endif

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
    std::string tableName = previousToken_.lexeme;

    expect(lexer::TOK_SET);

    // assigments
    Table::InsertType assignments;
    parseAssignments(assignments);

    // where
    std::unique_ptr<filters::Filter> whereCondition = nullptr;
    if (match(lexer::TOK_WHERE))
    {
        whereCondition = parseWhere();
    }

    // Create and return the command object
    auto command = std::make_unique<commands::Update>(
        tableName, std::move(whereCondition), assignments);

#ifdef DEBUG
    std::cout << "// Parsing update to table " + tableName +
                     " command is ended!"
              << std::endl;
#endif

    return command;
}

std::unique_ptr<commands::Delete> Parser::parseDelete()
{
    expect(lexer::TOK_DELETE);

    // Table name
    expect(lexer::TOK_IDENTIFIER);
    std::string tableName = previousToken_.lexeme;

    // where
    std::unique_ptr<filters::Filter> whereCondition = nullptr;
    if (match(lexer::TOK_WHERE))
    {
        whereCondition = parseWhere();
    }

    // Create and return the command object
    auto command = std::make_unique<commands::Delete>(
        tableName, std::move(whereCondition));

#ifdef DEBUG
    std::cout << "// Parsing delete to table " + tableName +
                     " command is ended!"
              << std::endl;
#endif

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

std::unique_ptr<expression::Expression> Parser::parseEqualityExpression()
{
    auto left = parseRelationalExpression();

    while (match(lexer::TOK_EQUAL) || match(lexer::TOK_NOT_EQUAL))
    {
        lexer::Token op = previousToken_;
        auto right = parseRelationalExpression();
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
    else if (match(lexer::TOK_DOT))
    {
        expect(lexer::TOK_IDENTIFIER);
        return std::make_unique<expression::IdentifierExpression>(
            previousToken_.lexeme);
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
        throw DatabaseException("Unexpected token in expression at line " +
                                std::to_string(currentToken_.line));
    }
}

std::unique_ptr<filters::Filter> Parser::parseWhere()
{
    return parseOrFilter();
}

std::unique_ptr<filters::Filter> Parser::parseOrFilter()
{
    auto left = parseAndFilter();

    while (match(lexer::TOK_OR))
    {
        auto right = parseAndFilter();
        left = std::make_unique<filters::LogicalFilter>(
            filters::LogicalFilter::OR, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<filters::Filter> Parser::parseAndFilter()
{
    auto left = parseNotFilter();

    while (match(lexer::TOK_AND))
    {
        auto right = parseNotFilter();
        left = std::make_unique<filters::LogicalFilter>(
            filters::LogicalFilter::AND, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<filters::Filter> Parser::parseNotFilter()
{
    if (match(lexer::TOK_NOT))
    {
        auto operand = parseNotFilter();
        return std::make_unique<filters::NotFilter>(std::move(operand));
    }
    else
    {
        return parseComparisonFilter();
    }
}

std::unique_ptr<filters::Filter> Parser::parseComparisonFilter()
{
    expect(lexer::TOK_IDENTIFIER);
    std::string fieldName = previousToken_.lexeme;

    filters::ComparisonFilter::Operator op;

    if (match(lexer::TOK_EQUAL))
    {
        op = filters::ComparisonFilter::EQUAL;
    }
    else if (match(lexer::TOK_NOT_EQUAL))
    {
        op = filters::ComparisonFilter::NOT_EQUAL;
    }
    else if (match(lexer::TOK_LESS))
    {
        op = filters::ComparisonFilter::LESS_THAN;
    }
    else if (match(lexer::TOK_LESS_EQUAL))
    {
        op = filters::ComparisonFilter::LESS_THAN_OR_EQUAL;
    }
    else if (match(lexer::TOK_GREATER))
    {
        op = filters::ComparisonFilter::GREATER_THAN;
    }
    else if (match(lexer::TOK_GREATER_EQUAL))
    {
        op = filters::ComparisonFilter::GREATER_THAN_OR_EQUAL;
    }
    else
    {
        throw DatabaseException("Invalid WHERE operator");
    }

    auto value = parseCondition();

    return std::make_unique<filters::ComparisonFilter>(fieldName, op,
                                                       value->evaluate({}));
}

} // namespace parser

} // namespace db