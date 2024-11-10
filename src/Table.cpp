#include "Table.hpp"
#include "Field.hpp"

#include <memory>
#include <string>
#include <variant>
#include <unordered_map>
#include <vector>

db::Table::Table(std::string name,
                 std::vector<FieldType> fields)
{
    name_ = name;
    for (auto&& field : fields)
    {
        fields_.push_back(field);
        fieldMap_[field->name()] = field;
        if (field->isKey())
        {
            keyField_ = field;
        }
        if (field->isUnique())
        {
            uniquieFields_.push_back(field);
        }
    }
    if (uniquieFields_.empty())
    {
        auto&& idField = db::fieldset::Id();
        auto&& sharedIdField = std::make_shared<fieldset::BaseField>(idField);
        fields_.push_back(sharedIdField);
        uniquieFields_.push_back(sharedIdField);
        keyField_ = sharedIdField;
    }
}

void db::Table::insert(std::map<std::string, fieldset::BaseField::value_type> mappedRecord)
{
    Record newRecord{fields_.size()};
    for(auto&& [name, value] : mappedRecord){
        auto it = fieldMap_.find(name);
        if(it == fieldMap_.end()) {
            // Field Validation
            throw TableException("INSERT: Invalid type name!");
        }
        Record::Row newRow;
        if (std::holds_alternative<bool>(value)){
            newRow.type = fieldset::FieldTypes::Bool;
        }
        else if (std::holds_alternative<int>(value)){
            newRow.type = fieldset::FieldTypes::Integer;
        }
        else if (std::holds_alternative<std::string>(value)){
            newRow.type = fieldset::FieldTypes::String;
        }
        else if (std::holds_alternative<std::vector<char>>(value)){
            newRow.type = fieldset::FieldTypes::Bytes;
        }
        newRow.size = fieldMap_[name]->getValueSize();
        newRow.rowData = value;
        newRecord.recordList[recordMapping_[name]] = std::make_unique<Record::Row>(newRow);
        // TODO: Additional checks?
    }

    for (auto&& record : *records_)
    {
        for (auto&& uniqueField : uniquieFields_)
        {
            if(record.recordList[recordMapping_[uniqueField->name()]] == ){
                
            }
        }
    }
};