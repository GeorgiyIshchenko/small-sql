#include "string"

namespace db
{
class DatabaseException : public std::exception
{
private:
    std::string message;

public:
    DatabaseException(const std::string& msg)
        : message(msg)
    {
    }

    virtual const char* what() const noexcept override
    {
        return message.c_str();
    }
};
} // namespace db
