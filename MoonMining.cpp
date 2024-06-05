
#include <iostream>
#include <random>
#include <chrono>
#include "MoonMining.h"

MoonMining::MoonMining(int max_threads, int max_trucks, int max_loading_sites, std::string file_name) :
    max_threads_(max_threads),
    full_trucks_semaphore_{ max_threads },
    empty_trucks_semaphore_{ max_threads },
    max_trucks_(max_trucks),
    max_unloading_sites_(max_loading_sites),
    loading_sites_semaphore_(max_loading_sites),
    unloading_queuelist_(max_loading_sites),
    output_file_(file_name)
{

    //initialize unloading site queuelist
    for (int i = 0; i < unloading_queuelist_.size(); ++i)
    {
        std::queue<int> new_queue;
        unloading_queuelist_[i] = new_queue;
    }

    if (output_file_.is_open())
    {
        output_file_ << "****************MINING TRUCK STATISTICS***************" << std::endl << std::endl;
    }

}

void MoonMining::CreateThreads()
{
    for (int p = 0; p < max_trucks_; ++p) {
        producer_threads_.emplace_back([&] { Produce(); });

    }

    for (int c = 0; c < max_trucks_; ++c) {
        consumer_truck_threads_.emplace_back([&] { ConsumeLoadedTrucksIntoQueues(); });
    }

    for (int i = 0; i < max_unloading_sites_; ++i) {
        consumer_site_threads_.emplace_back([&] {ConsumeUnloadSite(i); });
    }
}

MoonMining::~MoonMining()
{
    if (output_file_.is_open())
    {
        output_file_.close();
    }

}

void MoonMining::JoinThreads()
{
    for (int i = 0; i < max_threads_; ++i)
    {
        producer_threads_[i].join(); // Wait for producer thread to finish
        consumer_truck_threads_[i].join(); // Wait for consumer thread to finish
    }

    for (int i = 0; i < max_unloading_sites_; ++i)
    {
        consumer_site_threads_[i].join(); // Wait for consumer thread to finish
    }

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
                std::this_thread::sleep_for(std::chrono::seconds(rand_num));

                //add travel time to unloading site
                std::this_thread::sleep_for(std::chrono::seconds(1));

                std::thread::id trd_id = std::this_thread::get_id();
                auto end_time = std::chrono::steady_clock::now();

                mining_statistics_[trd_id] = std::chrono::duration<float>(end_time - start_time).count();

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
        }
        loading_sites_semaphore_.release();
    }
}

void MoonMining::ConsumeUnloadSite(int site_number)
{
    int load = 1;
    bool end_run = false;
    int index = site_number - 1;//adjust for zero based indexing
    while (!end_run) {
        loading_sites_semaphore_.acquire();
        {
            std::lock_guard<std::mutex> guard(mutex_);

            if (unloading_queuelist_[index].size() > 0)
            {
                //time to unload the truck
                std::this_thread::sleep_for(std::chrono::seconds(5));
                 
                unloading_queuelist_[index].pop();
                if (truck_queue_.size() > 0)
                {
                    truck_queue_.pop();
                }
                
                //add travel time to unloading site
                std::this_thread::sleep_for(std::chrono::seconds(5));

                //Keep Track of how many trucks are unloaded at each unloading site
                unloading_site_statistics_[index] += 1;
                std::cout << "unloading site: " << site_number << "  number of trucks unloaded: " << unloading_site_statistics_[index] << std::endl;
            }

            //std::cout << "Truck size after unloading: " << truck_queue_.size() << std::endl << std::endl; // Display buffer size after consuming
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
        output_file_ << "Unloading Site Number, " << i << " ,total trucks queued, " << unloading_queuelist_[i].size() << std::endl;
    }

    output_file_.close();
    
}


