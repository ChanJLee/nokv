#include <iostream>
#include "../../main/cpp/core.h"
#include <string>
#include <inttypes.h>

template<class T, class O>
O test_cast() {
    T v1 = 0;
    O v2;
    return v2 = v1;
}

int main(int argc, char *argv[]) {



    std::cout << "init" << std::endl;
    nkv::init(argv[1]);
    std::cout << "create kv" << std::endl;
    nkv::KV *kv = nkv::KV::create(argv[2]);
    std::cout << "write" << std::endl;
    kv->write("string", "hello world");
    kv->write("boolean", true);
    kv->write("float", 2);
    kv->write("int32", 0x12345678);
    kv->write<nkv::kv_int64_t>("int64", 0x1234567878563412);
    std::cout << "read all" << std::endl;
    kv->read_all([=](const char *const key, const nkv::byte *value, nkv::byte type, size_t size) {
        printf("key: %s, value: ", key);
        switch (type) {
            case nkv::TYPE_INT64: {
                nkv::kv_int64_t v = 0;
                kv->read(key, v);
                printf("0x%llx", v);
                break;
            }
            case nkv::TYPE_INT32: {
                nkv::kv_int32_t v = 0;
                kv->read(key, v);
                printf("0x%x", v);
                break;
            }
            case nkv::TYPE_FLOAT: {
                nkv::kv_float_t v = 0;
                kv->read(key, v);
                printf("%f", v);
                break;
            }
            case nkv::TYPE_BOOLEAN: {
                nkv::kv_boolean_t v = 0;
                kv->read(key, v);
                printf("%d", v);
                break;
            }
            case nkv::TYPE_STRING: {
                const char *v = 0;
                kv->read(key, v);
                printf("%s", v);
                break;
            }
        }
        printf("\n");
    });
    std::cout << "destroy" << std::endl;
    nkv::KV::destroy(kv);
    return 0;
}