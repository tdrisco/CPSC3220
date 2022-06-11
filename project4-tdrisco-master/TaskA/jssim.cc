#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include "task.h"
#include "sched.h"

using namespace std;

string appName = "jssim";
string default_policy = "srtf";
string default_workload = "w0";

int main(int argc, char *argv[]) {

    string workload = default_workload;
    string policy = default_policy;
    vector<int> quantum {2, 4};

    // parse the argument
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') { // option
            if (argv[i][1] == 'w') {
                if (i + 1 < argc) {
                    workload = argv[++i];
                    continue;
                }
            } else if (argv[i][1] == 'p') {
                if (i + 1 < argc) {
                    policy = argv[++i];
                    continue;
                }
            } else if (argv[i][1] == 'q') {
                quantum.clear();
                while (i + 1 < argc) {
                    if (argv[i+1][0] == '-')
                        break;
                    quantum.push_back(atoi(argv[++i]));
                }
                continue;
            } else {
                printf("usage: %s -p fifo -w [workload]\n", argv[0]);
                printf("usage: %s -p srtf -w [workload]\n", argv[0]);
                printf("usage: %s -p rr -q [time-quantum] -w [workload] \n", argv[0]);
                printf("usage: %s -p mlfq -q [time-quantum-1, time-quantum-2] -w [workload]\n", argv[0]);
                exit(-1);
            }
        }
    }

    if (workload == "") {
        printf("please specify the workload file using -w [workload]\n");
        exit(-1);
    }

    TaskSet taskSet(workload.c_str());
    TaskSet departures;
    //taskSet.print();

    if (strcmp(policy.c_str(), "fifo") == 0) {
        FIFOScheduler fifo;
        printf("%s workload=%s\n", "FIFOScheduler", workload.c_str());
        fifo.sched(taskSet, departures);
    } else if (strcmp(policy.c_str(), "rr") == 0) {
        if (quantum.empty())
            quantum.push_back(2);
        RoundRobinScheduler rr(quantum.front());
        printf("%s %d workload=%s\n", "RoundRobinScheduler", quantum.front(), workload.c_str());
        rr.sched(taskSet, departures);
    } else if (strcmp(policy.c_str(), "srtf") == 0) {
        SRTFScheduler srtf;
        printf("%s workload=%s\n", "SRTFScheduler", workload.c_str());
        srtf.sched(taskSet, departures);
    } else if (strcmp(policy.c_str(), "mlfq") == 0) {
        if (quantum.empty())
            quantum.push_back(2);
        if (quantum.size() == 1)
            quantum.push_back(2 * quantum.front());
        MLFQScheduler mlfq(quantum[0], quantum[1]);
        printf("%s high_priority_queue_quantum=%d low_priority_queue_quantum=%d workload=%s\n", "MLFQScheduler",
                quantum[0], quantum[1], workload.c_str());
        mlfq.sched(taskSet, departures);
    } else {
        printf("unimplemented policy - %s\n", policy.c_str());
        exit(-1);
    }

    ostringstream oss;
    oss << workload << "-" << policy << ".result";
    departures.save(oss.str());

    return 0;
}