#include <iostream>
#include "../../main/cpp/core.h"
#include <string>
#include <inttypes.h>

template<class I, class O>
O test_cast() {
    I v1 = 0;
    O v2 = 1;
    return v2 = (O) (v1);
}

int main(int argc, char *argv[]) {
//    test_cast<nokv::kv_boolean_t , nokv::kv_float_t>();
//    test_cast<nokv::kv_boolean_t , nokv::kv_int32_t>();
//    test_cast<nokv::kv_boolean_t , nokv::kv_int64_t>();
//
//    test_cast<nokv::kv_float_t , nokv::kv_boolean_t>();
//    test_cast<nokv::kv_float_t , nokv::kv_int32_t>();
//    test_cast<nokv::kv_float_t , nokv::kv_int64_t>();
//
//    test_cast<nokv::kv_int32_t , nokv::kv_float_t>();
//    test_cast<nokv::kv_int32_t , nokv::kv_boolean_t>();
//    test_cast<nokv::kv_int32_t , nokv::kv_int64_t>();
//
//    test_cast<nokv::kv_int64_t , nokv::kv_float_t>();
//    test_cast<nokv::kv_int64_t , nokv::kv_int32_t>();
//    test_cast<nokv::kv_int64_t , nokv::kv_boolean_t>();

    std::cout << "init" << std::endl;
    nokv::init(argv[1]);
    std::cout << "create kv" << std::endl;
    nokv::KV *kv = nokv::KV::create(argv[2]);
    std::cout << "write" << std::endl;
    kv->write("string", "hello world");
    kv->write("boolean", true);
    kv->write("float", 2);
    kv->write("int32", 0x12345678);
    kv->write<nokv::kv_int64_t>("int64", 0x1234567878563412);
    std::cout << "read all" << std::endl;
    kv->read_all([=](const char *const key, const nokv::byte *value, nokv::byte type, size_t size) {
        printf("key: %s, value: ", key);
        switch (type) {
            case nokv::TYPE_INT64: {
                nokv::kv_int64_t v = 0;
                kv->read(key, v);
                printf("0x%llx", v);
                break;
            }
            case nokv::TYPE_INT32: {
                nokv::kv_int32_t v = 0;
                kv->read(key, v);
                printf("0x%x", v);
                break;
            }
            case nokv::TYPE_FLOAT: {
                nokv::kv_float_t v = 0;
                kv->read(key, v);
                printf("%f", v);
                break;
            }
            case nokv::TYPE_BOOLEAN: {
                nokv::kv_boolean_t v = 0;
                kv->read(key, v);
                printf("%d", v);
                break;
            }
            case nokv::TYPE_STRING: {
                const char *v = 0;
                kv->read(key, v);
                printf("%s", v);
                break;
            }
        }
        printf("\n");
    });
    std::cout << "destroy" << std::endl;
    nokv::KV::destroy(kv);
    return 0;
}