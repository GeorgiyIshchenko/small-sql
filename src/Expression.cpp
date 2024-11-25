#include "Expression.hpp"
#include "Lexer.hpp"
#include "DataBaseException.hpp"

namespace db
{

namespace expression
{

// TODO:
Context::Value Context::getValue(const std::string& name) const
{
    (void)name;
    return {};
}

Context::Value Context::getStringLength(const std::string& name) const
{
    (void)name;
    return {};
}

Context::Value IdentifierExpression::evaluate(const Context& context) const
{
    return context.getValue(name_);
}

Context::Value LiteralExpression::evaluate(const Context& context) const
{
    (void)context;
    if (token_.type == lexer::TOK_INT_LITERAL)
    {
        return std::stoi(token_.lexeme);
    }
    else if (token_.type == lexer::TOK_STRING_LITERAL)
    {
        return token_.lexeme;
    }
    else if (token_.type == lexer::TOK_TRUE)
    {
        return true;
    }
    else if (token_.type == lexer::TOK_FALSE)
    {
        return false;
    }
    else if (token_.type == lexer::TOK_HEX_LITERAL)
    {
        columns::Bytes::value_type bytes;
        for (auto&& num : token_.lexeme)
        {
            if (std::isdigit(num))
            {
                bytes.push_back(std::atoi(&num) - 30);
            }
            else
            {
                bytes.push_back(std::atoi(&num) - 41);
            }
        }
        return bytes;
    }
    throw DatabaseException("Expression.cpp: Unknown token type");
}

Context::Value BinaryExpression::evaluate(const Context& context) const
{
    Context::Value leftValue = left_->evaluate(context);
    Context::Value rightValue = right_->evaluate(context);

    switch (op_.type)
    {
    case lexer::TOK_PLUS:
        return std::get<int>(leftValue) + std::get<int>(rightValue);
    case lexer::TOK_MINUS:
        return std::get<int>(leftValue) - std::get<int>(rightValue);
    case lexer::TOK_MULTIPLY:
        return std::get<int>(leftValue) * std::get<int>(rightValue);
    case lexer::TOK_DIVIDE:
        return std::get<int>(leftValue) / std::get<int>(rightValue);
    case lexer::TOK_EQUAL:
        return leftValue == rightValue;
    case lexer::TOK_NOT_EQUAL:
        return leftValue != rightValue;
    case lexer::TOK_AND:
        return std::get<bool>(leftValue) && std::get<bool>(rightValue);
    case lexer::TOK_OR:
        return std::get<bool>(leftValue) || std::get<bool>(rightValue);
    default:
        throw DatabaseException("Unsupported operator in evaluation");
    }
}

Context::Value UnaryExpression::evaluate(const Context& context) const
{
    Context::Value operandValue = operand_->evaluate(context);

    switch (op_.type)
    {
    case lexer::TOK_NOT:
        if (auto boolValue = std::get_if<bool>(&operandValue))
        {
            return !(*boolValue);
        }
        else
        {
            throw DatabaseException("Operator '!' requires a boolean operand");
        }
    case lexer::TOK_MINUS:
        if (auto intValue = std::get_if<int>(&operandValue))
        {
            return -(*intValue);
        }
        else
        {
            throw DatabaseException(
                "Unary '-' operator requires a numeric operand");
        }
    default:
        throw DatabaseException("Unsupported unary operator");
    }
}

Context::Value StringLengthExpression::evaluate(const Context& context) const
{
    return context.getStringLength(name_);
}

} // namespace expression

} // namespace db