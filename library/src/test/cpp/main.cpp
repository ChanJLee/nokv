#include <iostream>
#include "../../main/cpp/core.h"
#include <string>
#include <inttypes.h>

int main(int argc, char *argv[]) {
    std::cout << "init" << std::endl;
    nkv::init(argv[1]);
    std::cout << "create kv" << std::endl;
    nkv::KV *kv = nkv::KV::create(argv[2]);
    std::cout << "write" << std::endl;
    kv->write<nkv::kv_string_t>("string", "hello world");
    kv->write<nkv::kv_boolean_t>("boolean", true);
    kv->write<nkv::kv_float_t>("float", 2);
    kv->write<nkv::kv_int32_t>("int32", 0x12345678);
    kv->write<nkv::kv_int64_t>("int64", 0x1234567878563412);
    std::cout << "read all" << std::endl;
    kv->read_all([=](const char *const key, const nkv::byte *value, nkv::byte type, size_t size) {
        printf("key: %s, value: ", key);
        switch (type) {
            case nkv::TYPE_INT64: {
                int64_t v = 0;
                kv->read_int64(key, v);
                printf("0x%x", v);
                break;
            }
            case nkv::TYPE_INT32: {
                int32_t v = 0;
                kv->read_int32(key, v);
                printf("0x%x", v);
                break;
            }
            case nkv::TYPE_FLOAT: {
                float v = 0;
                kv->read_float(key, v);
                printf("%f", v);
                break;
            }
            case nkv::TYPE_BOOLEAN: {
                bool v = 0;
                kv->read_boolean(key, v);
                printf("%d", v);
                break;
            }
            case nkv::TYPE_STRING: {
                char* v = 0;
                kv->read_string(key, &v);
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