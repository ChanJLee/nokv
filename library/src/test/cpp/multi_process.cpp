#include <iostream>
#include "../../main/cpp/core.h"
#include <string>
#include <inttypes.h>
#include <unistd.h> // for usleep
#include <mutex>

int main(int argc, char *argv[]) {
    nkv::init(argv[1]);
    nkv::KV *kv = nkv::KV::create(argv[2]);

    sleep(5);
    std::cout << "ready to write: " << getpid() << std::endl;
    nkv::kv_int32_t v = 0;
    for (int i = 0; i < 10000; ++i) {
        nkv::ScopedLock<nkv::KV> lock(*kv);
        if (kv->contains("fuck")) {
            if (kv->read("fuck", v)) {
                std::cerr << "read failed" << std::endl;
                return -1;
            }

            kv->write("fuck", ++v);
        } else {
            kv->write("fuck", 0);
        }
    }

    std::cout << "write end: last value: " << v << std::endl;
    nkv::KV::destroy(kv);
    return 0;
}