#pragma once

#include <vector>
#include <string>

namespace db {

std::string escapeCSVField(const std::string& field);

std::vector<std::string> parseCSVLine(const std::string& line);

}