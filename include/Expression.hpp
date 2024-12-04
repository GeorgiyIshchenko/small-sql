#pragma once

#include "Lexer.hpp"
#include "Column.hpp"

#include <memory>

namespace db
{

namespace expression
{

class Context
{


public: 

    Context() = default;

    Context(const std::string& tableName): tableName_(tableName) {}

public:
    using Value = columns::BaseColumn::value_type;

public:
    Value getValue(const std::string& name) const;
    Value getStringLength(const std::string& name) const;

private:

    std::string tableName_;

};

class Expression
{
public:
    virtual ~Expression() = default;

    virtual Context::Value evaluate(const Context& context) const = 0;
};

class BinaryExpression : public Expression
{
public:
    BinaryExpression(lexer::Token op, std::unique_ptr<Expression> left,
                     std::unique_ptr<Expression> right)
        : op_(op), left_(std::move(left)), right_(std::move(right))
    {
    }

    Context::Value evaluate(const Context& context) const override;

private:
    lexer::Token op_;
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};

class UnaryExpression : public Expression
{
public:
    UnaryExpression(lexer::Token op, std::unique_ptr<Expression> operand)
        : op_(op), operand_(std::move(operand))
    {
    }

    Context::Value evaluate(const Context& context) const override;

private:
    lexer::Token op_;
    std::unique_ptr<Expression> operand_;
};

class LiteralExpression : public Expression
{
public:
    LiteralExpression(const lexer::Token& token)
        : token_(token)
    {
    }

    Context::Value evaluate(const Context& context) const override;

private:
    lexer::Token token_;
};

class IdentifierExpression : public Expression
{
public:
    IdentifierExpression(const std::string& name)
        : name_(name)
    {
    }

    Context::Value evaluate(const Context& context) const override;

private:
    std::string name_;
};

class StringLengthExpression : public Expression
{
public:
    StringLengthExpression(const std::string& name)
        : name_(name)
    {
    }

    Context::Value evaluate(const Context& context) const override;

private:
    std::string name_;
};

} // namespace expression

} // namespace db