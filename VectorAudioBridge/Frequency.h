#pragma once
#include <string>
struct Frequency {
    std::string name;
    std::string frequency;

    friend constexpr bool operator==(const Frequency& lhs, const Frequency& rhs) {
        return lhs.frequency == rhs.frequency && lhs.name == rhs.name;
    }
};
