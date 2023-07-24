#include "Active_frequencies.h"

const bool Active_frequencies::is_changed() const
{
    return changed;
}

const frequency_pairs& Active_frequencies::get()
{
    changed = false;
    return pairs;
}

void Active_frequencies::set(frequency_pairs& p)
{
    if (p != pairs) {
        pairs.swap(p);
        changed = true;
    }
    curl_buffer.clear();
}

void Active_frequencies::clear()
{
    curl_buffer.clear();
}
