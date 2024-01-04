#pragma once
#include <algorithm>
#include <string>
#include <vector>

namespace helpers {
std::vector<std::string> tokenize(std::string_view str, char delimiter);
}