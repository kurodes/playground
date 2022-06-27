#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

constexpr int kBlockSize = 512;

int main()
{
    int fd = open("/tmp/dio_test.txt", O_CREAT | O_RDWR | O_DIRECT, 0664);
    assert(fd > 0);

    char* buffer;
    size_t length = kBlockSize;
    size_t offset = kBlockSize/2;

    int ret = posix_memalign((void **)&buffer, kBlockSize, length);
    assert(ret == 0);

    memset(buffer, 'H', length);

    off_t sret = lseek(fd, offset, SEEK_SET);
    assert(sret == offset);

    ssize_t wret = write(fd, (void *)buffer, length);
    assert(wret == kBlockSize);

    return 0;
}