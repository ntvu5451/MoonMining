// MoonProject.cpp : Defines the entry point for the application.
//

#include "MoonMining.h"

using namespace std;
#include <chrono>
#include <thread>
#include <mutex>
#include <semaphore>
#include <queue>
#include <random>
#include <thread>
#include <vector>

using namespace std::literals;

int main(int argc, char* argv[]){
    
    //get size from command line to set num of stations
    //run for 72 hrs
    //get values from command line
    int max_truck_num = 25;
    int max_load_sites = 20;
    int max_threads = max_truck_num;
    std::string filename = "output.csv";
    if ((argc >= 3))
    {
        max_truck_num = atoi(argv[1]);
        max_load_sites = atoi(argv[2]);
    }
    if ((argc == 4))
    {
        filename = argv[3];
    }                                                                              

    MoonMining mining_obj(max_threads, max_truck_num, max_load_sites,filename);
    mining_obj.CreateThreads();
    mining_obj.JoinThreads();

    mining_obj.PrintQueueListStats();

    return 0;

}
