#pragma once
#include <string>
#include <vector>

using frequency_pairs = std::vector<std::pair<std::string, std::string>>;

class Active_frequencies {
    friend class Vectoraudio_socket;

public:
    const bool is_changed() const;
    const frequency_pairs& get();
    void set(frequency_pairs& p);
    void clear();

private:
    frequency_pairs pairs;
    bool changed { false };
    std::string curl_buffer;
};
