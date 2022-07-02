// © 2020 Erik Rigtorp <erik@rigtorp.se>
// SPDX-License-Identifier: MIT

// Test latency spikes due to page cache write back.
// See https://rigtorp.se/virtual-memory/ for details
//
// usage: ./writeback <cpu> <file>
//
// if you use a file on tmpfs there should be no spikes
// if you use a file on a disk drive you should see a latency spikes every one
// second
//
// When a page in the page cache has been modified it is marked as dirty and
// needs to be eventually written back to disk. This process is called writeback
// and is triggered automatically on a timer or when specifically requested
// using the system calls fsync, fdatasync, sync, syncfs, msync, and others. If
// any of the dirty pages are part of a writable memory mapping, the writeback
// process must first update the page table to mark the page as read-only before
// writing it to disk. Any subsequent memory write to the page will cause a page
// fault, letting the kernel update the page cache state to dirty and mark the
// page writable again. In practice this means that writeback causes TLB
// shootdowns and that writes to pages that are currently being written to disk
// must stall until the disk write is complete. This leads to latency spikes for
// any process that is using file backed writable memory mappings.
//
// To avoid latency spikes due to page cache writeback you cannot create any
// file backed writable memory mappings. Creating anonymous writable memory
// mappings using mmap(...MAP_ANONYMOUS) or by mapping files on Linux tmpfs
// filesystem is fine.

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <cstring> // memcpy
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <map>

#include <vector>
#include <thread>

int main(int argc, char *argv[]) {

    int fd = open(argv[2], O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    int cpu = std::stoi(argv[1]);

    size_t size = 1 << 20;
    if (ftruncate(fd, size) == -1) {
        perror("ftruncate");
        exit(1);
    }

    auto *p = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    auto threshold = std::chrono::nanoseconds::max();
    auto runtime = std::chrono::seconds(300);

    // calculate threshold as minimum timestamp delta * 16
    if (threshold == std::chrono::nanoseconds::max()) {
        auto ts1 = std::chrono::steady_clock::now();
        for (int i = 0; i < 10000; ++i) {
            for (size_t i = 0; i < size; i += 4096) {
                ((char *) p)[i] = 1;
            }
            auto ts2 = std::chrono::steady_clock::now();
            if (ts2 - ts1 < threshold) {
                threshold = ts2 - ts1;
            }
            ts1 = ts2;
        }
        threshold *= 16;
    }

    std::cout << "threshold: " << threshold.count() << " ns" << std::endl;

    // avoid page faults and TLB shootdowns when saving samples
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("mlockall");
        std::cerr << "WARNING failed to lock memory, increase RLIMIT_MEMLOCK "
                     "or run with CAP_IPC_LOC capability.\n";
    }

    const auto deadline = std::chrono::steady_clock::now() + runtime;
    std::atomic<size_t> active_threads = {0};

    std::cout << "pid: " << getpid() << std::endl;

    auto func = [&](int cpu) {
        // pin current thread to assigned CPU
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set);
        if (sched_setaffinity(0, sizeof(set), &set) == -1) {
            perror("sched_setaffinity");
            exit(1);
        }

        // run jitter measurement loop
        auto ts1 = std::chrono::steady_clock::now();
        while (ts1 < deadline) {
            for (size_t i = 0; i < size; i += 4096) {
                ((char *) p)[i] = 1;
            }
            auto ts2 = std::chrono::steady_clock::now();
            if (ts2 - ts1 < threshold) {
                ts1 = ts2;
                continue;
            }
            std::cerr << "jitter " << std::setw(10) << (ts2 - ts1).count() << " ns"
                      << std::endl;
            ts1 = std::chrono::steady_clock::now();
        }
    };

    auto t = std::thread([&] { func(cpu); });

    while (std::chrono::steady_clock::now() < deadline) {
        fdatasync(fd);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    t.join();

    return 0;
}