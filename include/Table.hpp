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

        Record(size_t recordListSize)
        {
            rows.resize(recordListSize);
        }

        std::vector<Row> rows = {};
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

public:
    using ColumnType = std::shared_ptr<columns::BaseColumn>;

    using OrderedIndex =
        std::multimap<columns::BaseColumn::value_type, std::shared_ptr<Record>>;

    using InsertType = std::map<std::string, columns::BaseColumn::value_type>;

    using QueryType = std::list<Record>;

    using Table_ptr = std::shared_ptr<Table>;

    using SingleSelectResult = std::vector<std::shared_ptr<Record>>;

public:
    Table(const std::string& name, std::vector<ColumnType>& columns);

    Table(const Table& other) = delete;

    Table(Table&& other) = delete;

public:
    std::list<ColumnType> getColumns() const
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
    void insert(InsertType& insertMap);

    SingleSelectResult select(std::vector<std::string>&, Filter& filter);

    void del(Filter& filter);

private:
    // Helpers
    void insertImpl(Record);
    void validateInsertion(InsertType&);
    Record createRecord(InsertType&);
    void validateRecord(Record& newRecord);
    void createIndexes(std::shared_ptr<Record>);

public:
    void serialize(std::filesystem::path dataFilePath);
    void deserialize(std::filesystem::path dataFilePath);

private:
    std::string tableName_;

    std::list<ColumnType> columns_{};
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