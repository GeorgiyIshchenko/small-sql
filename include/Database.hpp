#pragma once

#include "Table.hpp"

#include <string>
#include <unordered_map>

namespace db
{

class Database
{

public:

    Database() = default;

    void createTable(const std::string& name, std::vector<Table::ColumnType>& columns){
        Table newTable{name, columns};
        tables_[name] = newTable;
    }

    void insert(const std::string& name, ){

    }

private:
    std::unordered_map<std::string, Table> tables_;
};

} // namespace db
