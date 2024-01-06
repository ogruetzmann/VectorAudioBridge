#pragma once
#include <chrono>
using namespace std::chrono_literals;
class Status {
public:
    bool connection { true };
    bool error { false };
    std::chrono::system_clock::duration interval { 200ms };

    void connected() noexcept;
    void disconnected() noexcept;
};
