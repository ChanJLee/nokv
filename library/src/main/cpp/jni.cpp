#include <jni.h>
#include "core.h"
#include "scoped_str.h"

extern "C"
JNIEXPORT jlong JNICALL
Java_me_chan_nkv_NoKV_nativeCreate(JNIEnv *env, jclass clazz, jstring kv) {
    DEF_C_STR(env, kv, kv_path);
    return (jlong) nkv::KV::create(kv_path);
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_chan_nkv_NoKV_nativeInit(JNIEnv *env, jclass clazz, jstring metaFile) {
    DEF_C_STR(env, metaFile, meta);
    return nkv::init(meta);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_chan_nkv_NoKV_nativeRelease(JNIEnv *env, jclass clazz, jlong ptr) {
    auto kv = (nkv::KV *) ptr;
    nkv::KV::destroy(kv);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKV_nativeContains(JNIEnv *env, jclass clazz, jlong ptr, jstring key) {
    auto kv = (nkv::KV *) ptr;
    DEF_C_STR(env, key, k);
    return kv->contains(k);
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_chan_nkv_NoKV_nativeGetInt(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                   jint def_value) {
    auto kv = (nkv::KV *) ptr;
    DEF_C_STR(env, key, k);
    nkv::byte *mem = nullptr;
    if (kv->read(k, &mem)) {
        return def_value;
    }

    if (mem[0] == nkv::TYPE_STRING) {
        return def_value;
    }



    return def_value;
}