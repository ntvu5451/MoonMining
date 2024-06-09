
#include <iostream>
#include <random>
#include <chrono>
#include "MoonMining.h"

MoonMining::MoonMining(int max_threads, int max_trucks, int max_loading_sites, float total_run_hours, std::string file_name) :
    max_threads_(max_threads),
    max_unloading_sites_(max_loading_sites),
    max_trucks_(max_trucks),
    full_trucks_semaphore_{ max_threads },
    empty_trucks_semaphore_{ max_threads },
    unloading_sites_semaphore_(max_loading_sites),
    unloading_queuelist_(max_loading_sites),
    total_run_time_(total_run_hours)
{
}

MoonMining::~MoonMining()
{
    if (output_file_.is_open())
    {
        output_file_.close();
    }

}
bool MoonMining::Init(std::string out_file)
{
    bool return_status = false;

    //get chrono time for seeding srand
    auto now = std::chrono::system_clock::now();

    // Convert the time to a seed value
    unsigned int seed = unsigned int(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());

    // Seed the random number generator
    srand(seed);

    //open output file
    output_file_.open(out_file.c_str(), std::ofstream::out);
    if(output_file_.is_open())
    {
        return_status = true;
        output_file_ << "****************MINING TRUCK STATISTICS***************" << std::endl << std::endl;
    }
    else
    {
        std::cout << "output file: "<< out_file << " failed to open." << std::endl;
    }

    //start the run time clock
    run_time_start_ = std::chrono::steady_clock::now();

    return return_status;
}

bool MoonMining::CreateThreads()
{
    bool return_status = true;

    for (int p = 0; p < max_trucks_; ++p) {
        producer_threads_.emplace_back([&] { Produce(); });
    }

    for (int c = 0; c < max_trucks_; ++c) {
        consumer_truck_threads_.emplace_back([&] { ConsumeLoadedTrucksIntoQueues(); });
    }

    for (int i = 0; i < max_unloading_sites_; ++i) {
        consumer_site_threads_.emplace_back([&] {ConsumeUnloadSite(i); });
    }

    if (producer_threads_.size() != max_trucks_ && consumer_truck_threads_.size() != max_trucks_)
    {
        return_status = false;
        std::cout << "producer threads and/or consumer truck threads were not created successfully." << std::endl;
    }
    if (consumer_site_threads_.size() != max_unloading_sites_)
    {
        std::cout << "consumer site threads were not created successfully." << std::endl;
        return_status = false;
    }

    return return_status;
}



bool MoonMining::JoinThreads()
{
    bool return_status = true;
    int unjoinable_count = 0;
    for (int i = 0; i < max_threads_; ++i)
    {
        if (producer_threads_[i].joinable())
        {
            producer_threads_[i].join(); // Wait for producer thread to finish
        }
        else
        {
            unjoinable_count++;
            return_status = false;
        }
        if (consumer_truck_threads_[i].joinable())
        {
            consumer_truck_threads_[i].join(); // Wait for consumer thread to finish
        }
        else
        {
            unjoinable_count++;
            return_status = false;
        }
    }

    for (int i = 0; i < max_unloading_sites_; ++i)
    {
        if (consumer_site_threads_[i].joinable())
        {
            consumer_site_threads_[i].join(); // Wait for consumer thread to finish
        }
        else
        {
            unjoinable_count++;
            return_status = false;

        }
    }
    if (!return_status)
    {
        std::cout << unjoinable_count << " threads failed to join." << std::endl;
    }

    return return_status;
}

void MoonMining::Produce() {
    bool end_run = false;
    
    while (!end_run) {
        int item = 0;
        empty_trucks_semaphore_.acquire();
        {
            std::lock_guard<std::mutex> lock_guard(mutex_);
            if (truck_queue_.size() < max_trucks_)
            {
                auto start_time = std::chrono::steady_clock::now();
                truck_queue_.push(item);
                int rand_num = 1 + (rand() % 5);//1-5 hrs
                std::this_thread::sleep_for(std::chrono::hours(rand_num));

                //add travel time to unloading site
                std::this_thread::sleep_for(std::chrono::minutes(30));

                std::thread::id trd_id = std::this_thread::get_id();
                auto end_time = std::chrono::steady_clock::now();

                //save mining stats per truck in hours
                mining_statistics_[trd_id] = (std::chrono::duration<float>(end_time - start_time).count())/3600.0;

                //check if duration for mining has reached max (total_run_time)
                if(((std::chrono::duration<float>(end_time - run_time_start_).count())/3600.0) > total_run_time_)
                {
                    end_run = true;
                }

                std::cout << "truck Id: " << trd_id << "  mining time: " <<mining_statistics_[trd_id] << std::endl;
                output_file_ << "truck_id ," << trd_id << " , mining_time, " << mining_statistics_[trd_id] << std::endl;
                std::cout << "Truck count after Mining: " << truck_queue_.size() << std::endl << std::endl;
            }
        }
        full_trucks_semaphore_.release();
    }

}

void MoonMining::ConsumeLoadedTrucksIntoQueues() {
    int load = 1;
    bool end_run = false;
    while (!end_run) {
        full_trucks_semaphore_.acquire();
        {
            std::lock_guard<std::mutex> guard(mutex_);

            //if truck_queue_.size() > 1, then add truck item to wait
            if (truck_queue_.size() > 0)
            {
                //Get truck into shortest queue for unloading
                PopulateUnLoadingSiteQueue();
            }

            //check if duration for mining has reached max total_run_time
            auto end_time = std::chrono::steady_clock::now();
            if ((std::chrono::duration<float>(end_time - run_time_start_).count() / 3600.0) > total_run_time_)
            {
                end_run = true;
            }
        }
        unloading_sites_semaphore_.release();
    }

}

void MoonMining::ConsumeUnloadSite(int site_number)
{
    int load = 1;
    bool end_run = false;
  
    while (!end_run) {
        unloading_sites_semaphore_.acquire();
        {
            std::lock_guard<std::mutex> guard(mutex_);

            if (unloading_queuelist_[site_number].size() > 0)
            {
                //time to unload the truck
                std::this_thread::sleep_for(std::chrono::minutes(5));
                 
                unloading_queuelist_[site_number].pop();
                if (truck_queue_.size() > 0)
                {
                    truck_queue_.pop();
                }
                
                //Keep Track of how many trucks are unloaded at each unloading site
                unloading_site_statistics_[site_number] += 1;

                //adjust site number to start at 1 for logging only
                std::cout << "unloading site: " << (site_number +1) << "  number of trucks unloaded: " << unloading_site_statistics_[site_number] << std::endl;

                //add travel time for each unloaded truck to travel back to mining site for more loads
                std::this_thread::sleep_for(std::chrono::minutes(30));

            }

            //check if duration for mining has reached max total_run_time_ hrs
            auto end_time = std::chrono::steady_clock::now();
            if ((std::chrono::duration<float>(end_time - run_time_start_).count() / 3600.0) > total_run_time_)
            {
                end_run = true;
            }
        }
        empty_trucks_semaphore_.release();
    }

}

void MoonMining::PopulateUnLoadingSiteQueue()
{
     int min_index = 0;
     int load_item = 1;
     //find unloading site with shortest queue
     for(int i = 0; i < unloading_queuelist_.size(); ++i)
     {
         //populate the first laoding queue that is empty
         if (unloading_queuelist_[i].size() == 0)
         {
             min_index = i;
             unloading_queuelist_[min_index].push(load_item);
             std::cout << "Unloading Site Number: " << min_index << " the queue is size: " << unloading_queuelist_[min_index].size() << std::endl;
             return;
         }
         else if (unloading_queuelist_[i].size() < unloading_queuelist_[min_index].size())
         {

             min_index = i; // Now unloading_queuelist_[min_index] is the shortest queue
            
         }
     }
     unloading_queuelist_[min_index].push(load_item);
     std::cout << " Unloading Site Number: " << min_index << " the queue is size: " << unloading_queuelist_[min_index].size() << std::endl;
}

void MoonMining::PrintQueueListStats()
{
    if (!output_file_.is_open())
    {
        std::cout << "Error. PrintQueueListStats(). output_file is not open." << std::endl;
        return;
    }

    for (int i = 0; i < unloading_queuelist_.size(); ++i)
    {
        output_file_ << std::endl << std::endl;
        output_file_ << "***************UNLOADING SITE STATISTICS***************" << std::endl;
        output_file_ << "Unloading Site Number, " << i << " , total trucks queued, " << unloading_queuelist_[i].size() << std::endl;
    }

    output_file_.close();
}


