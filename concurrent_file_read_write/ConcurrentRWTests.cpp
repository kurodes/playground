//
// Created by lwh on 22-6-27.
//
#include <atomic>
#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <thread>

#include "gtest/gtest.h"

const int kBufferSize = 8192;
//n write, 1 read
const int kThreadNum = 16;
const int kWriteTimes = 10000;

bool CheckBuffer(char *buf, int size) {
    for (int i = 0; i < kBufferSize; i++) {
        if (buf[i] != buf[0]) {
            for (int j = 0; j < kBufferSize; j++) {
                std::cout << buf[j];
            }
            std::cout << "\n" << std::flush;
            return false;
        }
    }
    return true;
}

class IntraProcess : public ::testing::Test {
   public:
    void SetUp() override {
        for (int i = 0; i < 26; i++) {
            write_bufs[i] = (char *)malloc(kBufferSize);
            memset(write_bufs[i], 'A' + i, kBufferSize);
        }
        read_bufs = (char *)malloc(kBufferSize);
    }
    char *write_bufs[26];
    char *read_bufs;
    bool stop_flag = false;
};

TEST(Simple, SequentialRW) {
    char *buf = (char *)malloc(kBufferSize);
    memset(buf, 'a', kBufferSize);
    char *buf2 = (char *)malloc(kBufferSize);
    int fd, ret;
    fd = open("/tmp/test.txt", O_RDWR | O_CREAT, 0644);
    ret = pwrite(fd, buf, kBufferSize, 0);
    ret = pread(fd, buf2, kBufferSize, 0);
    CheckBuffer(buf2, kBufferSize);
    ASSERT_TRUE(memcmp(buf, buf2, kBufferSize) == 0);
    close(fd);
}


TEST_F(IntraProcess, ConcurrentBufferedIO) {
    int fd = open("/tmp/test_BufferedIO.txt", O_RDWR | O_CREAT, 0644);
    pwrite(fd, write_bufs[0], kBufferSize, 0);

    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum - 1; i++) {
        threads[i] = new std::thread([fd, this]() {
            for (int j = 0; j < kWriteTimes; j++) {
                pwrite(fd, write_bufs[rand() % 26], kBufferSize, 0);
            }
        });
    }
    threads[kThreadNum - 1] = new std::thread([fd, this]() {
        while (true) {
            pread(fd, read_bufs, kBufferSize, 0);
            ASSERT_TRUE(CheckBuffer(read_bufs, kBufferSize));
            if (stop_flag) break;
        }
    });
    for (int i = 0; i < kThreadNum - 1; i++) {
        if (threads[i]->joinable()) threads[i]->join();
    }
    stop_flag = true;
    if (threads[kThreadNum-1]->joinable())
        threads[kThreadNum-1]->join();

    close(fd);
}

TEST_F(IntraProcess, ConcurrentDirectIO) {
    int fd = open("/tmp/test_DirectIO.txt", O_RDWR | O_CREAT | O_DIRECT, 0644);
    pwrite(fd, write_bufs[0], kBufferSize, 0);

    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum - 1; i++) {
        threads[i] = new std::thread([fd, this]() {
            for (int j = 0; j < kWriteTimes; j++) {
                pwrite(fd, write_bufs[rand() % 26], kBufferSize, 0);
            }
        });
    }
    threads[kThreadNum - 1] = new std::thread([fd, this]() {
        while (true) {
            pread(fd, read_bufs, kBufferSize, 0);
            ASSERT_TRUE(CheckBuffer(read_bufs, kBufferSize));
            if (stop_flag) break;
        }
    });
    for (int i = 0; i < kThreadNum - 1; i++) {
        if (threads[i]->joinable()) threads[i]->join();
    }
    stop_flag = true;
    if (threads[kThreadNum-1]->joinable())
        threads[kThreadNum-1]->join();

    close(fd);
}

TEST_F(IntraProcess, ConcurrentDAX) {
    int fd = open("/mnt/pmem/test_DAXIO.txt", O_RDWR | O_CREAT, 0644);
    pwrite(fd, write_bufs[0], kBufferSize, 0);

    std::thread *threads[kThreadNum];
    for (int i = 0; i < kThreadNum - 1; i++) {
        threads[i] = new std::thread([fd, this]() {
            for (int j = 0; j < kWriteTimes; j++) {
                pwrite(fd, write_bufs[rand() % 26], kBufferSize, 0);
            }
        });
    }
    threads[kThreadNum - 1] = new std::thread([fd, this]() {
        while (true) {
            pread(fd, read_bufs, kBufferSize, 0);
            ASSERT_TRUE(CheckBuffer(read_bufs, kBufferSize));
            if (stop_flag) break;
        }
    });
    for (int i = 0; i < kThreadNum - 1; i++) {
        if (threads[i]->joinable()) threads[i]->join();
    }
    stop_flag = true;
    if (threads[kThreadNum-1]->joinable())
        threads[kThreadNum-1]->join();

    close(fd);
}