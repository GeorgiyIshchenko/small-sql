#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <sys/types.h>
#include <variant>
#include <vector>

namespace db
{
namespace columns
{

enum class ColumType : u_char
{
    None = 0,
    Integer = 1, // Explicit declaration for deserialization
    Id = 2,
    Bool = 3,
    String = 4,
    Bytes = 5,
};

class BaseColumn;

void serialize(std::ofstream& file, std::shared_ptr<BaseColumn> column);

std::shared_ptr<BaseColumn> deserialize(std::ifstream& file);

void serializeCSV(std::ofstream& file, std::shared_ptr<BaseColumn> column);

std::shared_ptr<BaseColumn> deserializeCSV(std::istringstream& file);

class BaseColumn
{

public:
    using value_type = std::variant<bool, int, std::string, std::vector<uint8_t>>;

public:
    BaseColumn(const std::string& name, value_type defaultValue = {},
               bool index = false, bool unique = false, bool key = false)
        : name_(name), defaultValue_(defaultValue), index_(index), key_(key)
    {
        unique_ = key ? true : unique;
    }

    BaseColumn(const BaseColumn&) = default;

    BaseColumn(BaseColumn&&) = default;

    virtual ~BaseColumn() = default;

public:
    const std::string& name() const
    {
        return name_;
    }

    bool isUnique() const
    {
        return unique_;
    }

    bool isKey() const
    {
        return key_;
    }

    bool isIndex() const
    {
        return index_;
    }

    bool hasDefault() const
    {
        return defaultValue_.has_value();
    }

    value_type getDefaultValue() const
    {
        return defaultValue_.value();
    }

    virtual ColumType getColumnType() const = 0;

    virtual bool isAutoIncrement() const
    {
        return false;
    }

    virtual size_t getValueSize() = 0;

    friend void columns::serialize(std::ofstream& file,
                                   std::shared_ptr<BaseColumn> column);

    friend std::shared_ptr<BaseColumn>
    columns::deserialize(std::ifstream& file);

    friend void columns::serializeCSV(std::ofstream& file,
                                      std::shared_ptr<BaseColumn> column);

    friend std::shared_ptr<BaseColumn>
    columns::deserializeCSV(std::istringstream& file);

public:
    static ColumType getValueColumnType(value_type value)
    {
        if (std::holds_alternative<bool>(value))
        {
            return columns::ColumType::Bool;
        }
        if (std::holds_alternative<int>(value))
        {
            return columns::ColumType::Integer;
        }
        if (std::holds_alternative<std::string>(value))
        {
            return columns::ColumType::String;
        }
        if (std::holds_alternative<std::vector<uint8_t>>(value))
        {
            return columns::ColumType::Bytes;
        }
        return ColumType::None;
    };

protected:
    std::string name_;
    std::optional<value_type> defaultValue_;
    bool unique_;
    bool index_;
    bool key_;
};

class Integer : public BaseColumn
{

public:
    using value_type = int;

public:
    Integer(const std::string& name, int defaultValue = {}, bool index = false,
            bool unique = false, bool key = false, bool autoIncrement = false)
        : BaseColumn(name, defaultValue, index, unique, key),
          autoIncrement_(autoIncrement)
    {
    }

public:
    ColumType getColumnType() const override
    {
        return ColumType::Integer;
    }

    bool isAutoIncrement() const override
    {
        return autoIncrement_;
    }

    size_t getValueSize() override
    {
        return sizeof(value_type);
    };

private:
    bool autoIncrement_;
};

class Id final : public Integer
{

public:
    Id()
        : Integer("id", 0, true, true, true, true)
    {
    }

    ColumType getColumnType() const override
    {
        return ColumType::Id;
    }
};

class Bool final : public BaseColumn
{

public:
    using value_type = bool;

public:
    Bool(const std::string& name, bool defaultValue = {}, bool index = false,
         bool unique = false, bool key = false)
        : BaseColumn(name, defaultValue, index, unique, key) {};

    ColumType getColumnType() const override
    {
        return ColumType::Integer;
    }

    size_t getValueSize() override
    {
        return sizeof(value_type);
    };
};

class String final : public BaseColumn
{

public:
    using value_type = std::string;

public:
    String(const std::string& name, size_t maxLen,
           std::string defaultValue = {}, bool index = false,
           bool unique = false, bool key = false)
        : BaseColumn(name, defaultValue, index, unique, key), maxLen_(maxLen)
    {
    }

    ColumType getColumnType() const override
    {
        return ColumType::String;
    }

    size_t getValueSize() override
    {
        return maxLen_;
    };

public:
    size_t maxLen() const
    {
        return maxLen_;
    }

private:
    size_t maxLen_;
};

class Bytes final : public BaseColumn
{

public:
    using value_type = std::vector<uint8_t>;

public:
    Bytes(const std::string& name, size_t maxLen,
          std::vector<uint8_t> defaultValue = {}, bool index = false,
          bool unique = false, bool key = false)
        : BaseColumn(name, defaultValue, index, unique, key), maxLen_(maxLen)
    {
    }

    ColumType getColumnType() const override
    {
        return ColumType::Bytes;
    }

    size_t getValueSize() override
    {
        return maxLen_;
    };

private:
    size_t maxLen_;
};

} // namespace columns

} // namespace db
