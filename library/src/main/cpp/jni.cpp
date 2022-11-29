#include <jni.h>
#include "core.h"
#include "scoped_str.h"
#include "lock.h"

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
    return nokv::KV::init(meta);
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
    nokv::ScopedLock<nokv::KV> lock(*kv);
    return kv->contains(k);
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_chan_nkv_NoKV_nativeGetInt(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                   jint def_value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);
    nokv::kv_int32_t v = 0;

    nokv::ScopedLock<nokv::KV> lock(*kv);
    if (kv->get_int32(k, v)) {
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
    nokv::kv_boolean_t v = false;

    nokv::ScopedLock<nokv::KV> lock(*kv);
    if (kv->get_boolean(k, v)) {
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
    nokv::ScopedLock<nokv::KV> lock(*kv);

    if (kv->get_int64(k, v)) {
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

    nokv::ScopedLock<nokv::KV> lock(*kv);
    if (kv->get_float(k, v)) {
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

    nokv::kv_string_t v = {};
    nokv::ScopedLock<nokv::KV> lock(*kv);

    int code = 0;
    if ((code = kv->get_string(k, v)) < 0) {
        return def_value;
    }
    return code == nokv::VALUE_NULL ? nullptr : env->NewStringUTF(v.str_);
}

extern "C"
JNIEXPORT void JNICALL
Java_me_chan_nkv_NoKvEditor_nativeBeginTransaction(JNIEnv *env, jclass clazz, jlong ptr) {
    auto kv = (nokv::KV *) ptr;
    kv->lock();
}

extern "C"
JNIEXPORT void JNICALL
Java_me_chan_nkv_NoKvEditor_nativeEndTransaction(JNIEnv *env, jclass clazz, jlong ptr) {
    auto kv = (nokv::KV *) ptr;
    kv->unlock();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativeClear(JNIEnv *env, jclass clazz, jlong ptr) {
    auto kv = (nokv::KV *) ptr;
    return kv->remove_all() == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativeRemove(JNIEnv *env, jclass clazz, jlong ptr, jstring key) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);
    return kv->remove(k) == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutString(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                            jstring value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    if (value == nullptr) {
        return kv->put_null(k) == 0;
    }

    DEF_C_STR(env, value, v);
    return kv->put_string(k, v) == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutStringSet(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                               jobject value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    nokv::kv_array_t array = {};
    if (nokv::kv_array_t::create(array)) {
        return false;
    }

    int code = -1;
    {
        nokv::ScopedLock<nokv::KV> lock(*kv);
        code = kv->put_array(k, array);
    }
    nokv::kv_array_t::free(array);

    // TODO: implement nativePutStringSet()
    return code == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutInteger(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                             jint value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    nokv::ScopedLock<nokv::KV> lock(*kv);
    return kv->put_int32(k, value) == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutLong(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                          jlong value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    nokv::ScopedLock<nokv::KV> lock(*kv);
    return kv->put_int64(k, value) == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutFloat(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                           jfloat value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    nokv::ScopedLock<nokv::KV> lock(*kv);
    return kv->put_float(k, value) == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutBoolean(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                             jboolean value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    nokv::ScopedLock<nokv::KV> lock(*kv);
    return kv->put_boolean(k, value) == 0;
}