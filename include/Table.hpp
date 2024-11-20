#pragma once

#include "Column.hpp"

#include <filesystem>
#include <functional>
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

    struct Record
    {

        struct Row
        {
            columns::ColumType type;
            size_t size;
            columns::BaseColumn::value_type rowData;
        };

        Record(size_t recordListSize)
        {
            rows.resize(recordListSize);
        }

        std::vector<Row> rows = {};
    };

public:
    using ColumnType = std::shared_ptr<columns::BaseColumn>;

    using OrderedIndex =
        std::multimap<columns::BaseColumn::value_type, std::shared_ptr<Record>>;

    using InsertType = std::map<std::string, columns::BaseColumn::value_type>;

    using QueryType = std::list<Record>;

    using FilterFunction = std::function<QueryType(QueryType&)>;

public:
    using value_type = columns::BaseColumn::value_type;

public:
    Table(const std::string& name, std::vector<ColumnType>& columns);

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

    Table select(std::vector<std::string>&, FilterFunction& filter);

    void del(FilterFunction& filter);

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