#include <thread>
#include <climits>
#include <functional>
#include <iostream>
#include <string>
#include "CoarseLockList.h"
#include "OptimisticList.h"
#include <thread>
#include <vector>

//const int kTestSize = 40000;
//const int kThreadNum = 2;

template<typename T>
typename std::remove_reference<T>::type&& move(T&& param) {
    using ReturnType = typename std::remove_reference<T>::type&&;
    return static_cast<ReturnType>(param);
}

int main() {
    auto a = std::make_shared<int>(10);
    *a = 10;
    std::cout << *a << std::endl;





//#if defined(COARSELIST)
//    List<int, int> *list = new CoarseLockList<int, int>(INT_MIN, INT_MAX);
//#elif defined(NAIVELIST)
//    List<int, int> *list = new NaiveList<int, int>(INT_MIN, INT_MAX);
//#elif defined(LOCKSINGLENODELIST)
//    List<int, int> *list = new LockSingleList<int, int>(INT_MIN, INT_MAX);
//#elif defined(LOCKCOUPLINGLIST)
//    List<int, int> *list = new LockCouplingList<int, int>(INT_MIN, INT_MAX);
//#elif defined(OPTIMISTICLIST)
//    List<int, int> *list = new OptimisticList<int, int>(INT_MIN, INT_MAX);
//#endif
//    std::vector<int> workload[kThreadNum];
//    /* generate workload for each thread */
//    for (int i = 0; i < kTestSize; ++i) {
//        workload[rand() % kThreadNum].push_back(i);
//    }
//
//    /* init items */
//    int expect_result = 0;
//    for (int i = 0; i < kThreadNum; ++i) {
//        if (i % 2 != 0) expect_result += (int) workload[i].size();
//        for (auto key: workload[i])
//            list->Put(key, key);
//    }
//    std::thread *threads[kThreadNum];
//    for (int i = 0; i < kThreadNum; ++i) {
//        threads[i] = new std::thread{[i, &list, &workload]() {
//            for (auto key: workload[i]) {
//                if (i % 2 == 0) {
//                    list->Del(key);
//                } else {
//                    int value;
//                    list->Get(key, value);
//                }
//            }
//        }};
//    }
//    for (auto thread: threads) {
//        if (thread->joinable())
//            thread->join();
//    }
    return 0;
}