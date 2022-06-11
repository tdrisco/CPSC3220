//
// Created by xizhouf on 3/7/20.
//
#include <fstream>
#include <iostream>
#include <string>
#include "task.h"
using namespace std;

const char *JobStateNames[] = {
        "Created",
        "Queued",
        "Running",
        "Paused",
        "Competed"
};

bool sortByTaskArrival(Task& a, Task& b) {
        return ((a.arrival_time < b.arrival_time) || ((a.arrival_time == b.arrival_time) && (a.task_id < b.task_id)));
}
bool sortByTaskId(Task& a, Task& b) { return a.task_id < b.task_id; }

TaskSet::TaskSet(const char *inFile) {
    load(inFile);
}

bool TaskSet::load(const char *inFile) {
    ifstream ifs(inFile);
    if (ifs.fail()) {
        cerr  << "open file " << inFile << " failed\n";
        return false;
    }

    char c;
    string s;
    ifs >> c;
    has_priority = false;
    while (c == '#') {
        getline(ifs, s);
        if (s == "ID,Arrival,Service")
            has_priority = false;
        else
            has_priority = true;
        ifs >> c;
    }
    ifs.unget();
    // cout << "has_priority: " << has_priority << endl;

    while (true) {
        Task t;
        ifs >> t.task_id >> t.arrival_time >> t.service_time;
        if (has_priority)
            ifs >> t.priority;
        // cout << t.task_id << " " << t.service_time << " " << ifs.good() << endl;
        if (ifs.good())
            tasks.push_back(t);
        else
            break;
    }
    ifs.close();
    tasks.sort(sortByTaskArrival);
    return true;
}

bool TaskSet::nextArrivedBefore(Task &task, Time time) {
    if (tasks.empty())
        return false;
    task = tasks.front();
    task.remaining_time = task.service_time;
    if (task.arrival_time > time)
        return false;
    tasks.pop_front();
    return true;
}

bool TaskSet::next(Task &task) {
    if (tasks.empty())
        return false;
    task = tasks.front();
    task.remaining_time = task.service_time;
    tasks.pop_front();
    return true;
}


void TaskSet::print() {
    tasks.sort(sortByTaskId);
    if (tasks.size() >= 1 && tasks.front().departure_time == 0) {
        cout << "###Incoming Tasks###" << endl;
        cout << "#ID,Arrival,Service" << endl;
        for (auto it = tasks.cbegin(); it != tasks.cend(); ++it) {
            cout << it->task_id << " " << it->arrival_time
                 << " " << it->service_time << endl;
        }
    }
    if (tasks.size() >= 1 && tasks.back().departure_time > 0) {
        Time totalResponseTime = 0;
        Time totalWaitTime = 0;
        cout << "###Task Scheduling Results###" << endl;
        cout << "#ID,Arrival,Service,Departure,Response,Wait" << endl;
        for (auto it = tasks.cbegin(); it != tasks.cend(); ++it) {
            Time responseTime = it->departure_time - it->arrival_time;
            Time waitTime = responseTime - it->service_time;
            totalResponseTime += responseTime;
            totalWaitTime += waitTime;
            cout << it->task_id << " " << it->arrival_time
                 << " " << it->service_time << " " << it->departure_time
                 << " " << responseTime << " " << waitTime << endl;
        }
        cout << "Total response time = " << totalResponseTime << endl;
        cout << "Total wait time = " << totalWaitTime << endl;
    }
}

void TaskSet::append(Task &task) {
    tasks.push_back(task);
}

void TaskSet::save(string filename) {
    ofstream out;
    out.open(filename, ofstream::out | ofstream::trunc);
    if (out.bad()) {
        cerr << "failed to create file: " << filename << endl;
        return;
    }
    out << "#ID,Arrival,Service,Departure,Response,Wait" << endl;
    for (auto it = tasks.cbegin(); it != tasks.cend(); ++it) {
        Time responseTime = it->departure_time - it->arrival_time;
        Time waitTime = responseTime - it->service_time;
        out << it->task_id << " " << it->arrival_time
             << " " << it->service_time << " " << it->departure_time
             << " " << responseTime << " " << waitTime << endl;
    }
    out.close();
}
