#pragma once
#include <string>
class Curl_buffer
{
public:
    enum class type { rx = 0, tx, active };
    type t;
    std::string buffer;

    Curl_buffer(type t) : t(t) {}
    inline void clear() { buffer.clear(); }
};

