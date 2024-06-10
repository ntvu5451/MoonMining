
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <chrono>
#include "MoonMining.h"

MoonMining::MoonMining(int max_threads, int max_trucks, int max_loading_sites, float total_run_hours, std::string file_name) :
    max_threads_(max_threads),
    max_unloading_sites_(max_loading_sites),
    max_trucks_(max_trucks),
    full_trucks_semaphore_{ max_trucks },
    empty_trucks_semaphore_{ max_trucks },
    unloading_sites_semaphore_{max_loading_sites},
    unloading_queuelist_(max_loading_sites),
    total_run_time_(total_run_hours * TIME_SCALE_FACTOR)
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
    auto now = std::chrono::steady_clock::now();

    // Convert the time to a seed value
    unsigned int seed = unsigned int(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());

    // Seed the random number generator
    srand(seed);

    //open output file for truck statistics
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

    //open output file for unloading site statistics
    std::string outfile_name = "unloading_site_" + out_file;
    site_output_file_.open(outfile_name.c_str(), std::ofstream::out);
    if(site_output_file_.is_open())
    {
        return_status = true;
        site_output_file_ << "***************UNLOADING SITE STATISTICS***************" << std::endl;
    }
    else
    {
        std::cout << "output file: "<< outfile_name<< " failed to open." << std::endl;
    }

    //initialize vector of unloading site queues
    std::queue<int> new_queue;
    std::fill(unloading_queuelist_.begin(), unloading_queuelist_.end(), new_queue);

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
    for (int i = 0; i < producer_threads_.size(); ++i)
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

    for (int i = 0; i < consumer_site_threads_.size(); ++i)
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
    double sim_time_duration_seconds = 0.0;
    time_t system_time_now;
    
    while (!end_run) {
        int item = 0;
        int rand_num = 1;
        empty_trucks_semaphore_.acquire();
        {

            std::lock_guard<std::mutex> lock_guard(mutex_);

            //check if duration for mining has reached max total_run_time_hrs
            end_run = IsRunTimeExceeded();
            if (truck_queue_.size() < max_trucks_ && !end_run)
            {
                truck_queue_.push(item);

                //generate simulated mining time between 1-5 hrs
                rand_num = 1 + (rand() % 5);
            
                auto time_now = std::chrono::steady_clock::now();
                //calculate mining duration time with 30 min travel time to unloading site
                auto const sim_time = ( time_now + std::chrono::hours(rand_num) + std::chrono::minutes(30));

                //save mining stats per truck in hours
                std::thread::id trd_id = std::this_thread::get_id();
               
                //calculate mining time
                sim_time_duration_seconds = duration_cast<std::chrono::seconds>(sim_time - time_now).count();

                //convert to hrs for stats
                mining_statistics_[trd_id] = (sim_time_duration_seconds)/3600.0;

                system_time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
               
               //log to output file
                output_file_ << "truck_id ," << trd_id << " , mining_time, " << mining_statistics_[trd_id] << 
                " , "<< std::ctime(&system_time_now)<< std::endl;

                std::cout << "truck Id: " << trd_id << "  mining time: " <<mining_statistics_[trd_id] << std::endl;
                std::cout << "Truck count after Mining: " << truck_queue_.size() << std::endl << std::endl;
            }

        }//end semaphore acquire

        //simulate mining time with time scale factor
        int scaled_mining_time_secs = sim_time_duration_seconds * TIME_SCALE_FACTOR;
        std::this_thread::sleep_for(std::chrono::seconds(scaled_mining_time_secs));

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
            end_run = IsRunTimeExceeded();
            if (truck_queue_.size() > 0 && !end_run)
            {
                //Get truck into shortest queue for unloading
                PopulateUnLoadingSiteQueue();
            }

        }

        unloading_sites_semaphore_.release();
    }

}

void MoonMining::ConsumeUnloadSite(int site_number)
{
    int load = 1;
    bool end_run = false;
    double sim_time_duration_seconds = 0.0;
    std::time_t system_time_now; 
  
    while (!end_run) {
        unloading_sites_semaphore_.acquire();
        {
            std::lock_guard<std::mutex> guard(mutex_);

            //check if duration for mining has reached max total_run_time_hrs
            end_run = IsRunTimeExceeded();
            if (unloading_queuelist_.size() > site_number && !end_run)
            {
                if(unloading_queuelist_[site_number].size() > 0)
                {
                    unloading_queuelist_[site_number].pop();

                    system_time_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                    //log to output file
                    site_output_file_<< "unloading site: ,"<< (site_number +1) << " , total number of trucks unloaded: , " 
                        << unloading_site_statistics_[site_number] << " , "<< std::ctime(&system_time_now) << std::endl;

                    if (truck_queue_.size() > 0)
                    {
                        truck_queue_.pop();
                    }
                    
                    //Keep Track of how many trucks are unloaded at each unloading site
                    unloading_site_statistics_[site_number] += 1;

                    //adjust site number to start at 1 for logging only
                    std::cout << "unloading site: "
                    << (site_number +1) << "  total number of trucks unloaded: " 
                    << unloading_site_statistics_[site_number] << std::endl;

                }
            }
        }

        //advance 30 min travel time back to mining site and 5 min to unload truck
        auto const sim_time = std::chrono::steady_clock::now() + std::chrono::minutes(30) + std::chrono::minutes(5);
        sim_time_duration_seconds = duration_cast<std::chrono::seconds>(sim_time - std::chrono::steady_clock::now()).count();

        //Scale the unloading time and travel time 
        int scaled_mining_time_secs = sim_time_duration_seconds * TIME_SCALE_FACTOR;
        std::this_thread::sleep_for(std::chrono::seconds(scaled_mining_time_secs));
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

bool MoonMining::IsRunTimeExceeded()
{
    bool time_expired = false;
    auto time_now = std::chrono::steady_clock::now();
    auto time_diff = time_now - run_time_start_;
    auto time_secs = duration_cast<std::chrono::seconds>(time_diff).count();
    auto time_hrs = time_secs/3600.0;
    if(time_hrs > total_run_time_)
    {
       time_expired = true;
    }
    return time_expired;
}

