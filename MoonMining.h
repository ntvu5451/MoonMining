#ifndef MOONMINING_H
#define MOONMINING_H

#include <thread>
#include <mutex>
#include <semaphore>
#include <queue>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

//scale to simulate faster clock
constexpr float TIME_SCALE_FACTOR = .015;

/** A thread-safe class which supports concurrent read-threads and write-threads
	manipulating the queue concurrently. In other words, the thread-safe queue can
	be used in the consumer-producer problem with multiple consumers and producers. **/

class MoonMining {

public:

	virtual ~MoonMining();

	/// <summary>
	/// <constructor for MoonMining that involves utilization of concurrent multithreading>
	/// </summary>
	/// <param name="max_threads">used to set how max number for semaphore threads</param>
	/// <param name="max_trucks">max number of mining trucks(n)</param>
	/// <param name="max_loading_sites">max number of loading sites(m)</param>
	/// <param name="total_run_hours">total desired run time for moon project</param>
	/// <param name="file_name">name of output file</param>
	MoonMining(int max_threads=10, int max_trucks=10, int max_loading_sites=5, float total_run_hours=2, std::string file_name="stats_output.csv");


	/// <summary>
	/// <initialize class variables and containers>
	/// <param name="file_name">name of output file</param>
	/// </summary>
	virtual bool Init(std::string out_file);

	/// <summary>
	/// <semaphore to load trucks at mining site>
	/// </summary>
	virtual void Produce();

	/// <summary>
	/// <semaphore to place loaded mined trucks>
	/// <into the shortest unloading sites queues>
	/// <as they become available from the mine>
	/// </summary>
	/// <summary>
	/// <semaphore to remove trucks from>
	/// <unloading site queues>
	/// </summary>
	/// <param name="site_number"></param>
	virtual void ConsumeUnloadSite(int site_number);

	/// <summary>
	/// <create threads for produce, consumeLoadedTrucksIntoQueues(),>
	/// <and ConsumeUnloadSite semaphores>
	/// </summary>
	virtual bool CreateThreads();

	/// <summary>
	/// <populate loaded trucks into queues>
	/// <one queue for each unloading site>
	/// </summary>
	virtual void ConsumeLoadedTrucksIntoQueues();

	/// <summary>
	/// <Join all semaphore threads>
	/// </summary>
	virtual bool JoinThreads();

	/// <summary>
	/// <Print out the stats for each unloading site and number of trucks queued/processed>
	/// </summary>
	virtual void PrintQueueListStats();

private:

	/// <summary>
	/// <function to pop loaded trucks from truck_queue>
	/// <to Unloading site queues>
	/// </summary>
	virtual void PopulateUnLoadingSiteQueue();

	//set size of unloadStation via command line
	std::queue<int> unload_stations_;
	std::queue<int> truck_queue_;

	std::counting_semaphore<> full_trucks_semaphore_;
	std::counting_semaphore<> empty_trucks_semaphore_;
	std::counting_semaphore<> unloading_sites_semaphore_;

	std::vector<std::thread> producer_threads_, consumer_truck_threads_, consumer_site_threads_;
	std::map<std::thread::id, double> mining_statistics_;
	std::map<int, int> unloading_site_statistics_;
	std::vector<std::queue<int>> unloading_queuelist_;
	std::chrono::time_point<std::chrono::steady_clock> run_time_start_;
	//std::chrono::time_point<std::chrono::system_clock> current_sim_time_;

	std::mutex mutex_;

	int max_threads_;
	int max_trucks_;
	int max_unloading_sites_;
	float total_run_time_;

	std::ofstream output_file_;

};

#endif //MOONMINIG.H