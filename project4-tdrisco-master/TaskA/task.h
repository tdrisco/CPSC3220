#ifndef JSSIM_TASK_H
#define JSSIM_TASK_H

#include <list>
#include <vector>
#include <string>

//typedef enum {
//    TASK_Created = 0,
//    TASK_Queued = 1,
//    TASK_Running = 2,
//    TASK_Paused = 3,
//    TASK_Completed = 4,
//} TaskState;

typedef unsigned long Time;

struct Task {
    int task_id;
    int priority;   // smaller value, higher priority
    Time arrival_time;
    Time service_time;
    Time remaining_time;
    Time departure_time;
};

extern const char *JobStateNames[];

class TaskSet {
public:
    bool has_priority;
    std::list<Task> tasks;
public:
    TaskSet(){}    /* constructor */
    TaskSet(const char* inFile); /* constructor with a workload file */
    bool load(const char* inFile); /* load tasks from a workload file */
    bool empty() {return tasks.empty();}
    bool nextArrivedBefore(Task& task, Time time=0); /* get next job before time; return true if has a task; false otherwise */
    bool next(Task& task);
    void append(Task& task);
    void print();
    void save(std::string filename);
};
#endif //JSSIM_TASK_H
