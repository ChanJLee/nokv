#include "mm.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

namespace mm {

    Memory *Memory::create(const std::string &path, size_t size) {
        int fd = open(path.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd < 0) {
            perror("open");
            return nullptr;
        }

        // 2. 设置文件大小
        if (ftruncate(fd, size) == -1) {
            perror("ftruncate");
            close(fd);
            return nullptr;
        }

        // 3. mmap 文件
        ShmMutex* shm_ptr = (ShmMutex*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (shm_ptr == MAP_FAILED) {
            perror("mmap");
            close(fd);
            return nullptr;
        }

        close(fd); // mmap 后可以关闭 fd

        // 4. 安全初始化 mutex
        // 只有第一个进程初始化，使用原子操作防止 race
        int expected = 0;
        if (__sync_bool_compare_and_swap(&shm_ptr->initialized, expected, 1)) {
            pthread_mutexattr_t attr;
            if (pthread_mutexattr_init(&attr) != 0) {
                perror("pthread_mutexattr_init");
                return nullptr;
            }
            if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
                perror("pthread_mutexattr_setpshared");
                return nullptr;
            }
            if (pthread_mutex_init(&shm_ptr->mutex, &attr) != 0) {
                perror("pthread_mutex_init");
                return nullptr;
            }
            pthread_mutexattr_destroy(&attr);

            shm_ptr->counter = 0;
            printf("Process %d initialized shared memory.\n", getpid());
        } else {
            // 等待初始化完成
            while (shm_ptr->initialized != 1) {
                usleep(1000);
            }
            printf("Process %d attached to existing shared memory.\n", getpid());
        }

        return new Memory(shm_ptr, size);
    }
}