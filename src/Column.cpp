
#include "Column.hpp"
#include <algorithm>
#include <cstddef>
#include <optional>

void db::columns::serialize(std::ofstream& file,
                            std::shared_ptr<BaseColumn> column)
{
    file << static_cast<int>(column->getColumnType());
    // General info
    file << column->name();
    file << column->isIndex();
    file << column->isUnique();
    file << column->isKey();
    file << column->hasDefault();
    // Special for Integer
    file << column->isAutoIncrement();
    if (column->hasDefault())
    {
        file << column->getValueSize();
        if (column->getColumnType() == columns::ColumType::Integer)
        {
            file << std::get<columns::Integer::value_type>(
                column->defaultValue_.value());
        }
        else if (column->getColumnType() == columns::ColumType::Id)
        {
            file << std::get<columns::Id::value_type>(
                column->defaultValue_.value());
        }
        else if (column->getColumnType() == columns::ColumType::Bool)
        {
            file << std::get<columns::Bool::value_type>(
                column->defaultValue_.value());
        }
        else if (column->getColumnType() == columns::ColumType::Bytes)
        {
            auto bytes = std::get<columns::Bytes::value_type>(
                column->defaultValue_.value());
            std::for_each(bytes.begin(), bytes.end(),
                          [&file](auto val) { file << val; });
        }
        else if (column->getColumnType() == columns::ColumType::String)
        {
            auto chars = std::get<columns::String::value_type>(
                column->defaultValue_.value());
            std::for_each(chars.begin(), chars.end(),
                          [&file](auto val) { file << val; });
        }
    }
}

std::shared_ptr<db::columns::BaseColumn>
db::columns::deserialize(std::ifstream& file)
{
    int intType;
    file >> intType;
    ColumType type = static_cast<ColumType>(intType);
    if (type == ColumType::None){    
        return nullptr;
    }
    std::string name;
    std::getline(file, name);
    bool isIndex, isUnique, isKey, hasDefault, isAutoIncrement;
    file >> isIndex;
    file >> isUnique;
    file >> isKey;
    file >> hasDefault;
    file >> isAutoIncrement;
    std::optional<BaseColumn::value_type> defaultValue;
    if (hasDefault)
    {
        size_t size;
        file >> size;
        if (type == columns::ColumType::Integer)
        {
            Integer::value_type buf;
            file >> buf;
            defaultValue = buf;
        }
        else if (type == columns::ColumType::Id)
        {
            Id::value_type buf;
            file >> buf;
            defaultValue = buf;
        }
        else if (type == columns::ColumType::Bool)
        {
            Bool::value_type buf;
            file >> buf;
            defaultValue = buf;
        }
        else if (type == columns::ColumType::Bytes)
        {
            Bytes::value_type vector{};
            for (int _ = 0; _ < size; ++_)
            {
                Bytes::value_type::value_type buf;
                file >> buf;
                vector.push_back(buf);
            }
            defaultValue = vector;
        }
        else if (type == columns::ColumType::String)
        {
            String::value_type str{};
            for (int _ = 0; _ < size; ++_)
            {
                String::value_type::value_type chr;
                file >> chr;
                str += chr;
            }
            defaultValue = str;
        }
    }
    if (type == columns::ColumType::Integer)
    {
        return Integer(name, std::get<Integer::value_type>(defaultValue.value()), isIndex, isUnique, isKey, isAutoIncrement);
    }
    else if (type == columns::ColumType::Id)
    {

    }
    else if (type == columns::ColumType::Bool)
    {

    }
    else if (type == columns::ColumType::Bytes)
    {

    }
    else if (type == columns::ColumType::String)
    {

    }
    return nullptr;
}
