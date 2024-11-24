#pragma once

#include "Command.hpp"
#include "Lexer.hpp"

#include <vector>

namespace db
{

namespace parser
{

class Parser
{
public:
    Parser(lexer::Lexer& lexer);

    std::unique_ptr<commands::BaseCommand> parseCommand();

private:
    lexer::Lexer& lexer_;
    lexer::Token currentToken_;

    void advance();
    void expect(lexer::TokenType type);
    bool match(lexer::TokenType type);

    std::unique_ptr<commands::CreateTable> parseCreateTable();
    std::unique_ptr<commands::Insert> parseInsert();
    std::unique_ptr<commands::Select> parseSelect();
    std::unique_ptr<commands::Update> parseUpdate();
    std::unique_ptr<commands::Delete> parseDelete();
    std::unique_ptr<commands::Join> parseJoin();

    void parseColumnDefinitions(std::vector<Table::ColumnType>& columns);
    void parseAssignments(Table::InsertType& valuesMap);
};

} // namespace parser

} // namespace db
