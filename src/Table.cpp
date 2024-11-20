#include "Table.hpp"
#include "Column.hpp"

#include <algorithm>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

db::Table::Table(const std::string& name, std::vector<ColumnType>& values)
{
    tableName_ = name;
    for (auto&& column : values)
    {
        columns_.push_back(column);
        columnMap_[column->name()] = column;
        if (column->isKey())
        {
            keyColumn_ = column;
        }
        if (column->isUnique())
        {
            uniquieColumns_.push_back(column);
        }
        if (column->isIndex())
        {
            indexColumns_.push_back(column);
            orderedIndexes_[column->name()] = {};
        }
        if (column->isAutoIncrement())
        {
            autoIncrementColumnsMap_[column->name()] = 0;
        }
    }
    if (uniquieColumns_.empty())
    {
        auto&& idField = db::columns::Id();
        auto&& sharedIdField = std::make_shared<columns::BaseColumn>(idField);
        columns_.push_back(sharedIdField);
        uniquieColumns_.push_back(sharedIdField);
        keyColumn_ = sharedIdField;
    }
}

void db::Table::validateInsertion(InsertType& mappedRecord)
{
    Record newRecord{ columns_.size() };
    for (auto&& [name, value] : mappedRecord)
    {
        auto it = columnMap_.find(name);
        if (it == columnMap_.end())
        {
            throw TableException("Insert " + tableName_ +
                                 ": Invalid type name: " + name + "!");
        }
        if (it->second->isAutoIncrement())
        {
            throw TableException("Insert " + tableName_ +
                                 ": Autoincrement column: " + name +
                                 " cannot be inserted!");
        }
        // TODO: Additional checks?
    }
}

db::Table::Record db::Table::createRecord(InsertType& mappedRecord)
{
    Record newRecord{ columns_.size() };
    for (auto&& [name, value] : mappedRecord)
    {
        Record::Row newRow;
        if (std::holds_alternative<bool>(value))
        {
            newRow.type = columns::ColumTypes::Bool;
        }
        else if (std::holds_alternative<int>(value))
        {
            newRow.type = columns::ColumTypes::Integer;
        }
        else if (std::holds_alternative<std::string>(value))
        {
            newRow.type = columns::ColumTypes::String;
        }
        else if (std::holds_alternative<std::vector<char>>(value))
        {
            newRow.type = columns::ColumTypes::Bytes;
        }
        newRow.size = columnMap_[name]->getValueSize();
        newRow.rowData = value;
        newRecord.rows[recordMapping_[name]] = newRow;
    }
    for (auto&& [name, value] : autoIncrementColumnsMap_)
    {
        Record::Row newRow;
        newRow.size = columnMap_[name]->getValueSize();
        newRow.rowData = autoIncrementColumnsMap_[name];
        autoIncrementColumnsMap_[name]++;
        newRecord.rows[recordMapping_[name]] = newRow;
    }
    return newRecord;
}

void db::Table::validateRecord(db::Table::Record& newRecord)
{
    for (auto&& record : records_)
    {
        for (auto&& uniqueField : uniquieColumns_)
        {
            if (record.rows[recordMapping_[uniqueField->name()]].rowData ==
                newRecord.rows[recordMapping_[uniqueField->name()]].rowData)
            {
                throw TableException(
                    "Insert " + tableName_ +
                    ": Constraint unique field: " + uniqueField->name() + "!");
            }
        }
    }
}

void db::Table::createIndexes(std::shared_ptr<Record> sharedRecord)
{
    for (auto&& [key, map] : orderedIndexes_)
    {
        auto&& indexVal = sharedRecord->rows[recordMapping_[key]].rowData;
        auto ptrCopy = sharedRecord;
        map.insert(std::make_pair<columns::BaseColumn::value_type,
                                  std::shared_ptr<Record>>(std::move(indexVal),
                                                           std::move(ptrCopy)));
    }
}

void db::Table::insertImpl(Record newRecord)
{
    // Add to our table data
    records_.push_back(std::move(newRecord));

    // Make indexes
    auto sharedNewRecord =
        std::make_shared<Record>(records_[records_.size() - 1]);

    createIndexes(sharedNewRecord);
}

void db::Table::insert(InsertType mappedRecord)
{
    auto newRecord = createRecord(mappedRecord);

    validateRecord(newRecord);

    insertImpl(std::move(newRecord));
};

db::Table db::Table::select(std::vector<std::string>& column_names,
                            FilterFunction filter)
{
    std::vector<ColumnType> selectedColumns;
    std::for_each(column_names.begin(), column_names.end(),
                  [&selectedColumns, this](auto&& name)
                  {
                      auto it = columnMap_.find(name);
                      if (it == columnMap_.end())
                      {
                          throw TableException("Select " + tableName_ +
                                               ": there is no field " + name +
                                               "!");
                      }
                      selectedColumns.push_back(it->second);
                  });
    Table result{ "ST_" + tableName_, selectedColumns };
    QueryType newRecords = filter(records_);

    for (auto record : newRecords)
    {
        Record newRecord{ selectedColumns.size() };
        for (auto column : selectedColumns)
        {
            Record::Row newRow = record.rows[recordMapping_[column->name()]];
            newRecord.rows[result.recordMapping_[column->name()]] = newRow;
        }
        result.insertImpl(std::move(newRecord));
    }

    return result;
}

void db::Table::del(FilterFunction filter)
{
    QueryType recordsToDelete = filter(records_);


}
