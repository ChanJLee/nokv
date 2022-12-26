#include <iostream>
#include "../../main/cpp/nokv.h"
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
const char* demo_str = "x123456789";

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
        char key[6] = {0};
        sprintf(key, "%011d", i);
        kv->put_string(key, demo_str);
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
        ScopedLock<KV, true> lock(*kv);
        char key[12] = {0};
        sprintf(key, "%011d", i);
        const char *v = nullptr;
        if (kv->get_string(key, v) == 0) {
            ++count;
            if (strncmp(v, demo_str, 11)) {
                std::cerr << "check value failed -> [" << v << "][" << demo_str << "]" << std::endl;
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
        char key[6] = {0};
        sprintf(key, "%011d", i);
        {
            ScopedLock<KV, false> lock(*kv);
            kv->reload_if();
            kv->put_string(key, demo_str);
            kv->flush();
        }
        {
            ScopedLock<KV, false> lock(*kv);
            kv->reload_if();
            kv->remove(key);
            kv->flush();
        }
    }

    nokv::KV::destroy(kv);
    exit(0);
}

int main(int argc, char *argv[])
{
    int total = 10000;
    if (total >= 100000)
    {
        std::cerr << "total too large" << std::endl;
        return -1;
    }

    int sub_size = 10;
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

    std::cout << "start load kv" << std::endl;
    nokv::KV::init(argv[1]);
    nokv::KV *kv = nokv::KV::create(argv[2]);
    if (kv == nullptr)
    {
        std::cout << "load kv failed" << std::endl;
        exit(1);
    }

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
        std::cerr << "check kv count failed" << set.size() << "->" << total << std::endl;
        exit(1);
    }

    for (int i = 0; i < total; ++i)
    {
        char key[6] = {0};
        sprintf(key, "%011d", i);
        const char* v =  nullptr;
        if (kv->get_string(key, v))
        {
            std::cerr << "check key: " << key << " failed" << std::endl;
            exit(1);
        }

        if (strncmp(v, demo_str, 11))
        {
            std::cerr << "check key: " << key << "'s value failed" << std::endl;
            exit(1);
        }
    }

    std::cout << "pass" << std::endl;
    nokv::KV::destroy(kv);
    return 0;
}