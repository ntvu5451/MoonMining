
#include "Mock_MoonMining.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace {
    class MoonMiningGTest : public ::testing::Test {
    public:
        MoonMining* pMoonMineObj_ = nullptr;

        virtual void SetUp() {
            int max_truck_num = 50;
            int max_load_sites = 30;
            int max_threads = max_truck_num;
            float total_time = 2;
            std::string filename = "MineStats.csv";
            //pMoonMineObj_ = new Mock_MoonMining(max_threads, max_truck_num, max_load_sites, total_time,filename);
        }

        virtual void TearDown() {
            //if (pMoonMineObj_)
            //   delete pMoonMineObj_;
        }
        

    };
    }

    TEST(MoonMiningGTest, init_queues) {
        int max_truck_num = 50;
        int max_load_sites = 30;
        int max_threads = max_truck_num;
        float total_time = 5;
        std::string filename = "MineStats1.csv";
        MoonMining mining(max_threads, max_truck_num, max_load_sites, total_time, filename);
        EXPECT_TRUE(mining.Init(filename));
    }

    TEST(MoonMiningGTest, join_threads) {
        int max_truck_num = 50;
        int max_load_sites = 30;
        int max_threads = max_truck_num;
        float total_time = 5;
        std::string filename = "MineStats3.csv";
        Mock_MoonMining mock(max_threads, max_truck_num, max_load_sites, total_time, filename);
        ON_CALL(mock, JoinThreads()).WillByDefault(::testing::Return(true));
        //EXPECT_TRUE(mock.JoinThreads())
    }

    TEST(MoonMiningGTest, create_threads) {
        int max_truck_num = 1;
        int max_load_sites = 1;
        int max_threads = max_truck_num;
        float total_time = 1;
        std::string filename = "MineStats2.csv";
        MoonMining mineObj(max_threads, max_truck_num, max_load_sites, total_time, filename);
        mineObj.Init(filename);
        EXPECT_TRUE(mineObj.CreateThreads());
        //EXPECT_CALL(mineObj,ConsumeUnloadSite(1));
        //EXPECT_CALL(mineObj,ConsumeLoadedTrucksIntoQueues());
        //EXPECT_CALL(mineObj,Produce());

    }




//more tests ...

//run all tests 
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}