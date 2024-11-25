#include "Database.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Table.hpp"
#include <memory>

namespace db
{

void Database::createTable(std::string name,
                           std::vector<Table::ColumnType> columns)
{
#ifdef DEBUG
    std::cout << "Creating table: " + name << std::endl;
#endif
    tables_[name] = std::make_unique<Table>(name, std::move(columns));
#ifdef DEBUG
    std::cout << "Successfully created table: " + name << std::endl;
#endif
}

void Database::insert(std::string tableName, Table::InsertType insertMap)
{
#ifdef DEBUG
    std::cout << "Inserting data to table: " + tableName << std::endl;
#endif
    tables_[tableName]->insert(std::move(insertMap));
#ifdef DEBUG
    std::cout << "Successfully inserted data to table: " + tableName << std::endl;
#endif
}

void Database::execute(std::string request)
{
    lexer::Lexer lexer{ request };
    parser::Parser parser{ lexer };
    auto command = parser.parseCommand();
    command->execute();
}

void Database::loadTableFromFile(std::string name,
                                 std::filesystem::path dataFilePath)
{
    tables_[name] = std::make_unique<Table>(name);
#ifdef DEBUG
    tables_[name]->deserializeCSV(dataFilePath);
#else
    tables_[name]->deserialize(dataFilePath);
#endif
}

void Database::storeTableInFile(std::string name,
                                std::filesystem::path dataFilePath)
{
#ifdef DEBUG
    tables_[name]->serializeCSV(dataFilePath);
#else
    tables_[name]->serialize(dataFilePath);
#endif
}

} // namespace db
