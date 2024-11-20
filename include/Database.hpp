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

    void createTable(const std::string& name,
                     std::vector<Table::ColumnType>& columns)
    {
        Table newTable{ name, columns };
        tables_[name] = newTable;
    }

    void insert(const std::string& name, Table::InsertType& insertMap)
    {
        tables_[name].insert(insertMap);
    }

    Table select(const std::string& name, std::vector<std::string>& colum_names, Table::FilterFunction& filter){
        return tables_[name].select(colum_names, filter);
    }

    void del(const std::string& name, Table::FilterFunction& filter){
        tables_[name].del(filter);
    }

private:
    std::unordered_map<std::string, Table> tables_;
};

} // namespace db
