#include "Table.hpp"
#include "Column.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

db::Table::Table(std::string name, std::vector<ColumnType> values)
{
    name_ = name;
    for (auto&& field : values)
    {
        columns_.push_back(field);
        columnMap_[field->name()] = field;
        if (field->isKey())
        {
            keyColumn_ = field;
        }
        if (field->isUnique())
        {
            uniquieColumns_.push_back(field);
        }
        if (field->isIndex())
        {
            indexColumns_.push_back(field);
            auto fieldName = field->name();
            indexes_.insert(
                std::make_pair<std::string, Index>(std::move(fieldName), {}));
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

void db::Table::insert(
    std::map<std::string, columns::BaseColumn::value_type> mappedRecord)
{
    Record newRecord{ columns_.size() };
    for (auto&& [name, value] : mappedRecord)
    {
        auto it = columnMap_.find(name);
        if (it == columnMap_.end())
        {
            // Field Validation
            throw TableException("INSERT: Invalid type name: " + name + "!");
        }
        // TODO: Additional checks?
        Record::Row newRow;
        if (std::holds_alternative<bool>(value))
        {
            newRow.type = columns::FieldTypes::Bool;
        }
        else if (std::holds_alternative<int>(value))
        {
            newRow.type = columns::FieldTypes::Integer;
        }
        else if (std::holds_alternative<std::string>(value))
        {
            newRow.type = columns::FieldTypes::String;
        }
        else if (std::holds_alternative<std::vector<char>>(value))
        {
            newRow.type = columns::FieldTypes::Bytes;
        }
        newRow.size = columnMap_[name]->getValueSize();
        newRow.rowData = value;
        newRecord.rows[recordMapping_[name]] = newRow;
    }

    for (auto&& record : records_)
    {
        for (auto&& uniqueField : uniquieColumns_)
        {
            if (record.rows[recordMapping_[uniqueField->name()]].rowData ==
                newRecord.rows[recordMapping_[uniqueField->name()]].rowData)
            {
                throw TableException("INSERT: Constraint unique field: " +
                                     uniqueField->name() + "!");
            }
        }
    }

    // Add to our table data
    records_.push_back(std::move(newRecord));

    // Make indexes
    auto sharedNewRecord =
        std::make_shared<Record>(records_[records_.size() - 1]);

    for (auto&& [key, map] : indexes_)
    {
        auto&& indexVal = sharedNewRecord->rows[recordMapping_[key]].rowData;
        auto ptrCopy = sharedNewRecord;
        map.insert(std::make_pair<columns::BaseColumn::value_type,
                                  std::shared_ptr<Record>>(std::move(indexVal), std::move(ptrCopy)));
    }
};