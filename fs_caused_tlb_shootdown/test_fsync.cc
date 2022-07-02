/*
 * Demonstrate how fsync triggers TLB shootdowns and following page faults on ext4.
 *
 * The following result shows that
 *     no page fault (major faults) involved.
 *
 * $ sudo perf stat -e faults,minor-faults,major-faults ./cmake-build-debug/test_fsync
    Performance counter stats for './cmake-build-debug/test_fsync':
               40602      faults
               40602      minor-faults
                   0      major-faults
        28.284603556 seconds time elapsed
         0.878836000 seconds user
         6.463784000 seconds sys
 *
 * The following result shows that
 *     invlove some tlb shootdowns, but they are not the major performance bottleneck.
 * $ sudo perf stat -e tlb:tlb_flush ./cmake-build-debug/test_fsync
 *  Performance counter stats for './cmake-build-debug/test_fsync':

             13105      tlb:tlb_flush

      30.745604911 seconds time elapsed

       0.785518000 seconds user
       7.531740000 seconds sys
 */

#include <sched.h>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <thread>
#include <fcntl.h>
#include <unistd.h>

const int kThreadNum =4;
const int kTestNum = 10000;
const int kFileSize = 4 << 10;
const char kFilePath[] = "/tmp/test_fsync_tlb_flush";

int main() {
  // initialize file
  unlink(kFilePath);
  int fd = open(kFilePath, O_CREAT | O_RDWR | O_TRUNC, 0666);
  char* buffer = (char*)malloc(kFileSize);
  memset(buffer, 'A', kFileSize);
  write(fd, buffer, kFileSize);
  fsync(fd);
  free(buffer);

  // pin main thread to core[0]
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);
  sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);

  // start test
  std::thread *threads[kThreadNum];
  for (int i = 0; i < kTestNum; ++i) {
    for (int j = 0; j < kThreadNum; ++j) {
      threads[j] = new std::thread([fd, j]() {
        // pin current thread to core[j+1]
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(j+1, &cpuset);
        sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
        // read
        char *buffer = (char*)malloc(kFileSize);
        pread(fd, buffer, kFileSize, 0);
        // write
        memset(buffer, 'A'+j+1, kFileSize);
        pwrite(fd, buffer, kFileSize, 0);
      });
    }
    for (auto thread : threads) {
      thread->join();
    }
    fsync(fd);
  }
  return 0;
}
