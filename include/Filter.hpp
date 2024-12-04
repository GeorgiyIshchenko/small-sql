#pragma once

#include "Column.hpp"
#include "Table.hpp"

namespace db
{

namespace filters
{

class Filter
{
public:
    virtual ~Filter() = default;
    virtual bool matches(const Table::Record& record, Table& table) const = 0;
};


class ComparisonFilter : public Filter {
public:
    enum Operator {
        EQUAL,
        NOT_EQUAL,
        LESS_THAN,
        LESS_THAN_OR_EQUAL,
        GREATER_THAN,
        GREATER_THAN_OR_EQUAL
    };

    ComparisonFilter(const std::string& fieldName, Operator op, const columns::BaseColumn::value_type& value)
        : fieldName_(fieldName), op_(op), value_(value) {}

    bool matches(const Table::Record& record, Table& table) const override;

private:
    std::string fieldName_;
    Operator op_;
    columns::BaseColumn::value_type value_;
};

class LogicalFilter : public Filter {
public:
    enum LogicalOperator {
        AND,
        OR
    };

    LogicalFilter(LogicalOperator op, std::unique_ptr<Filter> left, std::unique_ptr<Filter> right)
        : op_(op), left_(std::move(left)), right_(std::move(right)) {}

    bool matches(const Table::Record& record, Table& table) const override;

private:
    LogicalOperator op_;
    std::unique_ptr<Filter> left_;
    std::unique_ptr<Filter> right_;
};

class NotFilter : public Filter {
public:
    NotFilter(std::unique_ptr<Filter> operand)
        : operand_(std::move(operand)) {}

    bool matches(const Table::Record& record, Table& table) const override;

private:
    std::unique_ptr<Filter> operand_;
};


} // namespace filters

} // namespace db
