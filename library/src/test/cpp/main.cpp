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
    kv->write_string("string", "hello world");
    kv->write_boolean("boolean", true);
    kv->write_float("float", 2);
    kv->write_int32("int32", 0x12345678);
    kv->write_int64("int64", 0x1234567878563412);
    std::cout << "read all" << std::endl;
    kv->read_all([=](const char *const key, const nkv::byte * value, nkv::byte type, size_t size) {
        std::string s((char* ) value, size);
        std::cout << "key: " << key << "| type:" << type << "| value: " << s << std::endl;
    });
    std::cout << "destroy" << std::endl;
    nkv::KV::destroy(kv);
    return 0;
}