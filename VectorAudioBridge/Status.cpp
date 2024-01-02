#include "Status.h"

void Status::connected()
{
    connection = true;
    interval = 200ms;
}

void Status::disconnected()
{
    connection = false;
    interval = 5s;
}
