#include <iostream>
#include "../../main/cpp/core.h"
#include <string>
#include <inttypes.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sys/wait.h>
#include <sstream>
#include <fstream>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stddef.h>
#include <set>

void print_trace(void)
{
    void *array[30];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace(array, 30);
    strings = backtrace_symbols(array, size);
    if (NULL == strings)
    {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    printf("Obtained %zd stack frames.\n", size);

    for (i = 0; i < size; i++)
    {
        printf("%s\n", strings[i]);
    }

    free(strings);
    strings = NULL;

    exit(EXIT_SUCCESS);
}

void sighandler_dump_stack(int sig)
{
    psignal(sig, "handler"); // 打印信号相关信息
    print_trace();
    signal(sig, SIG_DFL); // 恢复信号默认处理
    raise(sig);           // 继续后续的流程
}

using namespace nokv;

const char *fuck = "Mr. and Mrs. Dursley, of number four, Privet Drive, were proud to say that they were perfectly normal, thank you very much. They were the last people you'd expect to be involved in anything strange or mysterious, because they just didn't hold with such nonsense."
                   "Mr. Dursley was the director of a firm called Grunnings, which made drills. He was a big, beefy man with hardly any neck, although he did have a very large mustache. Mrs. Dursley was thin and blonde and had nearly twice the usual amount of neck, which came in very useful as she spent so much of her time craning over garden fences, spying on the neighbors. The Dursleys had a small son called Dudley and in their opinion there was no finer boy anywhere."
                   "The Dursleys had everything they wanted, but they also had a secret, and their greatest fear was that somebody would discover it. They didn't think they could bear it if anyone found out about the Potters. Mrs. Potter was Mrs. Dursley's sister, but they hadn't met for several years; in fact, Mrs. Dursley pretended she didn't have a sister, because her sister and her good-for-nothing husband were as unDursleyish as it was possible to be. The Dursleys shuddered to think what the neighbors would say if the Potters arrived in the street. The Dursleys knew that the Potters had a small son, too, but they had never even seen him. This boy was another good reason for keeping the Potters away; they didn't want Dudley mixing with a child like that."
                   "When Mr. and Mrs. Dursley woke up on the dull, gray Tuesday our story starts, there was nothing about the cloudy sky outside to suggest that strange and mysterious things would soon be happening all over the country. Mr. Dursley hummed as he picked out his most boring tie for work, and Mrs. Dursley gossiped away happily as she wrestled a screaming Dudley into his high chair."
                   "None of them noticed a large, tawny owl flutter past the window.";

void write_proc(char *argv[], int start, int end)
{
    if (signal(SIGSEGV, sighandler_dump_stack) == SIG_ERR)
        perror("signal failed");

    nokv::KV::init(argv[1]);
    nokv::KV *kv = nokv::KV::create(argv[2]);
    if (kv == nullptr)
    {
        std::cout << "fuck off, create kv failed, pid: " << getpid() << std::endl;
        exit(SIGBUS);
    }

    std::cout << getpid() << " write from: " << start << " to " << end << std::endl;
    for (int i = start; i < end; ++i)
    {
        ScopedLock<KV, false> lock(*kv);
        kv->reload_if();
        std::stringstream ss;
        ss << "kv_int32_" << i;
        const auto &key = ss.str();
        kv->put_int32(key.c_str(), i);
        kv->flush();
    }

    nokv::KV::destroy(kv);
    std::cout << getpid() << " write finished" << std::endl;
    exit(0);
}

void read_proc(char *argv[], int start, int end)
{
    if (signal(SIGSEGV, sighandler_dump_stack) == SIG_ERR)
        perror("signal failed");

    nokv::KV::init(argv[1]);
    nokv::KV *kv = nokv::KV::create(argv[2]);

    if (kv == nullptr)
    {
        std::cout << "fuck off, create kv failed, pid: " << getpid() << std::endl;
        exit(SIGBUS);
    }

    std::cout << getpid() << " read from: " << start << " to " << end << std::endl;

    int count = 0;
    for (int i = start; i < end; ++i)
    {
        ScopedLock<KV, false> lock(*kv);
        // kv->reload_if(); /* key */
        std::stringstream ss;
        ss << "kv_int32_" << i;
        const auto &key = ss.str();
        nokv::kv_int32_t v;
        if (kv->get_int32(key.c_str(), v) == 0)
        {
            ++count;
            if (v != i)
            {
                std::cerr << "check value failed" << std::endl;
                exit(1);
            }
        }
    }

    nokv::KV::destroy(kv);
    std::cout << getpid() << " read finished, get: " << count << std::endl;
    exit(0);
}

void adj_proc(char *argv[], int start, int end)
{
    if (signal(SIGSEGV, sighandler_dump_stack) == SIG_ERR)
        perror("signal failed");

    nokv::KV::init(argv[1]);
    nokv::KV *kv = nokv::KV::create(argv[2]);

    if (kv == nullptr)
    {
        std::cout << "fuck off, create kv failed, pid: " << getpid() << std::endl;
        exit(SIGBUS);
    }

    std::cout << getpid() << " adj from: " << start << " to " << end << std::endl;

    for (int i = start; i < end; ++i)
    {
        std::stringstream ss;
        ss << "kv_int32_" << i;
        const auto &key = ss.str();
        for (int j = 0; j < 2; ++j)
        {
            ScopedLock<KV, false> lock(*kv);
            kv->reload_if();
            if (j)
            {
                kv->remove(key.c_str());
            }
            else
            {
                kv->put_int32(key.c_str(), 1);
            }
            kv->flush();
        }
    }

    nokv::KV::destroy(kv);
    exit(0);
}

int main(int argc, char *argv[])
{
    int total = 20000;
    int sub_size = 2;
    int step = total / sub_size;

    std::vector<pid_t> children;
    for (int i = 0; i < sub_size; ++i)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            write_proc(argv, i * step, (i + 1) * step);
        }
        else
        {
            children.push_back(pid);
        }
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        read_proc(argv, 0, total);
    }
    else
    {
        children.push_back(pid);
    }

    pid = fork();
    if (pid == 0)
    {
        adj_proc(argv, total, total + step);
    }
    else
    {
        children.push_back(pid);
    }

    int status;
    int w;
    for (int i = 0; i < children.size(); ++i)
    {
        int child = children[i];
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

        if (WIFEXITED(status))
        {
            children[i] = -1;
        }
    }

    std::cout << "start check result" << std::endl;
    for (auto child : children)
    {
        if (child != -1)
        {
            std::cerr << "check child failed: " << child << std::endl;
            exit(SIGTERM);
        }
    }

    nokv::KV::init(argv[1]);
    nokv::KV *kv = nokv::KV::create(argv[2]);

    std::set<std::string> set;
    kv->read_all([&](const nokv::kv_string_t &key, nokv::Entry *entry) -> void
                 { 
        std::string s(key.str_);
        const auto& res = set.insert(s);
        if (!res.second) {
            std::cerr << "check kv entry failed: " << s << std::endl;
            exit(1);
        } });

    if (set.size() != total)
    {
        std::cerr << "check kv count failed" << std::endl;
        exit(1);
    }

    for (int i = 0; i < total; ++i)
    {
        std::stringstream ss;
        ss << "kv_int32_" << i;
        const auto &key = ss.str();
        kv_int32_t v = 0;
        if (kv->get_int32(key.c_str(), v))
        {
            std::cerr << "check key: " << key << " failed" << std::endl;
            exit(1);
        }

        if (v != i)
        {
            std::cerr << "check key: " << key << "'s value failed" << std::endl;
            exit(1);
        }
    }

    std::cout << "pass" << std::endl;
    nokv::KV::destroy(kv);
    return 0;
}