#include <iostream>
#include <random>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[]) {
    int seed = 1;
    int nPeriods = 5;
    int max_concurrent_arrivals = 2;
    int max_time_skip = 10;
    int max_cpu_burst = 20;

    if (argc >= 2) {
        seed = atoi(argv[1]);
    }
    if (argc >= 3) {
        nPeriods = atoi(argv[2]) ;
    }
    if (argc >= 4) {
        max_concurrent_arrivals = atoi(argv[3]) ;
    }
    if (argc >= 5) {
        max_time_skip = atoi(argv[4]) ;
    }
    if (argc >= 6) {
        max_time_skip = atoi(argv[5]) ;
    }

    auto rng = default_random_engine(seed);
    int min_cpu_burst = 1;

    uniform_int_distribution<int> burst(min_cpu_burst, max_cpu_burst);
    uniform_int_distribution<int> arrivals(1, max_concurrent_arrivals);
    uniform_int_distribution<int> timing(1, max_time_skip);

    int taskID = 1;
    int time = 0;
    cout << "#ID,Arrival,Service" << endl;
    for (int i = 0; i < nPeriods; i++) {
        for (int j = 0; j < arrivals(rng); j++) {
            cout << taskID++ << " " << time << " " << burst(rng) << endl;
        }
        time += timing(rng);
    }
}
