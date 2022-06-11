#include <deque>
#include <vector>
#include <iostream>
#include "task.h"
#include "sched.h"
#include "events.h"

using namespace std;

void RoundRobinScheduler::sched(TaskSet &taskSet, TaskSet &departures) {
    //Write your implementation below
    std::deque<Task> queue;
    Time current_time = 0;
    Task task;
    Task tempTask;
    vector<Event> traces;

    while (true) {
        if (queue.empty()) {
            // Get the next task when the system is idle
            if (taskSet.next(task)) {
                task.remaining_time = task.service_time;
                current_time = task.arrival_time;
                queue.push_back(task);
                traces.push_back(Event{task.arrival_time, task.task_id, 'Q', '-'});
            }
        }

        // Dp the scheduling
        if (!queue.empty()) {
            // update current time
            if (queue.front().arrival_time > current_time)
                current_time = queue.front().arrival_time;
            traces.push_back(Event{current_time, queue.front().task_id, 'R', '-'});
            Time timeToRun = queue.front().remaining_time;

            // Get tasks that arrives before current task completes
            if (taskSet.nextArrivedBefore(task, current_time + quantum)){
                task.remaining_time = task.service_time;
                queue.push_back(task);
                traces.push_back(Event{queue.back().arrival_time, queue.back().task_id, 'Q', '-'});
            }

            // Run current task to completion if time to run is less then quantum
            if(timeToRun <= quantum)
              {
                current_time += timeToRun;
                queue.front().remaining_time -= timeToRun;
                if (queue.front().remaining_time <= 0) {
                  traces.push_back(Event{current_time, queue.front().task_id, 'X', '-'});
                  queue.front().departure_time = current_time;
                  departures.append(queue.front());
                  queue.pop_front();
                  //cout << "Here in the normal check" << endl;
                }
              }
            //If timeToRun is greater then a quantum then run for a quantum and
            //move the task to the back of the queue.
            else
              {
                current_time += quantum;
                queue.front().remaining_time -= quantum;
                if (queue.front().remaining_time > 0) {
                  traces.push_back(Event{current_time, queue.front().task_id, 'Q', '-'});
                  tempTask = queue.front();
                  queue.pop_front();
                  queue.push_back(tempTask);
                }
              }

            continue;
        }

        // break when all tasks complete, i.e.: queue.empty() and taskSet.empty() are both true
        if (taskSet.empty())
            break;
    }

    // Print the task traces and depatures
    //cout << "Here before the print statements" << endl;
    print_traces(traces);
    departures.print();
}
