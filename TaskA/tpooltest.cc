//
// Created by Xizhou Feng on 2/26/20.
//

#include "ThreadPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct Param {
    long start;
    long end;
    long result;
};

void find_sum(void *p) {
    struct Param *param = (struct Param *) p;
    long n, s = 0L;
    for (n = param->start; n <= param->end; n++) {
        s += n;
    }
    param->result = s;
    fprintf(stdout, "sum of %ld ... %ld = %ld\n", param->start, param->end, param->result);
}

int main(int argc, char *argv[]) {
    unsigned int nThreads = 3;
    unsigned int seed = 0;
    unsigned int nProblems = 10;

    if (argc >= 4) {
        nThreads = atoi(argv[1]);
        seed = atoi(argv[2]);
        nProblems = atoi(argv[3]);
    }

    ThreadPool pool(nThreads);
    srand(seed);
    Param *problems = new Param[nProblems];
    unsigned int i;
    for (i = 0; i < nProblems; i++) {
        problems[i].start = 1;
        problems[i].end = i + 1;
        pool.submit(&find_sum, &problems[i]);
    }
    sleep(3);
    for (i = 0; i < nProblems; i++) {
        problems[i].start = rand() % 10;
        problems[i].end = problems[i].start + rand() % 10;
        pool.submit(&find_sum, &problems[i]);
    }
    sleep(3);
    pool.shutdown();

    delete[] problems;
}