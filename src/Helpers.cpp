#include "Helpers.hpp"

namespace db {

std::string escapeCSVField(const std::string& field)
{
    std::string escaped = field;
    size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != std::string::npos)
    {
        escaped.insert(pos, 1, '"');
        pos += 2;
    }
    if (escaped.find(',') != std::string::npos ||
        escaped.find('"') != std::string::npos)
    {
        escaped = "\"" + escaped + "\"";
    }
    return escaped;
}

std::vector<std::string> parseCSVLine(const std::string& line)
{
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); ++i)
    {
        char c = line[i];
        if (c == '"')
        {
            inQuotes = !inQuotes;
            // If the next character is also a quote, it's an escaped quote
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"')
            {
                field += '"';
                ++i;
            }
        }
        else if (c == ',' && !inQuotes)
        {
            fields.push_back(field);
            field.clear();
        }
        else
        {
            field += c;
        }
    }
    fields.push_back(field);
    return fields;
}

}