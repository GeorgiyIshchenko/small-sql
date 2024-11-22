#include "Table.hpp"
#include "Column.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
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
        if (column->hasDefault())
        {
            defaultColumns_.push_back(column);
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
    if (mappedRecord.size() != columnMap_.size())
    {
        throw TableException("Insert " + tableName_ +
                             ": Invalid amount of fields: " +
                             std::to_string(mappedRecord.size()) + "/" +
                             std::to_string(columnMap_.size()) + "!");
    }
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

    // Firstly, construct default object

    for (auto&& column : defaultColumns_)
    {
        Record::Row newRow;
        newRow.type = column->getColumnType();
        newRow.size = column->getValueSize();
        newRow.rowData = column->getDefaultValue();
    }

    // Rewrite defaults with existing values

    for (auto&& [name, value] : mappedRecord)
    {
        Record::Row newRow;
        newRow.type = columns::BaseColumn::getValueColumnType(value);
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
    auto sharedNewRecord = std::make_shared<Record>(records_.back());

    createIndexes(sharedNewRecord);
}

void db::Table::insert(InsertType& mappedRecord)
{
    auto newRecord = createRecord(mappedRecord);

    validateRecord(newRecord);

    insertImpl(std::move(newRecord));
};

db::Table::SingleSelectResult
db::Table::select(std::vector<std::string>& column_names,
                  db::Table::Filter& filter)
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

    SingleSelectResult result{};

    if (orderedIndexes_.count(filter.columnName_))
    {
        // TODO
    }
    else
    {
        for (auto&& record : records_)
        {
            if (filter.lowerBound_ <
                    record.rows[recordMapping_[filter.columnName_]].rowData &&
                record.rows[recordMapping_[filter.columnName_]].rowData <=
                    filter.upperBound_)
            {
                result.push_back(std::make_shared<Record>(record));
            }
        }
    }

    return result;
}

void db::Table::del(Filter& filter)
{
    // NOTE: Should delete in filter function for perfomance
    
}

void db::Table::serialize(std::filesystem::path dataFilePath)
{
    std::ofstream outputFile{ dataFilePath.native() };
    if (outputFile.is_open())
    {
        outputFile << columns_.size(); // Number of columns
        for (auto&& column : columns_)
        {
            columns::serialize(outputFile, column);
        }
        outputFile << static_cast<int>(
            columns::ColumType::None); // We dont have zero column type, so zero
                                       // is used as border

        outputFile << records_.size(); // Size of our table
        outputFile.close();
    }
}

void db::Table::deserialize(std::filesystem::path dataFilePath)
{
}
