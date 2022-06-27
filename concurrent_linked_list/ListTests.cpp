#include <gtest/gtest.h>
#include "CoarseLockList.h"
#include "NaiveList.h"
#include <climits>
#include <thread>
#include <vector>
#include "LockSingleList.h"
#include "LockCouplingList.h"
#include "OptimisticList.h"
#include "OptimisticLazyList.h"
#include "LockFreeList.h"
#include <ctime>

const int kTestSize = 40000;
const int kThreadNum = 8;

class ListFixture : public ::testing::Test {
protected:
    void SetUp() override {
        /* generate workload for each thread */
        srand(time(nullptr));
        for (int i = 0; i < kTestSize; ++i) {
            workload[rand() % kThreadNum].push_back(i);
        }
        //generate workload for
    }

//    void TearDown() override {}
#if defined(NAIVELIST)
    List<int, int> *list = new NaiveList<int, int>(INT_MIN, INT_MAX);
#elif defined(COARSELOCKLIST)
    List<int, int> *list = new CoarseLockList<int, int>(INT_MIN, INT_MAX);
#elif defined(LOCKSINGLELIST)
    List<int, int> *list = new LockSingleList<int, int>(INT_MIN, INT_MAX);
#elif defined(LOCKCOUPLINGLIST)
    List<int, int> *list = new LockCouplingList<int, int>(INT_MIN, INT_MAX);
#elif defined(OPTIMISTICLIST)
    List<int, int> *list = new OptimisticList<int, int>(INT_MIN, INT_MAX);
#elif defined(OPTIMISTICLAZYLIST)
    List<int, int> *list = new OptimisticLazyList<int, int>(INT_MIN, INT_MAX);
#elif defined(LOCKFREELIST)
    List<int, int> *list = new LockFreeList<int, int>(INT_MIN, INT_MAX);
#endif
    std::vector<int> workload[kThreadNum];
};

TEST_F(ListFixture, SequentialTest) {
    int ret, tmp;
    ret = list->Put(10, 10);
    ASSERT_TRUE(ret == 0);
    ret = list->Get(10, tmp);
    ASSERT_TRUE(ret == 0 && tmp == 10);
    ret = list->Del(10);
    ASSERT_TRUE(ret == 0);
    ret = list->Get(10, tmp);
    ASSERT_TRUE(ret == -1);
//    list->ShowTenThreadUnsafe();
}

TEST_F(ListFixture, ConcurrentPut) {
    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum; ++i) {
        threads[i] = new std::thread{[i, this]() {
            for (auto key: workload[i]) {
                list->Put(key, key);
            }
        }};
    }
    for (auto thread: threads) {
        if (thread->joinable())
            thread->join();
    }
//    list->ShowTenThreadUnsafe();
    EXPECT_EQ(list->CountThreadUnsafe(), kTestSize);
}

TEST_F(ListFixture, ConcurrentDel) {
    /* init items for Del */
    for (int i = 0; i < kTestSize; ++i) {
        list->Put(i, i);
    }
    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum; ++i) {
//        threads[i] = new std::thread{[]() {
//            for (auto key: keys) {
//                list->Del(key);
//            }
//        }, std::ref(workload[i]), list};
        threads[i] = new std::thread{[i, this]() {
            for (auto key: workload[i]) {
                list->Del(key);
            }
        }};
    }
    for (auto thread: threads) {
        if (thread->joinable())
            thread->join();
    }
    EXPECT_EQ(list->CountThreadUnsafe(), 0);
}

/* only read */
TEST_F(ListFixture, ConcurrentGet) {
}

TEST_F(ListFixture, ConcurrentPutDel) {
    /* init items only for Del */
    int expect_result = 0;
    for (int i = 0; i < kThreadNum; ++i) {
        if (i % 2 == 0) {
            expect_result += (int) workload[i].size();
            continue;
        }
        for (auto key: workload[i]) {
            list->Put(key, key);
        }
    }
    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum; i++) {
        threads[i] = new std::thread{[i, this]() {
            for (auto key: workload[i]) {
                if (i % 2 == 0) list->Put(key, key);
                else list->Del(key);
            }
        }};
    }
    for (auto thread: threads) {
        if (thread->joinable())
            thread->join();
    }
    ASSERT_EQ(list->CountThreadUnsafe(), expect_result);
}

TEST_F(ListFixture, ConcurrentPutGet) {
    /* init items only for Get */
    for (int i = 0; i < kThreadNum; ++i) {
        if (i % 2 == 0)
            continue;
        for (auto key: workload[i]) {
            list->Put(key, key);
        }
    }
    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum; ++i) {
        threads[i] = new std::thread{[i, this]() {
            for (auto key: workload[i]) {
                if (i % 2 == 0) {
                    list->Put(key, key);
                } else {
                    int value;
                    list->Get(key, value);
                    ASSERT_EQ(value, key);
                }
            }
        }};
    }
    for (auto thread: threads) {
        if (thread->joinable()) {
            thread->join();
        }
    }
    ASSERT_EQ(list->CountThreadUnsafe(), kTestSize);
}

TEST_F(ListFixture, ConcurrentDelGet) {
    /* init items */
    int expect_result = 0;
    for (int i = 0; i < kThreadNum; ++i) {
        if (i % 2 != 0) expect_result += (int) workload[i].size();
        for (auto key: workload[i])
            list->Put(key, key);
    }
    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum; ++i) {
        threads[i] = new std::thread{[i, this]() {
            for (auto key: workload[i]) {
                if (i % 2 == 0) {
                    list->Del(key);
                } else {
                    int value;
                    list->Get(key, value);
                    ASSERT_EQ(value, key);
                }
            }
        }};
    }
    for (auto thread: threads) {
        if (thread->joinable())
            thread->join();
    }
    ASSERT_EQ(list->CountThreadUnsafe(), expect_result);
}

TEST_F(ListFixture, ConcurrentPutDelGet) {
    /* init item */
    int expect_result = 0;
    for (int i = 0; i < kThreadNum; ++i) {
        if (i % 3 == 0) {
            expect_result += (int) workload[i].size();
        } else if (i % 3 == 1) {
            for (auto key: workload[i]) {
                list->Put(key, key);
            }
        } else {
            for (auto key: workload[i]) {
                list->Put(key, key);
            }
            expect_result += (int) workload[i].size();
        }
    }
    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum; i++) {
        threads[i] = new std::thread{[i, this]() {
            for (auto key: workload[i]) {
                if (i % 3 == 0) {
                    list->Put(key, key);
                } else if (i % 3 == 1) {
                    list->Del(key);
                } else {
                    int value;
                    list->Get(key, value);
                    ASSERT_EQ(value, key);
                }
            }
        }};
    }
    for (auto thread: threads) {
        if (thread->joinable())
            thread->join();
    }
    ASSERT_EQ(list->CountThreadUnsafe(), expect_result);
    //add two 
}
