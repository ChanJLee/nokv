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
    nokv::KV::init(argv[1]);
    std::cout << "create kv" << std::endl;
    nokv::KV *kv = nokv::KV::create(argv[2]);
    std::cout << "write" << std::endl;
    kv->put_string("string", "hello world");
//    kv->put_boolean("boolean", true);
//    kv->put_float("float", 2);
//    kv->put_int32("int32", 0x12345678);
//    kv->put_int64("int64", 0x1234567878563412);
//    std::cout << "read all" << std::endl;
//    kv->read_all([=](const char *const key, nokv::Entry *entry) {
//        printf("key: %s, value: ", key);
//        switch (entry->type()) {
//            case nokv::TYPE_INT64: {
//                nokv::kv_int64_t v = entry->as_int64();
//                printf("0x%llx", v);
//                break;
//            }
//            case nokv::TYPE_INT32: {
//                nokv::kv_int32_t v = entry->as_int32();
//                printf("0x%x", v);
//                break;
//            }
//            case nokv::TYPE_FLOAT: {
//                nokv::kv_float_t v = entry->as_float();
//                printf("%f", v);
//                break;
//            }
//            case nokv::TYPE_BOOLEAN: {
//                nokv::kv_boolean_t v = entry->as_boolean();
//                printf("%d", v);
//                break;
//            }
//            case nokv::TYPE_STRING: {
//                const char *v = entry->as_string().str_;
//                printf("%s", v);
//                break;
//            }
//        }
//        printf("\n");
//    });
//    std::cout << "destroy" << std::endl;
    nokv::KV::destroy(kv);
    return 0;
}