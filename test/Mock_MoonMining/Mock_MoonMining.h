#include "gmock/gmock.h"
#include "../../MoonMining.h"

class Mock_MoonMining : virtual public MoonMining {
public:
	
	//using MoonMining::PopulateUnLoadingSiteQueue;
	Mock_MoonMining(int max_threads, int max_truck_num, int max_load_sites, float total_time, std::string filename) 
	: MoonMining(max_threads, max_truck_num, max_load_sites, total_time, filename) {}
	MOCK_METHOD(bool, Init, (std::string), (override));
	MOCK_METHOD(void, Produce, (), (override));
	MOCK_METHOD(bool, CreateThreads, (), (override));
	MOCK_METHOD(bool, JoinThreads, (), (override));
	MOCK_METHOD(void, ConsumeUnloadSite,(int),(override));
	MOCK_METHOD(void, ConsumeLoadedTrucksIntoQueues,(),(override));
	MOCK_METHOD(void, PopulateUnLoadingSiteQueue,(),(override));
	MOCK_METHOD(bool, IsRunTimeExceeded,(), (override));
	using MoonMining::PrintQueueListStats;
	MOCK_METHOD(void, PrintQueueListStats,(),(override));

};