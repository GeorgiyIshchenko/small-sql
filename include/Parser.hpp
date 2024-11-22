#pragma once

#include <string>

namespace db
{

class Parser{

    explicit Parser(const std::string& request): request_(request){}

public:

    void parse();

private:

    std::string request_;

};

}
