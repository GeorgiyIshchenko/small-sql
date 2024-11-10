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

namespace fieldset
{

class BaseField
{

public:
    using value_type = std::variant<bool, int, std::string, std::vector<char>>;

public:
    BaseField(const std::string& name, value_type defaultValue = {},
              bool unique = false, bool key = false)
        : name_(name), defaultValue_(defaultValue), key_(key)
    {
        unique_ = key ? true : unique;
    }

    BaseField(const BaseField&) = default;

    BaseField(BaseField&&) = default;

    virtual ~BaseField() = default;

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

    virtual size_t getValueSize() = 0;

protected:
    std::string name_;
    std::optional<value_type> defaultValue_;
    bool unique_;
    bool key_;
};

class Integer : public BaseField
{

public:
    using value_type = int;

public:
    Integer(const std::string& name, int defaultValue = {}, bool unique = false,
            bool key = false, bool autoIncrement = false)
        : BaseField(name, defaultValue, unique, key),
          autoIncrement_(autoIncrement)
    {
    }

public:
    bool isAutoIncrement() const
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
        : Integer("id", 0, true, true, true)
    {
    }
};

class Bool : public BaseField
{

public:
    using value_type = bool;

public:
    Bool(const std::string& name, bool defaultValue = {}, bool unique = false,
         bool key = false)
        : BaseField(name, defaultValue, unique, key) {};

    size_t getValueSize() override
    {
        return sizeof(value_type);
    };
};

class String : public BaseField
{

public:
    using value_type = std::string;

public:
    String(const std::string& name, size_t maxLen,
           std::string defaultValue = {}, bool unique = false, bool key = false)
        : BaseField(name, defaultValue, unique, key), maxLen_(maxLen)
    {
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

class Bytes : public BaseField
{

public:
    using value_type = std::vector<char>;

public:
    Bytes(const std::string& name, size_t maxLen,
          std::vector<char> defaultValue = {}, bool unique = false,
          bool key = false)
        : BaseField(name, defaultValue, unique, key), maxLen_(maxLen)
    {
    }

    size_t getValueSize() override
    {
        return maxLen_;
    };

private:
    size_t maxLen_;
};

enum class FieldTypes : char
{
    Integer,
    Id,
    Bool,
    String,
    Bytes,
};

} // namespace fieldset

} // namespace db
