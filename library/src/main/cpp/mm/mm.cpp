#include "mm.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>
#include "../os/lock.h"
#include "../kv_log.h"

namespace mm {

    class ScopedFileLock {
    private:
        int _fd;
        FileLock *_lock;
    public:
        ScopedFileLock(const std::string &path) : _lock(nullptr) {
            _fd = open(path.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, S_IWUSR | S_IRUSR);
            _lock = _fd >= 0 ? new FileLock(_fd) : nullptr;
            if (_lock) {
                _lock->lock(false);
            }
        }

        ~ScopedFileLock() {
            if (_fd >= 0) {
                close(_fd);
                _lock->unlock(false);
                delete _lock;
            }
        }

        const bool valid() const { return _fd >= 0; }

        int fd() const {
            return _fd;
        }
    };

    class ScopedMmap {
        void *_kv;
        size_t _size;
    public:
        ScopedMmap(int fd, size_t size) : _kv(
                mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)), _size(size) {
        }

        ~ScopedMmap() {
            if (_kv != MAP_FAILED) {
                munmap(_kv, _size);
            }
        }

        void *get() const { return _kv; }
    };

    Memory *Memory::create(const std::string &path, size_t size) {
        char boot_id[BOOT_ID_SIZE] = {0};
        if (get_boot_id(boot_id, sizeof(boot_id)) != 0) {
            fprintf(stderr, "Failed to get boot_id\n");
            return nullptr;
        }
        LOGD("boot id %s", boot_id);

        ScopedFileLock file(path);
        if (!file.valid()) {
            LOGD("open file %s failed: %s", path.c_str(), strerror(errno));
            return nullptr;
        }

        int fd = file.fd();
        struct stat st{};
        if (fstat(fd, &st) != 0) {
            return nullptr;
        }

        bool new_file = st.st_size == 0;
        if (new_file) {
            auto pagesize = getpagesize();
            if (size < pagesize) {
                size = pagesize;
            } else if (size % pagesize != 0) {
                size = ((size / pagesize) + 1) * pagesize;
            }

            LOGD("page size %d, resize to %d", pagesize, size);
            st.st_size = size;
            if (ftruncate(fd, st.st_size) != 0) {
                perror("ftruncate");
                return nullptr;
            }
        }

        ScopedMmap m(fd, st.st_size);
        auto nokv = (Nokv *) m.get();
        if (nokv == MAP_FAILED) {
            LOGD("mmap failed: %s", strerror(errno));
            return nullptr;
        }

        LOGD("file boot id %s", nokv->boot_id);
        if (memcmp(nokv->boot_id, boot_id, BOOT_ID_SIZE) != 0) {
            LOGD("init mutex %d", getpid());
            pthread_mutexattr_t attr;
            if (pthread_mutexattr_init(&attr) != 0) {
                LOGD("pthread_mutexattr_init failed");
                return nullptr;
            }
            if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
                LOGD("set pshared failed");
                return nullptr;
            }
#ifdef PTHREAD_MUTEX_ROBUST
            LOGD("set robust");
            if (pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) != 0) {
                LOGD("set robust failed");
                return nullptr;
            }
#endif
            if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
                LOGD("set type failed");
                return nullptr;
            }
            if (pthread_mutex_init(&nokv->mutex, &attr) != 0) {
                perror("pthread_mutex_init");
                return nullptr;
            }
            pthread_mutexattr_destroy(&attr);
            strncpy(nokv->boot_id, boot_id, BOOT_ID_SIZE);
            nokv->boot_id[BOOT_ID_SIZE - 1] = '\0';
            if (msync(nokv, sizeof(Nokv), MS_SYNC) != 0) {
                LOGD("msync failed: %s", strerror(errno));
                return nullptr;
            }
        }

        LOGD("create memory %s, size %d, process %d did it", path.c_str(), size, getpid());
        return new Memory(nokv, size);
    }

    bool Memory::unlock() {
        return pthread_mutex_unlock(&_kv->mutex) == 0;
    }

    bool Memory::lock() {
        return pthread_mutex_lock(&_kv->mutex) == 0;
    }
}