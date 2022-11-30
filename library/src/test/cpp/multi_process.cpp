#include <iostream>
#include "../../main/cpp/core.h"
#include <string>
#include <inttypes.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sys/wait.h>

using namespace nokv;

struct MockData
{
    std::string key;
    int type_;
    union
    {
        kv_boolean_t boolean_;
        kv_float_t float_;
        kv_int32_t int32_;
        kv_int64_t int64_;
        kv_string_t string_;
        kv_array_t array_;
    } data_;

    MockData(const char *key) : key(key) {}
};

#define PUSH_MOCK_DATA(format, t, v, tag) \
    {                                     \
        char key[24] = {0};               \
        sscanf(key, format, &i);          \
        MockData data(key);               \
        data.data_.t##_ = v;              \
        data.type_ = tag;                 \
        vec.push_back(data);              \
    }

#define INSERT_KV(type, tag)                                          \
    {                                                                 \
        if (vec[i].type_ == tag)                                      \
        {                                                             \
            kv->put_##type(vec[i].key.c_str(), vec[i].data_.type##_); \
            continue;                                                 \
        }                                                             \
    }

void subprocess(char *argv[], std::vector<MockData> &vec, int start, int end)
{
    nokv::KV::init(argv[1]);
    nokv::KV *kv = nokv::KV::create(argv[2]);
    std::cout << "ready to write, pid: " << getpid() << std::endl;

    for (int i = start; i < end; ++i)
    {
        ScopedLock<KV> lock(*kv);
        {
            if (vec[i].type_ == 1)
            {
                kv->put_int32(vec[i].key.c_str(), vec[i].data_.int32_);
                continue;
            }
        }
        INSERT_KV(int32, 1);
        INSERT_KV(float, 2);
        INSERT_KV(int64, 3);
        INSERT_KV(boolean, 4);
        INSERT_KV(string, 4);
    }

    std::cout << "finish write, pid: " << getpid() << std::endl;
    nokv::KV::destroy(kv);
    exit(0);
}

int main(int argc, char *argv[])
{
    std::vector<MockData> vec;
    for (int i = 0; i < 1000000; ++i)
    {
        PUSH_MOCK_DATA("key_int32_%d", int32, 1, 1);
        PUSH_MOCK_DATA("key_float_%d", float, 3.5, 2);
        PUSH_MOCK_DATA("key_int64_%d", int64, 2, 3);
        PUSH_MOCK_DATA("key_boolean_%d", boolean, true, 4);
        {
            char key[24] = {0};
            sscanf(key, "key_string_%d", &i);
            MockData data(key);
            data.type_ = 5;
            data.data_.string_.str_ = new char[24];
            strcpy((char *)data.data_.string_.str_, key);
            vec.push_back(data);
        }
    }

    std::vector<pid_t> children;
    for (int i = 0; i < 100; ++i)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            subprocess(argv, vec, i * 10000, (i + 1) * 10000);
        }
        else
        {
            children.push_back(pid);
        }
    }

    int status;
    int w;
    for (auto child : children)
    {
        do
        {
            w = waitpid(child, &status, WUNTRACED | WCONTINUED);
            if (w == -1)
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status))
            {
                printf("exited, status=%d\n", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("killed by signal %d\n", WTERMSIG(status));
            }
            else if (WIFSTOPPED(status))
            {
                printf("stopped by signal %d\n", WSTOPSIG(status));
            }
            else if (WIFCONTINUED(status))
            {
                printf("continued\n");
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    std::cout << "start check result" << std::endl;

    return 0;
}