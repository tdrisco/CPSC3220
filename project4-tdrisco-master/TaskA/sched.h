//
// Created by xizhouf on 3/7/20.
//
#ifndef JSSIM_SCHED_H
#define JSSIM_SCHED_H
#include "task.h"

class FIFOScheduler {
public:
    FIFOScheduler(){};
    ~FIFOScheduler(){}
    void sched(TaskSet& taskSet, TaskSet& result);
};

class RoundRobinScheduler {
    Time quantum;
public:
    RoundRobinScheduler(Time _quantum): quantum(_quantum){};
    ~RoundRobinScheduler(){}
    void sched(TaskSet& taskSet, TaskSet& result);
};

class SRTFScheduler {
public:
    SRTFScheduler(){};
    ~SRTFScheduler(){}
    void sched(TaskSet& taskSet, TaskSet& result);
};

class MLFQScheduler {
    Time h_quantum, l_quantum;
public:
    MLFQScheduler(int high_quantum, int low_quantum): h_quantum(Time(high_quantum)), l_quantum(low_quantum) {};
    ~MLFQScheduler(){}
    void sched(TaskSet& taskSet, TaskSet& result);
};

#endif //JSSIM_SCHED_H
