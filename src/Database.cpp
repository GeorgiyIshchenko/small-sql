#include "Database.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Table.hpp"
#include <memory>

namespace db
{

void Database::createTable(const std::string& name,
                           std::vector<Table::ColumnType> columns)
{
    tables_[name] = std::make_unique<Table>(name, std::move(columns));
}

void Database::insert(const std::string& tableName, Table::InsertType insertMap)
{
    tables_[tableName]->insert(std::move(insertMap));
}

void Database::execute(const std::string& request)
{
    lexer::Lexer lexer{ request };
    parser::Parser parser{ lexer };
    auto command = parser.parseCommand();
    command->execute();
}

void Database::loadTableFromFile(const std::string& name,
                                 std::filesystem::path dataFilePath)
{
    tables_[name] = std::make_unique<Table>(name);
#ifdef DEBUG
    tables_[name]->deserializeCSV(dataFilePath);
#else
    tables_[name]->deserialize(dataFilePath);
#endif
}

void Database::storeTableInFile(const std::string& name,
                                std::filesystem::path dataFilePath)
{
#ifdef DEBUG
    tables_[name]->serializeCSV(dataFilePath);
#else
    tables_[name]->serialize(dataFilePath);
#endif
}

} // namespace db
