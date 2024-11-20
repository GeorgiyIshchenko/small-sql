#pragma once

#include "Column.hpp"

#include <any>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace db
{

class Table final
{

    struct Record
    {

        struct Row
        {
            columns::FieldTypes type;
            size_t size;
            columns::BaseColumn::value_type rowData;
        };

        Record(size_t recordListSize)
        {
            rows.resize(recordListSize);
        }

        std::vector<Row> rows = {};
    };

    using ColumnType = std::shared_ptr<columns::BaseColumn>;

    using Index =
        std::multimap<columns::BaseColumn::value_type, std::shared_ptr<Record>>;

public:
    using id_type = int;

public:
    Table(std::string name, std::vector<ColumnType> values);

public:
    std::vector<ColumnType> getColumns() const
    {
        return columns_;
    }

    std::vector<ColumnType> getUniqueColumns() const
    {
        return uniquieColumns_;
    };

    std::vector<ColumnType> getInndexColumns() const
    {
        return indexColumns_;
    };

    ColumnType getKeyField() const
    {
        return keyColumn_;
    };

public:
    void insert(std::map<std::string, columns::BaseColumn::value_type>);



private:
    constexpr static uint64_t id_ = 0;
    std::string name_;

    std::vector<ColumnType> columns_{};
    ColumnType keyColumn_;
    std::vector<ColumnType> uniquieColumns_{};
    std::vector<ColumnType> indexColumns_{};

    // std::string parameter is a name of a field
    std::unordered_map<std::string, ColumnType> columnMap_;
    std::unordered_map<std::string, size_t> recordMapping_;
    std::unordered_map<std::string, Index> indexes_;

    std::vector<Record> records_{};
};

class TableException : public std::exception
{
private:
    std::string message;

public:
    TableException(const std::string& msg)
        : message(msg)
    {
    }

    virtual const char* what() const noexcept override
    {
        return message.c_str();
    }
};

} // namespace db