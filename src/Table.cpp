#include "Table.hpp"
#include "Column.hpp"
#include "Helpers.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

db::Table::Table(const std::string& name, std::vector<ColumnType> values)
{
    tableName_ = name;
    size_t fieldIdx = 0;
    for (auto&& column : values)
    {
        columns_.push_back(column);
        columnMap_[column->name()] = column;
        recordMapping_[column->name()] = fieldIdx++;
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
        auto&& sharedIdField = std::make_shared<columns::Id>(idField);
        columns_.push_back(sharedIdField);
        columnMap_[idField.name()] = sharedIdField;
        recordMapping_[idField.name()] = fieldIdx++;
        autoIncrementColumnsMap_[idField.name()] = 0;
        uniquieColumns_.push_back(sharedIdField);
        keyColumn_ = sharedIdField;
    }
}

void db::Table::validateInsertion(InsertType& mappedRecord)
{
    if (mappedRecord.size() > columnMap_.size())
    {
        throw TableException("Insert " + tableName_ +
                             ": Invalid amount of fields: " +
                             std::to_string(mappedRecord.size()) + "/" +
                             std::to_string(columnMap_.size()) + "!");
    }
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

void db::Table::buildRecord(Record& newRecord, InsertType& mappedRecord)
{

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
        newRow.rowData = value;
        newRow.type = columns::BaseColumn::getValueColumnType(value);
        autoIncrementColumnsMap_[name]++;
        newRecord.rows[recordMapping_[name]] = newRow;
    }
}

void db::Table::validateRecord(std::shared_ptr<Record> newRecord)
{
    for (auto&& record : records_)
    {
        for (auto&& uniqueField : uniquieColumns_)
        {
            if (record.rows[recordMapping_[uniqueField->name()]].rowData ==
                newRecord->rows[recordMapping_[uniqueField->name()]].rowData)
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

void db::Table::insertImpl(InsertType mappedRecord)
{

    Record newRecord{ mappedRecord.size() + autoIncrementColumnsMap_.size() };

    buildRecord(newRecord, mappedRecord);

    auto shared = std::make_shared<Record>(newRecord);

    validateRecord(shared);

    // Add to our table data
    records_.push_back(newRecord);

    // Make indexes
    //createIndexes(shared);
}

void db::Table::insert(InsertType mappedRecord)
{
#if DEBUG
    std::cout << "New iserion: to table " + tableName_ + ": ";
    for (auto&& [key, val] : mappedRecord)
    {
        std::cout << "{" << key << ", ";
        if (std::holds_alternative<int>(val))
        {
            std::cout << "(int) " << std::to_string(std::get<int>(val));
        }
        else if (std::holds_alternative<bool>(val))
        {
            std::cout << "(bool) " << (std::get<bool>(val) ? "1" : "0");
        }
        else if (std::holds_alternative<std::string>(val))
        {
            std::cout << "(string) "
                      << escapeCSVField(std::get<std::string>(val));
        }
        std::cout << "} ";
    }
    std::cout << std::endl;
#endif

    validateInsertion(mappedRecord);

    insertImpl(mappedRecord);
};

db::Table::SingleSelectResult db::Table::select(FilterAndQuery filters)
{
    std::vector<ColumnType> selectedColumns;
    std::for_each(filters.begin(), filters.end(),
                  [&selectedColumns, this](auto&& filter)
                  {
                      auto it = columnMap_.find(filter.columnName_);
                      if (it == columnMap_.end())
                      {
                          throw TableException("Select " + tableName_ +
                                               ": there is no field " +
                                               filter.columnName_ + "!");
                      }
                      selectedColumns.push_back(it->second);
                  });

    SingleSelectResult result{};

    // if (orderedIndexes_.count(filter.columnName_))
    // {
    //     // TODO
    // }
    // else
    // {
    //     for (auto&& record : records_)
    //     {
    //         if (filter.lowerBound_ <
    //                 record.rows[recordMapping_[filter.columnName_]].rowData
    //                 &&
    //             record.rows[recordMapping_[filter.columnName_]].rowData <=
    //                 filter.upperBound_)
    //         {
    //             result.push_back(std::make_shared<Record>(record));
    //         }
    //     }
    // }

    return result;
}

void db::Table::del(FilterAndQuery filters)
{
    // NOTE: Should delete in filter function for perfomance
    (void)filters;
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
    (void)dataFilePath;
}

void db::Table::serializeCSV(std::filesystem::path dataFilePath)
{
    std::ofstream file(dataFilePath);
    if (!file.is_open())
    {
        throw TableException("Failed to open file for writing: " +
                             dataFilePath.string());
    }

    // table name

    file << "#TABLE_NAME" << std::endl;
    file << tableName_ << std::endl;

    // columns separator
    file << "#COLUMNS" << std::endl;

    // columns
    for (const auto& column : columns_)
    {
        columns::serializeCSV(file, column);
    }

    // data separator
    file << "#DATA" << std::endl;

    // header for me
    std::stringstream headerStream;
    for (const auto& column : columns_)
    {
        headerStream << escapeCSVField(column->name()) << ",";
    }
    std::string headerLine = headerStream.str();
    // remove comma
    if (!headerLine.empty())
    {
        headerLine.pop_back();
    }
    file << headerLine << std::endl;

    // records
    for (const auto& record : records_)
    {
        for (const auto& row : record.rows)
        {
            std::string valueStr;
            if (row.type == columns::ColumType::Bytes)
            {
                // bytes to hex
                auto& bytes = std::get<columns::Bytes::value_type>(row.rowData);
                valueStr = std::string(bytes.begin(), bytes.end());
                valueStr = escapeCSVField(valueStr);
            }
            else if (row.type == columns::ColumType::String)
            {
                valueStr = escapeCSVField(
                    std::get<columns::String::value_type>(row.rowData));
            }
            else if (row.type == columns::ColumType::Integer)
            {
                valueStr = std::to_string(
                    std::get<columns::Integer::value_type>(row.rowData));
            }
            else if (row.type == columns::ColumType::Bool)
            {
                valueStr = (std::get<columns::Bool::value_type>(row.rowData)
                                ? "true"
                                : "false");
            }
            else if (row.type == columns::ColumType::Id)
            {
                valueStr = std::to_string(
                    std::get<columns::Integer::value_type>(row.rowData));
            }
            file << valueStr << ",";
        }
        file << std::endl;
    }

    file.close();
}

void db::Table::deserializeCSV(std::filesystem::path dataFilePath)
{
    std::ifstream file(dataFilePath);
    if (!file.is_open())
    {
        throw TableException("Failed to open file for reading: " +
                             dataFilePath.string());
    }

    columns_.clear();
    columnMap_.clear();
    records_.clear();
    recordMapping_.clear();

    std::string line;

    // tableName
    std::getline(file, line);
    if (line != "#TABLE_NAME")
    {
        throw TableException("CSV doesnt have table name separator.");
    }
    std::getline(file, line);
    tableName_ = line;

    // columns
    std::getline(file, line);
    if (line != "#COLUMNS")
    {
        throw TableException("CSV doesnt have columns separator.");
    }
    while (std::getline(file, line))
    {
        if (line == "#DATA")
        {
            break;
        }
        std::istringstream iss(line);
        std::string columnLine = line;
        // Use columns::deserializeCSV to read column definition
        std::istringstream columnStream(columnLine);
        auto column = columns::deserializeCSV(columnStream);
        if (column)
        {
            columns_.push_back(column);
            columnMap_[column->name()] = column;
        }
        else
        {
            throw TableException("Failed to deserialize column from CSV.");
        }
    }

    // header
    if (!std::getline(file, line))
    {
        throw TableException("No header row found in CSV data.");
    }
    std::vector<std::string> columnNames = parseCSVLine(line);
    if (columnNames.size() != columns_.size())
    {
        throw TableException(
            "Mismatch between number of columns and header fields.");
    }

    // record mapping
    for (size_t i = 0; i < columnNames.size(); ++i)
    {
        recordMapping_[columnNames[i]] = i;
    }

    // records
    while (std::getline(file, line))
    {
        std::vector<std::string> fieldValues = parseCSVLine(line);
        if (fieldValues.size() != columns_.size())
        {
            throw TableException(
                "Mismatch between number of columns and data fields.");
        }

        Record record(columns_.size() + autoIncrementColumnsMap_.size());

        for (size_t i = 0; i < fieldValues.size(); ++i)
        {
            auto& column = columns_[i];
            Record::Row row;
            row.type = column->getColumnType();
            row.size = column->getValueSize();

            std::string valueStr = fieldValues[i];
            columns::ColumType colType = column->getColumnType();

            if (colType == columns::ColumType::Integer ||
                colType == columns::ColumType::Id)
            {
                int value = std::stoi(valueStr);
                row.rowData = value;
            }
            else if (colType == columns::ColumType::Bool)
            {
                bool value = valueStr == "true";
                row.rowData = value;
            }
            else if (colType == columns::ColumType::String)
            {
                row.rowData = valueStr;
            }
            else if (colType == columns::ColumType::Bytes)
            {
                std::vector<uint8_t> bytes(valueStr.begin(), valueStr.end());
                row.rowData = bytes;
            }
            else
            {
                throw TableException(
                    "Unknown column type during deserialization.");
            }

            record.rows[i] = row;
        }

        records_.push_back(record);
    }

    file.close();
}
