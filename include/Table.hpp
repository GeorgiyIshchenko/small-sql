#pragma once

#include "Field.hpp"

#include <any>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace db
{

class Table final
{

    struct Record
    {

        struct Row
        {
            fieldset::FieldTypes type;
            size_t size;
            std::any rowData;
        };

        Record(size_t recordListSize)
        {
            recordList.resize(recordListSize);
        }

        std::vector<std::unique_ptr<Row>> recordList = {};

    };

    using FieldType = std::shared_ptr<fieldset::BaseField>;

    using Index =
        std::map<fieldset::BaseField::value_type, std::shared_ptr<Record>>;

public:
    using id_type = int;

public:
    Table(std::string name, std::vector<FieldType> fields);

public:
    std::vector<FieldType> getFields() const
    {
        return fields_;
    }

    std::vector<FieldType> getUniqueFields() const
    {
        return uniquieFields_;
    };

    FieldType getKeyField() const
    {
        return keyField_;
    };

public:
    void insert(std::map<std::string, fieldset::BaseField::value_type>);

private:
    constexpr static uint64_t id_ = 0;
    std::string name_;

    std::vector<FieldType> fields_{};
    FieldType keyField_;
    std::vector<FieldType> uniquieFields_{};

    // std::string parameter is a name of a field
    std::map<std::string, FieldType> fieldMap_;
    std::map<std::string, size_t> recordMapping_;
    std::map<std::string, Index> indexes_;

    std::vector<Record>* records_{};
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