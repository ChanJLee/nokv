#include <iostream>
#include "../../main/cpp/nokv.h"
#include <string>
#include <inttypes.h>
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

int main(int argc, char *argv[]) {
    if (signal(SIGSEGV, sighandler_dump_stack) == SIG_ERR)
        perror("signal failed");

    std::cout << "init" << std::endl;
    nokv::KV::init(argv[1]);
    std::cout << "create kv" << std::endl;
    nokv::KV *kv = nokv::KV::create(argv[2]);
    std::cout << "write" << std::endl;
    kv->put_boolean("boolean", true);
    kv->put_string("string", "hello world2");
    kv->put_float("float", 3.1415926);
    kv->put_int32("int32", 0x123456);
    kv->put_int64("int64", 0xbabeaaaa);
    kv->put_string("suffix", "====");
    kv->put_string("string", "hello world");

    nokv::kv_array_t array;
    nokv::kv_array_t::create(array);

    array.put_string("a1");
    array.put_null();
    array.put_string("a3");

    kv->put_array("array", array);
    kv->put_string("suffix2", "====");
    kv->remove("suffix");
    kv->flush();

    std::cout << "read all, size: " << kv->size() << std::endl;
    kv->read_all([=](const nokv::kv_string_t& key, nokv::Entry *entry) {
        printf("key: %s, value: ", key.str_);
        switch (entry->type()) {
            case nokv::TYPE_INT64: {
                nokv::kv_int64_t v = entry->as_int64();
                printf("0x%llx", v);
                break;
            }
            case nokv::TYPE_INT32: {
                nokv::kv_int32_t v = entry->as_int32();
                printf("0x%x", v);
                break;
            }
            case nokv::TYPE_FLOAT: {
                nokv::kv_float_t v = entry->as_float();
                printf("%f", v);
                break;
            }
            case nokv::TYPE_BOOLEAN: {
                nokv::kv_boolean_t v = entry->as_boolean();
                printf("%d", v);
                break;
            }
            case nokv::TYPE_STRING: {
                const char *v = entry->as_string().str_;
                printf("%s", v);
                break;
            }
            case nokv::TYPE_ARRAY: {
                auto it = entry->as_array().it();
                nokv::Entry item;
                printf("[");
                while (it.next(&item)) {
                    if (item.type() == nokv::TYPE_STRING) {
                        const char *v = item.as_string().str_;
                        printf("%s,", v);
                    } else if (item.type() == nokv::TYPE_NULL) {
                        printf("null,");
                    }
                }
                printf("], size: %d", array.byte_count() - 4);
            }
        }
        printf("\n");
    });
    std::cout << "destroy" << std::endl;
    nokv::KV::destroy(kv);
    return 0;
}