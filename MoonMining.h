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
	/// <param name="file_name">name of output file</param>
	MoonMining(int max_threads, int max_trucks, int max_loading_sites, std::string file_name);

	/// <summary>
	/// <semaphore to load trucks at mining site>
	/// </summary>
	void Produce();

	/// <summary>
	/// <semaphore to place loaded mined trucks>
	/// <into the shortest unloading sites queues>
	/// <as they become available from the mine>
	/// </summary>
	void ConsumeLoadedTrucksIntoQueues();

	/// <summary>
	/// <semaphore to remove trucks from>
	/// <unloading site queues>
	/// </summary>
	/// <param name="site_number"></param>
	void ConsumeUnloadSite(int site_number);

	/// <summary>
	/// <create threads for produce, consumeLoadedTrucksIntoQueues(),>
	/// <and ConsumeUnloadSite semaphores>
	/// </summary>
	void CreateThreads();

	/// <summary>
	/// <Join all semaphore threads>
	/// </summary>
	void JoinThreads();

	/// <summary>
	/// <Print out the stats for each unloading site and number of trucks queued/processed>
	/// </summary>
	void PrintQueueListStats();

private:

	/// <summary>
	/// <function to pop loaded trucks from truck_queue>
	/// <to Unloading site queues>
	/// </summary>
	void PopulateUnLoadingSiteQueue();

	//set size of unloadStation via command line
	std::queue<int> unload_stations_;
	std::queue<int> truck_queue_;

	std::counting_semaphore<> full_trucks_semaphore_;
	std::counting_semaphore<> empty_trucks_semaphore_;
	std::counting_semaphore<> loading_sites_semaphore_;

	std::vector<std::thread> producer_threads_, consumer_truck_threads_, consumer_site_threads_;
	std::map<std::thread::id, double> mining_statistics_;
	std::map<int, int> unloading_site_statistics_;
	std::vector<std::queue<int>> unloading_queuelist_;

	std::mutex mutex_;

	int max_threads_;
	int max_trucks_;
	int max_unloading_sites_;

	std::ofstream output_file_;

};

#endif //MOONMINIG.H