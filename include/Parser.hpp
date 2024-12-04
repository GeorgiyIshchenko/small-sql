#pragma once

#include "Column.hpp"
#include "Command.hpp"
#include "Expression.hpp"
#include "Lexer.hpp"
#include "Filter.hpp"

#include <memory>
#include <vector>

namespace db
{

namespace parser
{

class Parser
{

public:
    struct JoinClause
    {
        std::string tableName;
        std::unique_ptr<expression::Expression> onCondition;
    };

public:
    Parser(lexer::Lexer& lexer): lexer_(lexer) {};

    std::unique_ptr<commands::BaseCommand> parseCommand();

private:
    lexer::Lexer& lexer_;
    lexer::Token currentToken_;
    lexer::Token previousToken_;

    void advance();
    void expect(lexer::TokenType type);
    bool match(lexer::TokenType type);

    columns::BaseColumn::value_type getActualValue(lexer::TokenType type,
                                                   std::string stringVal);
    std::unique_ptr<commands::CreateTable> parseCreateTable();
    std::unique_ptr<commands::Insert> parseInsert();
    std::unique_ptr<commands::Select> parseSelect();
    std::unique_ptr<commands::Update> parseUpdate();
    std::unique_ptr<commands::Delete> parseDelete();
    std::unique_ptr<commands::Join> parseJoin();

    JoinClause parseJoinClause();

    std::unique_ptr<expression::Expression> parseExpression();
    std::unique_ptr<expression::Expression> parseCondition();
    std::unique_ptr<expression::Expression> parseLogicalOrExpression();
    std::unique_ptr<expression::Expression> parseLogicalAndExpression();
    std::unique_ptr<expression::Expression> parseEqualityExpression();
    std::unique_ptr<expression::Expression> parseRelationalExpression();
    std::unique_ptr<expression::Expression> parseAdditiveExpression();
    std::unique_ptr<expression::Expression> parseMultiplicativeExpression();
    std::unique_ptr<expression::Expression> parseUnaryExpression();
    std::unique_ptr<expression::Expression> parsePrimaryExpression();

    std::unique_ptr<filters::Filter> parseWhere();
    std::unique_ptr<filters::Filter> parseOrFilter();
    std::unique_ptr<filters::Filter> parseAndFilter();
    std::unique_ptr<filters::Filter> parseNotFilter();
    std::unique_ptr<filters::Filter> parseComparisonFilter();


    void parseColumnDefinitions(std::vector<Table::ColumnType>& columns);
    void parseAssignments(Table::InsertType& valuesMap);
};

} // namespace parser

} // namespace db
