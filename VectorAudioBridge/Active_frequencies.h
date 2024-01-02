#pragma once
#include <mutex>
#include <string>
#include <vector>

#include "Frequency.h"

//using frequency_pairs = std::vector<std::pair<std::string, std::string>>;
using frequency_pairs = std::vector<Frequency>;

class Active_frequencies {
    friend class Vectoraudio_socket;

public:
    const bool is_changed();
    const frequency_pairs& get();
    void set(frequency_pairs& p);
    void clear();

private:
    frequency_pairs pairs;
    bool changed { false };
    std::string curl_buffer;
    std::mutex lock;
};
