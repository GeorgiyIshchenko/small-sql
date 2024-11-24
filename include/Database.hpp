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
    void createTable(const std::string& name,
                     std::vector<Table::ColumnType> columns);

    void execute(const std::string& request);

private:
    TablesContainer tables_;
};

} // namespace db
