#include <iostream>
#include "../../main/cpp/core.h"
#include <string>
#include <inttypes.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sys/wait.h>

using namespace nokv;

const char * fuck = "Mr. and Mrs. Dursley, of number four, Privet Drive, were proud to say that they were perfectly normal, thank you very much. They were the last people you'd expect to be involved in anything strange or mysterious, because they just didn't hold with such nonsense."
"Mr. Dursley was the director of a firm called Grunnings, which made drills. He was a big, beefy man with hardly any neck, although he did have a very large mustache. Mrs. Dursley was thin and blonde and had nearly twice the usual amount of neck, which came in very useful as she spent so much of her time craning over garden fences, spying on the neighbors. The Dursleys had a small son called Dudley and in their opinion there was no finer boy anywhere."
"The Dursleys had everything they wanted, but they also had a secret, and their greatest fear was that somebody would discover it. They didn't think they could bear it if anyone found out about the Potters. Mrs. Potter was Mrs. Dursley's sister, but they hadn't met for several years; in fact, Mrs. Dursley pretended she didn't have a sister, because her sister and her good-for-nothing husband were as unDursleyish as it was possible to be. The Dursleys shuddered to think what the neighbors would say if the Potters arrived in the street. The Dursleys knew that the Potters had a small son, too, but they had never even seen him. This boy was another good reason for keeping the Potters away; they didn't want Dudley mixing with a child like that."
"When Mr. and Mrs. Dursley woke up on the dull, gray Tuesday our story starts, there was nothing about the cloudy sky outside to suggest that strange and mysterious things would soon be happening all over the country. Mr. Dursley hummed as he picked out his most boring tie for work, and Mrs. Dursley gossiped away happily as she wrestled a screaming Dudley into his high chair."
"None of them noticed a large, tawny owl flutter past the window.";

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
            data.data_.string_.str_ = fuck;
            vec.push_back(data);
        }
        {
            char key[24] = {0};
            sscanf(key, "key_null_%d", &i);
            MockData data(key);
            data.type_ = 6;
            vec.push_back(data);
        }
        {
            char key[24] = {0};
            sscanf(key, "key_array_%d", &i);
            MockData data(key);
            data.type_ = 7;
            kv_array_t::create(data.data_.array_);
            data.data_.array_.put_string(fuck);
            data.data_.array_.put_null();
            data.data_.array_.put_string(fuck);
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

            if (WIFSIGNALED(status))
            {
                printf("killed by signal %d, pid %d\n", WTERMSIG(status), child);
            }
            else if (WIFSTOPPED(status))
            {
                printf("stopped by signal %d, pid %d\n", WSTOPSIG(status), child);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    std::cout << "start check result" << std::endl;

    return 0;
}