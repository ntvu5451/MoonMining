// MoonProject.cpp : Defines the entry point for the application.
//

#include "MoonMining.h"

int main(int argc, char* argv[]){
    
    //get size from command line to set num of stations
    //run for 72 hrs
    //get values from command line
    int max_truck_num = 8;
    int max_load_sites = 10;
    int max_threads= 10;
    float total_run_in_hrs = 72.0;
    std::string filename = "output.csv";
    if ((argc >= 4))
    {
        max_truck_num = atoi(argv[1]);
        max_load_sites = atoi(argv[2]);
        total_run_in_hrs = atof(argv[3]);
        if (argc == 5)
        {
            filename = std::string(argv[4]);
        }
    }
                                                                                 
    max_threads = max_truck_num;
    MoonMining mining_obj(max_threads, max_truck_num, max_load_sites,total_run_in_hrs,filename);
    bool status = mining_obj.Init(filename);
    if (!status)
    {
        std::cout << "MoonMining Init failed. Project stopped." << std::endl;
        return 0;
    }

    if (mining_obj.CreateThreads())
    {
        mining_obj.JoinThreads();
        mining_obj.PrintQueueListStats();
    }
    else
    {
        std::cout << "MoonMining CreateThreads() failed. Project stopped." << std::endl;
    }

    return 0;

}
