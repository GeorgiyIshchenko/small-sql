#pragma once

#include "Table.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace db
{

class Database
{

public:

    using TablesContainer = std::unordered_map<std::string, std::shared_ptr<Table>>;

private:
    Database() = default;

    ~Database() = default;

public:
    Database(const Database& other) = delete;
    Database& operator=(const Database& other) = delete;

    static Database& getInstance(){
        static Database instance;
        return instance;
    }

public:

    TablesContainer getTables() const {
        return tables_;
    }

public:
    void createTable(std::string& name,
                     std::vector<Table::ColumnType> columns);

    void insert(std::string& tableName, Table::InsertType insertMap);

    std::unique_ptr<Table::View> select(std::string& tableName, std::vector<std::string>& selectList, std::unique_ptr<filters::Filter> filter);

    void update(std::string& tableName, std::unique_ptr<filters::Filter> filter, Table::InsertType newValues);

    void del(std::string& tableName, std::unique_ptr<filters::Filter> filter);

    void execute(std::string request);

    void loadTableFromFile(std::string name, std::filesystem::path dataFilePath);

    void storeTableInFile(std::string name, std::filesystem::path dataFilePath);

private:
    TablesContainer tables_;
};

} // namespace db
