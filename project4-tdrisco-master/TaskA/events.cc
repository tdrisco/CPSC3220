#include <map>
#include <algorithm>
#include <iostream>
#include "events.h"

using namespace std;

/*
 * print_traces
 */
void print_traces(std::vector<Event>& events){
    Time t;
    int  runningTaskID = 0;
    map<char, vector<int>> queuedTasks;
    map<char, int> maxQueueSizes;


    // find the maximum time
    Time maxTime = 0;
    for (auto it = events.cbegin(); it !=events.cend(); ++it) {
        if (it->time > maxTime)
            maxTime = it->time;
    }

    // find the number of queues
    for (auto it = events.cbegin(); it !=events.cend(); ++it) {
        auto p = queuedTasks.find(it->queue);
        if (p == queuedTasks.cend()) {
            queuedTasks.emplace(it->queue, vector<int>{});
        }
    }

    for (auto it = queuedTasks.cbegin(); it != queuedTasks.cend(); ++it) {
        maxQueueSizes.emplace(it->first, 0);
    }

    for (t = 0; t <= maxTime; t++) {
        // find the running task
        for (auto it = events.cbegin(); it != events.cend(); ++it) {
            if (it->time == t) {
                switch (it->type) {
                    case 'Q': {
                        auto p = find(queuedTasks[it->queue].begin(), queuedTasks[it->queue].end(), it->task_id);
                        if (p != queuedTasks[it->queue].end()) {
                            queuedTasks[it->queue].erase(p);
                        }
                        queuedTasks[it->queue].push_back(it->task_id);
                        break;
                    }
                    case 'R': {
                        runningTaskID = it->task_id;
                        auto p = find(queuedTasks[it->queue].begin(), queuedTasks[it->queue].end(), it->task_id);
                        if (p != queuedTasks[it->queue].end()) {
                            queuedTasks[it->queue].erase(p);
                        }
                        break;
                    }
                    case 'E':
                    case 'X': {
                        auto p = find(queuedTasks[it->queue].begin(), queuedTasks[it->queue].end(), it->task_id);
                        if (p != queuedTasks[it->queue].end()) {
                            queuedTasks[it->queue].erase(p);
                        }
                        if (it->task_id == runningTaskID) {
                            runningTaskID = 0;
                        }
                        break;
                    }
                    case 'P': {
                        queuedTasks[it->queue].insert(queuedTasks[it->queue].begin(), it->task_id);
                        break;
                    }
                    default: {
                        cerr << "Unexpected event type " << it->type << endl;
                        break;
                    }
                }
            }
        }
        for (auto it = queuedTasks.cbegin(); it != queuedTasks.cend(); ++it) {
            maxQueueSizes[it->first] = maxQueueSizes[it->first] < int(queuedTasks[it->first].size()) ?
                    int(queuedTasks[it->first].size()) : maxQueueSizes[it->first];
        }
    }
    for (auto it = queuedTasks.begin(); it != queuedTasks.end(); ++it) {
        queuedTasks[it->first].clear();
    }

    cout << "#Scheduling Events###" << endl;
    cout << "Time Queue Event Task" << endl;
    for (auto it = events.begin(); it != events.end(); ++it) {
        cout.width(4); cout.fill('0'); cout << right << it->time;
        cout.width(4); cout.fill(' '); cout << it->queue;
        cout.width(4); cout.fill(' '); cout << it->type;
        cout.width(4); cout.fill(' '); cout << " " << it->task_id << endl;
    }

    cout << "###System States###" << endl;
    cout << "Time" << " Run";
    for (auto im = queuedTasks.begin(); im != queuedTasks.end(); ++im) {
        int w = int(4*maxQueueSizes[im->first]) - 9;
        if (w <= 1) {
            w = 1;
        }
        cout << left << " || InQueue" << (*im).first;
        cout.width(w); cout << right << ' ';
    }
    cout << endl;

    // print system state
    for (t = 0; t <= maxTime; t++) {
        // find the running task
        for (auto it = events.cbegin(); it != events.cend(); ++it) {
            if (it->time == t) {
                switch (it->type) {
                    case 'Q': {
                        auto p = find(queuedTasks[it->queue].begin(), queuedTasks[it->queue].end(), it->task_id);
                        if (p != queuedTasks[it->queue].end()) {
                            queuedTasks[it->queue].erase(p);
                        }
                        queuedTasks[it->queue].push_back(it->task_id);
                        break;
                    }
                    case 'R': {
                        runningTaskID = it->task_id;
                        auto p = find(queuedTasks[it->queue].begin(), queuedTasks[it->queue].end(), it->task_id);
                        if (p != queuedTasks[it->queue].end()) {
                            queuedTasks[it->queue].erase(p);
                        }
                        break;
                    }
                    case 'E':
                    case 'X': {
                        auto p = find(queuedTasks[it->queue].begin(), queuedTasks[it->queue].end(), it->task_id);
                        if (p != queuedTasks[it->queue].end()) {
                            queuedTasks[it->queue].erase(p);
                        }
                        if (it->task_id == runningTaskID) {
                            runningTaskID = 0;
                        }
                        break;
                    }
                    case 'P': {
                        queuedTasks[it->queue].insert(queuedTasks[it->queue].begin(), it->task_id);
                        break;
                    }
                    default: {
                        cerr << "Unexpected event type " << it->type << endl;
                        break;
                    }
                }
            }
        }

        cout.width(4); cout.fill('0'); cout << right << t;
        cout.width(4); cout.fill(' ');
        if (runningTaskID == 0)
            cout << right << ' ';
        else
            cout << right << runningTaskID;
        for (auto im = queuedTasks.begin(); im != queuedTasks.end(); ++im) {
            char q = (*im).first;
            cout << " ||";
            int w = maxQueueSizes[q] * 4;
            if (w <= 10) {
                w = 10;
            }
            for (auto it = queuedTasks[q].cbegin(); it != queuedTasks[q].cend(); ++it) {
                if (*it != runningTaskID) {
                    cout.width(4); cout.fill(' '); cout << right << *it;
                    w -= 4;
                }
            }
            cout.width(w); cout << right <<  ' ';
        }
        cout << endl;
    }
}
