/*
 * Demonstrate how msync triggers TLB shootdowns and following page faults on ext4.
 *
 * The following result shows that
 *     each msync will cause a page fault (major fault) on the next access of the process,
 *     this is because the pte is set to be read-only on each msync.
 * $ sudo perf stat -e faults,minor-faults,major-faults ./cmake-build-debug/test_msync
   Performance counter stats for './cmake-build-debug/test_msync':
               10840      faults
                 840      minor-faults
               10000      major-faults
        11.452372048 seconds time elapsed
         0.517591000 seconds user
         3.074177000 seconds sys
 *
 *
 * The following result shows that
 *     the number of tlb shootdowns is far more than the kTestNum,
 *     this may because each msync and the following access will cause tlb shootdowns.
 *
 * $ sudo perf stat -e tlb:tlb_flush ./cmake-build-debug/test_msync
 * $ https://github.com/kurodes/playground/blob/master/monitors/tlb_shootdown.py
   Performance counter stats for './cmake-build-debug/test_msync':
              174046      tlb:tlb_flush
        14.646354109 seconds time elapsed
         0.718749000 seconds user
         7.212277000 seconds sys
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
const char kFilePath[] = "/tmp/test_msync_tlb_flush";

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

  void *ptr = mmap(nullptr, kFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  // start test
  std::thread *threads[kThreadNum];
  for (int i = 0; i < kTestNum; ++i) {
    for (int j = 0; j < kThreadNum; ++j) {
      threads[j] = new std::thread([ptr, j]() {
        // pin current thread to core[j+1]
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(j+1, &cpuset);
        sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
        // read
        char tmp = *((char*)ptr + 10);
        // write
        memset(ptr, 'A', kFileSize);
      });
    }
    for (auto thread : threads) {
      thread->join();
    }
    msync(ptr, kFileSize, MS_SYNC);
  }
  return 0;
}
