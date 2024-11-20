#pragma once

#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <sys/types.h>
#include <variant>
#include <vector>

namespace db
{
namespace columns
{

enum class ColumTypes : u_char
{
    Integer,
    Id,
    Bool,
    String,
    Bytes,
};

class BaseColumn
{

public:
    using value_type = std::variant<bool, int, std::string, std::vector<char>>;

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

    virtual ColumTypes getColumnType() const = 0;

    virtual bool isAutoIncrement() const
    {
        return false;
    }

    virtual size_t getValueSize() = 0;

protected:
    std::string name_;
    std::optional<value_type> defaultValue_;
    bool unique_;
    bool key_;
    bool index_;
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
    ColumTypes getColumnType() const override
    {
        return ColumTypes::Integer;
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

    ColumTypes getColumnType() const override
    {
        return ColumTypes::Id;
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

    ColumTypes getColumnType() const override
    {
        return ColumTypes::Integer;
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

    ColumTypes getColumnType() const override
    {
        return ColumTypes::String;
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
    using value_type = std::vector<char>;

public:
    Bytes(const std::string& name, size_t maxLen,
          std::vector<char> defaultValue = {}, bool index = false,
          bool unique = false, bool key = false)
        : BaseColumn(name, defaultValue, index, unique, key), maxLen_(maxLen)
    {
    }

    ColumTypes getColumnType() const override
    {
        return ColumTypes::Bytes;
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
