
#include "Column.hpp"
#include "DataBaseException.hpp"
#include "Helpers.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>

void db::columns::serializeCSV(std::ofstream& file,
                               std::shared_ptr<BaseColumn> column)
{
    ColumType colType = column->getColumnType();
    std::string colTypeStr;

    switch (colType)
    {
    case ColumType::Integer:
        colTypeStr = "Integer";
        break;
    case ColumType::Id:
        colTypeStr = "Id";
        break;
    case ColumType::Bool:
        colTypeStr = "Bool";
        break;
    case ColumType::String:
        colTypeStr = "String";
        break;
    case ColumType::Bytes:
        colTypeStr = "Bytes";
        break;
    default:
        colTypeStr = "None";
        break;
    }

    std::string name = column->name();

    bool defaultValuePresent = column->hasDefault();

    std::string defaultValueTypeStr;
    std::string defaultValueStr;
    if (defaultValuePresent)
    {
        auto defaultValue = column->getDefaultValue();
        ColumType defaultValueType =
            BaseColumn::getValueColumnType(defaultValue);
        switch (defaultValueType)
        {
        case ColumType::Integer:
            defaultValueTypeStr = "int";
            defaultValueStr = std::to_string(std::get<int>(defaultValue));
            break;
        case ColumType::Bool:
            defaultValueTypeStr = "bool";
            defaultValueStr = std::get<bool>(defaultValue) ? "1" : "0";
            break;
        case ColumType::String:
            defaultValueTypeStr = "string";
            defaultValueStr = std::get<std::string>(defaultValue);
            break;
        case ColumType::Bytes:
            defaultValueTypeStr = "bytes";
            // Convert bytes to hex string or base64 if needed
            defaultValueStr = std::string(
                std::get<std::vector<uint8_t>>(defaultValue).begin(),
                std::get<std::vector<uint8_t>>(defaultValue).end());
            break;
        default:
            defaultValueTypeStr = "None";
            defaultValueStr = "";
            break;
        }
    }
    else
    {
        defaultValueTypeStr = "";
        defaultValueStr = "";
    }

    bool unique = column->isUnique();
    bool key = column->isKey();
    bool index = column->isIndex();

    std::stringstream ss;

    ss << escapeCSVField(colTypeStr) << ",";
    ss << escapeCSVField(name) << ",";
    ss << (defaultValuePresent ? "1" : "0") << ",";
    ss << escapeCSVField(defaultValueTypeStr) << ",";
    ss << escapeCSVField(defaultValueStr) << ",";
    ss << (unique ? "1" : "0") << ",";
    ss << (key ? "1" : "0") << ",";
    ss << (index ? "1" : "0") << ",";

    if (colType == ColumType::Integer || colType == ColumType::Id)
    {
        bool autoIncrement = column->isAutoIncrement();
        ss << (autoIncrement ? "1" : "0");
    }
    else if (colType == ColumType::String)
    {
        auto strColumn = std::static_pointer_cast<String>(column);
        ss << strColumn->maxLen();
    }
    else if (colType == ColumType::Bytes)
    {
        auto bytesColumn = std::static_pointer_cast<Bytes>(column);
        ss << bytesColumn->getValueSize();
    }
    else
    {
        ss << "0"; // end
    }

    file << ss.str() << std::endl;
}

std::shared_ptr<db::columns::BaseColumn>
db::columns::deserializeCSV(std::istringstream& file)
{
    std::string line;
    if (!std::getline(file, line))
    {
        return nullptr;
    }

    std::vector<std::string> fields = parseCSVLine(line);

    // Check if we have at least 9 fields
    if (fields.size() < 9)
    {
        throw DatabaseException("Error: Invalid number of fields in CSV line.");
    }

    // Extract fields
    std::string colTypeStr = fields[0];
    std::string name = fields[1];
    bool defaultValuePresent = fields[2] == "1";
    std::string defaultValueTypeStr = fields[3];
    std::string defaultValueStr = fields[4];
    bool unique = fields[5] == "1";
    bool key = fields[6] == "1";
    bool index = fields[7] == "1";
    std::string additionalFieldStr = fields[8];

    // Determine ColumnType
    ColumType colType;
    if (colTypeStr == "Integer")
    {
        colType = ColumType::Integer;
    }
    else if (colTypeStr == "Id")
    {
        colType = ColumType::Id;
    }
    else if (colTypeStr == "Bool")
    {
        colType = ColumType::Bool;
    }
    else if (colTypeStr == "String")
    {
        colType = ColumType::String;
    }
    else if (colTypeStr == "Bytes")
    {
        colType = ColumType::Bytes;
    }
    else
    {
        colType = ColumType::None;
    }

    // Reconstruct defaultValue
    std::optional<BaseColumn::value_type> defaultValue{};

    if (defaultValuePresent)
    {
        ColumType defaultValueType;
        if (defaultValueTypeStr == "int")
        {
            defaultValueType = ColumType::Integer;
        }
        else if (defaultValueTypeStr == "bool")
        {
            defaultValueType = ColumType::Bool;
        }
        else if (defaultValueTypeStr == "string")
        {
            defaultValueType = ColumType::String;
        }
        else if (defaultValueTypeStr == "bytes")
        {
            defaultValueType = ColumType::Bytes;
        }
        else
        {
            defaultValueType = ColumType::None;
        }

        if (defaultValueStr.size())
        {
            switch (defaultValueType)
            {
            case ColumType::Integer:
            {
                Integer::value_type val = std::stoi(defaultValueStr);
                defaultValue = val;
                break;
            }
            case ColumType::Bool:
            {
                Bool::value_type val = defaultValueStr == "true";
                defaultValue = val;
                break;
            }
            case ColumType::String:
            {
                defaultValue = defaultValueStr;
                break;
            }
            case ColumType::Bytes:
            {
                Bytes::value_type bytes(defaultValueStr.begin(),
                                        defaultValueStr.end());
                defaultValue = bytes;
                break;
            }
            default:
                defaultValue = std::nullopt;
                break;
            }
        }
    }

    // Construct the appropriate column object
    std::shared_ptr<BaseColumn> column;

    if (colType == ColumType::Integer)
    {
        bool autoIncrement = additionalFieldStr == "1";
        int defaultIntValue =
            defaultValuePresent && defaultValue.has_value()
                ? std::get<Integer::value_type>(defaultValue.value())
                : 0;
        column = std::make_shared<Integer>(name, defaultIntValue, index, unique,
                                           key, autoIncrement);
    }
    else if (colType == ColumType::Id)
    {
        column = std::make_shared<Id>();
    }
    else if (colType == ColumType::Bool)
    {
        bool defaultBoolValue =
            defaultValuePresent && defaultValue.has_value()
                ? std::get<Bool::value_type>(defaultValue.value())
                : false;
        column =
            std::make_shared<Bool>(name, defaultBoolValue, index, unique, key);
    }
    else if (colType == ColumType::String)
    {
        size_t maxLen = std::stoull(additionalFieldStr);
        String::value_type defaultStringValue =
            defaultValuePresent && defaultValue.has_value()
                ? std::get<String::value_type>(defaultValue.value())
                : "";
        column = std::make_shared<String>(name, maxLen, defaultStringValue,
                                          index, unique, key);
    }
    else if (colType == ColumType::Bytes)
    {
        size_t maxLen = std::stoull(additionalFieldStr);
        Bytes::value_type defaultBytesValue =
            defaultValuePresent && defaultValue.has_value()
                ? std::get<Bytes::value_type>(defaultValue.value())
                : Bytes::value_type();
        column = std::make_shared<Bytes>(name, maxLen, defaultBytesValue, index,
                                         unique, key);
    }
    else
    {
        throw DatabaseException("Error: Unknown column type.");
    }

    return column;
    return {};
}
