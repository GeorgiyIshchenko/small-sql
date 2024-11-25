#pragma once

#include "Column.hpp"

#include <filesystem>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace db
{

class Table final
{

public:
    using value_type = columns::BaseColumn::value_type;

public:
    struct Record
    {

        struct Row
        {
            columns::ColumType type;
            size_t size;
            value_type rowData;
        };

        Record(size_t size)
        {
            for(size_t i = 0; i < size; ++i){
                rows.emplace_back(columns::ColumType::None, 0, 0);
            }
        }

        std::vector<Row> rows;
    };

    struct Filter
    {
        std::string columnName_;
        value_type lowerBound_;
        value_type upperBound_;

        Filter(const std::string& columnName, value_type lowerBound,
               value_type upperBound)
            : columnName_(columnName),
              lowerBound_(lowerBound),
              upperBound_(upperBound) {};
    };

    using FilterAndQuery = std::vector<Filter>;

public:
    using ColumnType = std::shared_ptr<columns::BaseColumn>;

    using OrderedIndex =
        std::multimap<columns::BaseColumn::value_type, std::shared_ptr<Record>>;

    using InsertType = std::map<std::string, columns::BaseColumn::value_type>;

    using QueryType = std::list<Record>;

    using Table_ptr = std::shared_ptr<Table>;

    using SingleSelectResult = std::vector<std::shared_ptr<Record>>;

public:
    explicit Table(const std::string& name)
        : tableName_(name) {};

    Table(const std::string& name, std::vector<ColumnType> columns);

    Table(const Table& other) = delete;

    Table(Table&& other) = delete;

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
    void insert(InsertType insertMap);

    SingleSelectResult select(FilterAndQuery filter);

    void del(FilterAndQuery filter);

private:
    // Helpers
    void insertImpl(InsertType mappedRecord);
    void validateInsertion(InsertType&);
    void buildRecord(Record&, InsertType&);
    void validateRecord(std::shared_ptr<Record>);
    void createIndexes(std::shared_ptr<Record>);

public:
    void serialize(std::filesystem::path dataFilePath);
    void deserialize(std::filesystem::path dataFilePath);
    void serializeCSV(std::filesystem::path dataFilePath);
    void deserializeCSV(std::filesystem::path dataFilePath);

private:
    std::string tableName_;

    std::vector<ColumnType> columns_{};
    ColumnType keyColumn_;
    std::vector<ColumnType> uniquieColumns_{};
    std::vector<ColumnType> indexColumns_{};
    std::vector<ColumnType> defaultColumns_{};
    std::unordered_map<std::string, columns::Integer::value_type>
        autoIncrementColumnsMap_{};

    // std::string parameter is a name of a field
    std::unordered_map<std::string, ColumnType> columnMap_;
    std::unordered_map<std::string, size_t> recordMapping_;
    std::unordered_map<std::string, OrderedIndex> orderedIndexes_;

    QueryType records_{};
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