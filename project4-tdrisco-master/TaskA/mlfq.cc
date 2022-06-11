 #include <deque>
#include <vector>
#include <iostream>
#include "task.h"
#include "sched.h"
#include "events.h"

using namespace std;

void MLFQScheduler::sched(TaskSet &taskSet, TaskSet& departures) {
    // Write your implementation below
    std::deque<Task> pQueue;
    std::deque<Task> queue;
    Time current_time = 0;
    Time preemptTime = 0;
    Task task;
    Task tempTask;
    vector<Event> traces;

    while (true) {
        if (pQueue.empty() && queue.empty()) {
            // Get the next task when the system is idle
            if (taskSet.next(task)) {
                task.remaining_time = task.service_time;
                current_time = task.arrival_time;
                pQueue.push_back(task);
                traces.push_back(Event{task.arrival_time, task.task_id, 'Q', 'H'});
            }
        }

        // Dp the scheduling
        if (!pQueue.empty()) {
            // update current time
            if (pQueue.front().arrival_time > current_time)
                current_time = pQueue.front().arrival_time;
            traces.push_back(Event{current_time, pQueue.front().task_id, 'R', 'H'});
            Time timeToRun = pQueue.front().remaining_time;

            // Get tasks that arrives before current task completes
            if (taskSet.nextArrivedBefore(task, current_time + h_quantum)){
                task.remaining_time = task.service_time;
                pQueue.push_back(task);
                traces.push_back(Event{pQueue.back().arrival_time, pQueue.back().task_id, 'Q', 'H'});
            }

            // Run current task to completion if time to run is less then quantum
            if(timeToRun <= h_quantum)
              {
                current_time += timeToRun;
                pQueue.front().remaining_time -= timeToRun;
                if (pQueue.front().remaining_time <= 0) {
                  traces.push_back(Event{current_time, pQueue.front().task_id, 'X', 'H'});
                  pQueue.front().departure_time = current_time;
                  departures.append(pQueue.front());
                  pQueue.pop_front();
                  //cout << "Here in the normal check" << endl;
                }
              }
            //If timeToRun is greater then a quantum then run for a quantum and
            //move the task to the back of the queue.
            else
              {
                current_time += h_quantum;
                pQueue.front().remaining_time -= h_quantum;
                if (pQueue.front().remaining_time > 0) {
                  traces.push_back(Event{current_time, pQueue.front().task_id, 'E', 'H'});
                  traces.push_back(Event{current_time, pQueue.front().task_id, 'Q', 'L'});
                  tempTask = pQueue.front();
                  pQueue.pop_front();
                  queue.push_back(tempTask);
                }
              }

            //continue;
      //  }

        while (!queue.empty() && pQueue.empty()) {
            // update current time
            if (queue.front().arrival_time > current_time)
                current_time = queue.front().arrival_time;
            traces.push_back(Event{current_time, queue.front().task_id, 'R', 'L'});
            Time timeToRun = queue.front().remaining_time;

            // Get tasks that arrives before current task completes
            if (taskSet.nextArrivedBefore(task, current_time)){
                task.remaining_time = task.service_time;
                pQueue.push_back(task);
                traces.push_back(Event{pQueue.back().arrival_time, pQueue.back().task_id, 'Q', 'H'});
                traces.push_back(Event{pQueue.back().arrival_time, pQueue.back().task_id, 'P', 'L'});
            }
            preemptTime = task.arrival_time-current_time;

            // Run current task to completion if time to run is less then quantum
            if(preemptTime > l_quantum)
                preemptTime = l_quantum;

            if(timeToRun <= preemptTime)
              {
                current_time += timeToRun;
                queue.front().remaining_time -= timeToRun;
                if (queue.front().remaining_time <= 0) {
                  traces.push_back(Event{current_time, queue.front().task_id, 'X', 'L'});
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
                current_time += preemptTime;
                queue.front().remaining_time -= preemptTime;
                if (queue.front().remaining_time > 0) {
                  traces.push_back(Event{current_time, queue.front().task_id, 'Q', 'L'});
                  tempTask = queue.front();
                  queue.pop_front();
                  queue.push_back(tempTask);
                  //cout << "Here in the normal check" << endl;
                }
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
