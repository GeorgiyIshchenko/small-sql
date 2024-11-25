#pragma once

#include "Database.hpp"

#include "Table.hpp"
#include <optional>
#include <vector>

namespace db
{

namespace commands
{

using CommandRetType = std::optional<Table::Table_ptr>;

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
    CreateTable(std::string tableName,
                std::vector<Table::ColumnType> columns)
        : tableName_(std::move(tableName)), columns_(std::move(columns)) {};

    virtual ~CreateTable() = default;

public:
    CommandRetType execute() override
    {
        Database::getInstance().createTable(std::move(tableName_),
                                            std::move(columns_));
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
    Select()
    {
    }

    ~Select() = default;

public:
    CommandRetType execute() override
    {
        return {};
    }
};

class Update final : public BaseCommand
{

public:
    Update()
    {
    }

    ~Update() = default;

public:
    CommandRetType execute() override
    {
        return {};
    }
};

class Delete final : public BaseCommand
{

public:
    Delete()
    {
    }

    ~Delete() = default;

public:
    CommandRetType execute() override
    {
        return {};
    }
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