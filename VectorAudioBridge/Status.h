#pragma once
#include <chrono>
using namespace std::chrono_literals;
class Status
{
public:
    bool connection { false };
    bool error{ false };
    std::chrono::system_clock::duration interval{ 5s };

    void connected();
    void disconnected();
};

