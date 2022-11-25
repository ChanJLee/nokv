#include <jni.h>
#include "core.h"
#include "scoped_str.h"

extern "C"
JNIEXPORT jlong JNICALL
Java_me_chan_nkv_NoKV_nativeCreate(JNIEnv *env, jclass clazz, jstring kv) {
    DEF_C_STR(env, kv, kv_path);
    return (jlong) nokv::KV::create(kv_path);
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_chan_nkv_NoKV_nativeInit(JNIEnv *env, jclass clazz, jstring metaFile) {
    DEF_C_STR(env, metaFile, meta);
    return nokv::init(meta);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_chan_nkv_NoKV_nativeRelease(JNIEnv *env, jclass clazz, jlong ptr) {
    auto kv = (nokv::KV *) ptr;
    nokv::KV::destroy(kv);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKV_nativeContains(JNIEnv *env, jclass clazz, jlong ptr, jstring key) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);
    return kv->contains(k);
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_chan_nkv_NoKV_nativeGetInt(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                   jint def_value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);
    nokv::kv_int32_t v = 0;
    if (kv->read(k, v)) {
        return def_value;
    }
    return v;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKV_nativeGetBoolean(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                       jboolean def_value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);
    nokv::kv_boolean_t v = 0;
    if (kv->read(k, v)) {
        return def_value;
    }
    return v;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_me_chan_nkv_NoKV_nativeGetLong(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                    jlong def_value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);
    nokv::kv_int64_t v = 0;
    if (kv->read(k, v)) {
        return def_value;
    }
    return v;
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_me_chan_nkv_NoKV_nativeGetFloat(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                     jfloat def_value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);
    nokv::kv_float_t v = 0;
    if (kv->read(k, v)) {
        return def_value;
    }
    return v;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_me_chan_nkv_NoKV_nativeGetString(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                      jstring def_value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);
    nokv::kv_string_t v = nullptr;
    if (kv->read(k, v)) {
        return def_value;
    }
    return env->NewStringUTF(v);
}