#include <deque>
#include <vector>
#include "sched.h"
#include "task.h"
#include "events.h"

using namespace std;

void SRTFScheduler::sched(TaskSet &taskSet, TaskSet &departures) {
    std::deque<Task> queue;
    Time current_time = 0;
    Task task;
    vector<Event> traces;
    current_time = 0;

    while (true) {
        // if no task in the queue, get next task from the taskSet and update the time
        if (queue.empty()) {
            if (taskSet.next(task)) {
                task.remaining_time = task.service_time;
                current_time = task.arrival_time;
                queue.push_back(task);
                traces.push_back(Event{task.arrival_time, task.task_id, 'Q', '-'});
            }
        }

        // get all tasks that arrive  at or before current time
        while (taskSet.nextArrivedBefore(task, current_time)) {
            task.remaining_time = task.service_time;
            auto it = queue.begin();
            for (; it != queue.end(); ++it) {
                if (it->remaining_time > task.remaining_time) {
                    queue.insert(it, task);
                    break;
                }
            }
            if (it == queue.end()) {
                queue.push_back(task);
            }
            traces.push_back(Event{task.arrival_time, task.task_id, 'Q', '-'});
        }

        // run the task with the shortest remaining time
        if (!queue.empty()) {
            if (queue.front().arrival_time > current_time)
                current_time = queue.front().arrival_time;
            traces.push_back(Event{current_time, queue.front().task_id, 'R', '-'});

            Time timeToRun = queue.front().remaining_time;

            // check if any new task arrives before current task completes
            if (taskSet.nextArrivedBefore(task, current_time + timeToRun)){
                Time usedTimeSoFar = task.arrival_time - current_time;
                Time remainTime = queue.front().remaining_time - usedTimeSoFar;

                task.remaining_time = task.service_time;
                if (task.remaining_time < remainTime) {
                    // preempt current task and put the new job into the front and go to next schedule iteration
                    current_time += usedTimeSoFar;
                    queue.front().remaining_time -= usedTimeSoFar;
                    traces.push_back(Event{current_time, queue.front().task_id, 'P', '-'});
                    queue.push_front(task);
                    traces.push_back(Event{task.arrival_time, task.task_id, 'Q', '-'});
                    continue;
                } else {
                    // insert the new task to the queue sorted by the remaining time
                    auto it = queue.begin();
                    for(;it!=queue.end();++it) {
                        if (it->remaining_time > task.remaining_time) {
                            queue.insert(it, task);
                            traces.push_back(Event{task.arrival_time, task.task_id, 'Q', '-'});
                            break;
                        }
                    }
                    if (it == queue.end()) {
                        queue.push_back(task);
                        traces.push_back(Event{task.arrival_time, task.task_id, 'Q', '-'});
                    }
                }
            }

            // run current task to complete
            current_time += timeToRun;
            queue.front().remaining_time -= timeToRun;

            if (queue.front().remaining_time <= 0) {
                traces.push_back(Event{current_time, queue.front().task_id, 'X', '-'});
                queue.front().departure_time = current_time;
                departures.append(queue.front());
                queue.pop_front();
            }

            continue;
        }

        // stop the scheduling when both the taskSet and the queue is empty
        if (taskSet.empty())
            break;
    }
    print_traces(traces);
    departures.print();
}

