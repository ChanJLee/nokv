#include <iostream>
#include "../../main/cpp/core.h"

int main(int argc, char *argv[]) {
    nkv::init(argv[1]);
    nkv::KV *kv = nkv::KV::create(argv[2]);
    kv->write_boolean("boolean", true);
    kv->write_float("float", 2);
    kv->write_int32("int32", 0x12345678);
    kv->write_int64("int64", 0x1234567878563412);
    kv->write_string("string", "hello world");
    nkv::KV::destroy(kv);
    return 0;
}