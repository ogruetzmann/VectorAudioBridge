#include "Helpers.h"

std::vector<std::string> helpers::tokenize(std::string_view str, char delimiter)
{
    if (!str.size())
        return std::vector<std::string>();

    std::vector<std::string> results;
    auto pos = str.find(delimiter);
    while (pos != str.npos) {
        results.push_back(std::string { str.substr(0, pos) });
        str.remove_prefix(pos + 1);
        pos = str.find(delimiter);
    }
    results.push_back(std::string { str });
    return results;
}
