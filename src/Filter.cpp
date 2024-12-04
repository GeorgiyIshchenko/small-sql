#include "Filter.hpp"
#include "DataBaseException.hpp"

namespace db
{

namespace filters
{

bool ComparisonFilter::matches(const Table::Record& record, Table& table) const
{
    columns::BaseColumn::value_type recordValue =
        record.rows[table.getRecordMapping()[fieldName_]].rowData;

    switch (op_)
    {
    case EQUAL:
        return recordValue == value_;
    case NOT_EQUAL:
        return recordValue != value_;
    case LESS_THAN:
        return recordValue < value_;
    case LESS_THAN_OR_EQUAL:
        return recordValue <= value_;
    case GREATER_THAN:
        return recordValue > value_;
    case GREATER_THAN_OR_EQUAL:
        return recordValue >= value_;
    default:
        throw DatabaseException("Unknown comparison operator");
    }
}

bool LogicalFilter::matches(const Table::Record& record, Table& table) const {
    bool leftResult = left_->matches(record, table);
    bool rightResult = right_->matches(record, table);

    if (op_ == AND) {
        return leftResult && rightResult;
    } else if (op_ == OR) {
        return leftResult || rightResult;
    } else {
        throw DatabaseException("Unknown logical operator");
    }
}

bool NotFilter::matches(const Table::Record& record, Table& table) const {
    return !operand_->matches(record, table);
}

} // namespace filters

} // namespace db
