#include <assert.h>
#include <fcntl.h>
#include <linux/aio_abi.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

static int io_setup(unsigned nr, aio_context_t *ctxp) {
    return syscall(__NR_io_setup, nr, ctxp);
}

static int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp) {
    return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

static int io_getevents(aio_context_t ctx, long min_nr, long max_nr,
                        struct io_event *events, struct timespec *timeout) {
    return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

static int io_cancel();

static int io_destroy(aio_context_t ctx) {
    return syscall(__NR_io_destroy, ctx);
}

int main() {
    int fd = open("/tmp/aio_test.txt", O_CREAT | O_RDWR | O_DIRECT, 0664);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    aio_context_t ctx = 0;
    int ret = io_setup(64, &ctx);
    if (ret < 0) {
        perror("io_setup");
        return -1;
    }
    std::cout << "aio_context_t = " << ctx << std::endl;

    size_t n_reqs = 8;
    size_t b_length = 512;
    char **bl = (char **)malloc(sizeof(char *) * n_reqs);
    for (size_t i = 0; i < n_reqs; ++i) {
        posix_memalign((void **)&bl[i], 512, b_length);
        memset(bl[i], 'A' + i, b_length);
    }

    size_t n_write = n_reqs / 2;
    size_t n_read = n_reqs / 2;
    iocb **cbl = (iocb **)malloc(sizeof(iocb *) * n_write);
    for (size_t i = 0; i < n_write; ++i) {
        cbl[i] = (iocb *)malloc(sizeof(iocb));
        memset(cbl[i], 0, sizeof(iocb));
        cbl[i]->aio_fildes = fd;
        cbl[i]->aio_lio_opcode = IOCB_CMD_PWRITE;
        cbl[i]->aio_buf = (uint64_t)bl[i];
        cbl[i]->aio_nbytes = b_length;
        cbl[i]->aio_offset = b_length * i;
    }

    iocb **cbl_r = (iocb **)malloc(sizeof(iocb *) * n_read);
    for (size_t i = 0; i < n_read; ++i) {
        cbl_r[i] = (iocb *)malloc(sizeof(iocb));
        memset(cbl_r[i], 0, sizeof(iocb));
        cbl_r[i]->aio_fildes = fd;
        cbl_r[i]->aio_lio_opcode = IOCB_CMD_PREAD;
        cbl_r[i]->aio_buf = (uint64_t)bl[i+n_write];
        cbl_r[i]->aio_nbytes = b_length;
        cbl_r[i]->aio_offset = b_length * i;
    }
    
    for (size_t i = 0; i < n_write; ++i) {
        std::cout << (char *)(cbl[i]->aio_buf) << std::endl;
    }
    for (size_t i = 0; i < n_read; ++i) {
        std::cout << (char *)(cbl_r[i]->aio_buf) << std::endl;
    }

    ret = io_submit(ctx, n_write, cbl);
    if (ret < 0) {
        perror("io_submit");
        return -1;
    }
    ret = io_submit(ctx, n_read, cbl_r);
    if (ret < 0) {
        perror("io_submit");
        return -1;
    }

    io_event *events = (io_event *)malloc(sizeof(io_event) * n_reqs);\
    timespec timeout = {
        .tv_sec = 1,
        .tv_nsec = 0
    };
    ret = io_getevents(ctx, n_reqs, n_reqs, events, &timeout);
    if (ret < 0) {
        perror("io_getevents");
        return -1;
    }
    for (size_t i = 0; i < n_reqs; ++i)
    {
        std::cout << events[i].res << "\t" << events[i].res2 << "\t" 
        << events[i].data << events[i].obj << std::endl;
    }

    for (size_t i = 0; i < n_reqs; ++i)
    {
        std::cout << bl[i] << std::endl;
    }

    ret = io_destroy(ctx);
    if (ret < 0) {
        perror("io_destroy");
        return -1;
    }

    free(bl);
    free(cbl);
    free(events);
    return 0;
}