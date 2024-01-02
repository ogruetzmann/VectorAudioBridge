#include "Active_frequencies.h"

const bool Active_frequencies::is_changed()
{
    std::lock_guard<std::mutex> guard(lock);
    return changed;
}

const frequency_pairs& Active_frequencies::get()
{
    std::lock_guard<std::mutex> guard(lock);
    changed = false;
    return pairs;
}

void Active_frequencies::set(frequency_pairs& p)
{
    if (p != pairs) {
        std::lock_guard<std::mutex> guard(lock);
        pairs.swap(p);
        changed = true;
    }
    curl_buffer.clear();
}

void Active_frequencies::clear()
{
    curl_buffer.clear();
}
