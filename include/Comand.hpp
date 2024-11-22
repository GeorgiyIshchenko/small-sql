#include "Database.hpp"

#include "Table.hpp"
#include <optional>

namespace db
{

namespace commands
{

class BaseCommand
{

public:
    BaseCommand() = default;

    virtual ~BaseCommand() = default;

public:
    virtual std::optional<Table::Table_ptr> execute();
};

class Insert : BaseCommand
{

public:
    Insert(const std::string& tableName, Table::InsertType valuesMap)
        : tableName_(tableName), valuesMap_(std::move(valuesMap)) {};

    virtual ~Insert() = default;

public:
    virtual std::optional<Table::Table_ptr> execute()
    {
        Database::getInstance().getTables()[tableName_]->insert(valuesMap_);
        return {};
    }

private:
    const std::string tableName_;
    Table::InsertType valuesMap_;
};

class Select : BaseCommand
{

};

} // namespace commands

} // namespace db