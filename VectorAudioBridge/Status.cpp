#include "Status.h"

void Status::connected() noexcept
{
    connection = true;
    interval = 200ms;
}

void Status::disconnected() noexcept
{
    connection = false;
    interval = 5s;
}
