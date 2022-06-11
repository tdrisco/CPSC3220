#ifndef __EVENTS_H
#define __EVENTS_H
#include <vector>
#include "task.h"

struct Event {
    Time time;
    long task_id;
    char type;
    char queue;
};
void print_traces(std::vector<Event>& events);

#endif