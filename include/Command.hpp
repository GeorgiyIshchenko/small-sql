#pragma once

#include "Database.hpp"
#include "Filter.hpp"

#include "Table.hpp"
#include <memory>
#include <optional>
#include <vector>

namespace db
{

namespace commands
{

using CommandRetType = std::optional<std::unique_ptr<Table::View>>;

class BaseCommand
{

public:
    BaseCommand() = default;

    virtual ~BaseCommand() = default;

public:
    virtual CommandRetType execute() = 0;
};

class CreateTable final : public BaseCommand
{

public:
    CreateTable(std::string tableName, std::vector<Table::ColumnType> columns)
        : tableName_(std::move(tableName)), columns_(std::move(columns)) {};

    virtual ~CreateTable() = default;

public:
    CommandRetType execute() override
    {
        Database::getInstance().createTable(tableName_, std::move(columns_));
        return {};
    }

private:
    std::string tableName_;
    std::vector<Table::ColumnType> columns_;
};

class Insert final : public BaseCommand
{

public:
    Insert(std::string tableName, Table::InsertType valuesMap)
        : tableName_(std::move(tableName)), valuesMap_(std::move(valuesMap)) {};

    ~Insert() = default;

public:
    CommandRetType execute() override
    {
        Database::getInstance().getTables()[tableName_]->insert(valuesMap_);
        return {};
    }

private:
    std::string tableName_;
    Table::InsertType valuesMap_;
};

class Select final : public BaseCommand
{

public:
    Select(std::string tableName, std::vector<std::string>& selectList, std::unique_ptr<filters::Filter> filter)
        : tableName_(tableName), selectList_(selectList), filter_(std::move(filter))
    {
    }

    ~Select() = default;

public:
    CommandRetType execute() override
    {
        auto view = Database::getInstance().select(tableName_, selectList_, std::move(filter_));
        view->print();
        return view;
    }

private:
    std::string tableName_;
    std::vector<std::string> selectList_;
    std::unique_ptr<filters::Filter> filter_;
};

class Update final : public BaseCommand
{

public:
    Update(std::string tableName, std::unique_ptr<filters::Filter> filter,
           Table::InsertType newValues)
        : tableName_(tableName),
          filter_(std::move(filter)),
          newValues_(std::move(newValues))
    {
    }

    ~Update() = default;

public:
    CommandRetType execute() override
    {
        Database::getInstance().update(tableName_, std::move(filter_), newValues_);
        return {};
    }

private:
    std::string tableName_;
    std::unique_ptr<filters::Filter> filter_;
    Table::InsertType newValues_;
};

class Delete final : public BaseCommand
{

public:
    Delete(std::string tableName, std::unique_ptr<filters::Filter> filter)
        : tableName_(tableName), filter_(std::move(filter))
    {
    }

    ~Delete() = default;

public:
    CommandRetType execute() override
    {
        Database::getInstance().del(tableName_, std::move(filter_));
        return {};
    }

private:
    std::string tableName_;
    std::unique_ptr<filters::Filter> filter_;
};

class Join final : public BaseCommand
{

public:
    Join()
    {
    }

    ~Join() = default;

public:
    CommandRetType execute() override
    {
        return {};
    }
};

enum class CommandId : char
{
    CreateTable,
    Insert,
    Select,
    Delete,
    Update,
    Join,
};

} // namespace commands

} // namespace db