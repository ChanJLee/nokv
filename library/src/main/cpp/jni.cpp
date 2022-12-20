#include <jni.h>
#include "nokv.h"
#include "jni_utils.h"
#include "kv_lock.h"
#include "kv_log.h"

#define AS_STR(type) #type

#define TO_BOX_TYPE(type, sym_type) \
    jclass type##_clazz = env->FindClass("java/lang/" AS_STR(type)); \
    jmethodID type##_valueof = env->GetStaticMethodID(type##_clazz, "valueOf", "(" AS_STR(sym_type) ")Ljava/lang/"  AS_STR(type) ";");

// todo
#define GET_VALUE_FROM_KV(type) \
    int code = kv->get_##type(k, v); \
    if (code == ERROR_CACHE_INVALID) { \
       kv->reload_if(); \
    } \
    code = kv->get_##type(k, v); \

extern "C"
JNIEXPORT jlong JNICALL
Java_me_chan_nkv_NoKV_nativeCreate(JNIEnv *env, jclass clazz, jstring ws) {
    DEF_C_STR(env, ws, ws_path);
    return (jlong) nokv::KV::create(ws_path);
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_chan_nkv_NoKV_nativeInit(JNIEnv *env, jclass clazz, jstring ws) {
    DEF_C_STR(env, ws, meta);
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
    nokv::ScopedLock<nokv::KV, true> lock(*kv);
    return kv->contains(k) == 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_me_chan_nkv_NoKV_nativeGetInt(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                   jint def_value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    nokv::kv_int32_t v = 0;
    nokv::ScopedLock<nokv::KV, true> lock(*kv);
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
    nokv::ScopedLock<nokv::KV, true> lock(*kv);
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
    nokv::ScopedLock<nokv::KV, true> lock(*kv);

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
    nokv::ScopedLock<nokv::KV, true> lock(*kv);
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

    const char *v = NULL;
    nokv::ScopedLock<nokv::KV, true> lock(*kv);

    int code = 0;
    if ((code = kv->get_string(k, v)) < 0) {
        return def_value;
    }
    return code == nokv::VALUE_NULL ? nullptr : env->NewStringUTF(v);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_me_chan_nkv_NoKV_nativeGetStringSet(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                         jobject def_values) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    nokv::kv_array_t v = {};
    nokv::ScopedLock<nokv::KV, true> lock(*kv);

    int code = 0;
    if ((code = kv->get_array(k, v)) < 0) {
        return def_values;
    }

    if (code == nokv::VALUE_NULL) {
        return nullptr;
    }

    jclass set_clazz = env->FindClass("java/util/HashSet");
    jmethodID set_ctor = env->GetMethodID(set_clazz, "<init>", "()V");
    jmethodID add_method = env->GetMethodID(env->FindClass("java/util/Set"), "add",
                                            "(Ljava/lang/Object;)Z");

    jobject set = env->NewObject(set_clazz, set_ctor);
    nokv::kv_array_t::iterator it = v.it();
    nokv::Entry ent;
    while (it.next(&ent)) {
        auto t = ent.type();
        if (t == nokv::TYPE_NULL) {
            env->CallBooleanMethod(set, add_method, (jobject) nullptr);
        } else if (t == nokv::TYPE_STRING) {
            env->CallBooleanMethod(set, add_method,
                                   env->NewStringUTF(ent.as_string().str_));
        } else {
            // todo invalid state
            LOGD("read map invalid state");
        }
    }
    return set;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativeBeginTransaction(JNIEnv *env, jclass clazz, jlong ptr) {
    auto kv = (nokv::KV *) ptr;
    kv->lock(false);
    return kv->reload_if();
}

extern "C"
JNIEXPORT void JNICALL
Java_me_chan_nkv_NoKvEditor_nativeEndTransaction(JNIEnv *env, jclass clazz, jlong ptr) {
    auto kv = (nokv::KV *) ptr;
    kv->flush();
    kv->unlock(false);
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

    if (value == nullptr) {
        return kv->put_null(k) == 0;
    }

    nokv::kv_array_t array = {};
    if (nokv::kv_array_t::create(array)) {
        return false;
    }

    jclass set_clazz = env->FindClass("java/util/Set");
    jclass it_clazz = env->FindClass("java/util/Iterator");
    jmethodID iterator_method = env->GetMethodID(set_clazz, "iterator", "()Ljava/util/Iterator;");
    jmethodID hasNext_method = env->GetMethodID(it_clazz, "hasNext", "()Z");
    jmethodID next_method = env->GetMethodID(it_clazz, "next", "()Ljava/lang/Object;");

    jobject it = env->CallObjectMethod(value, iterator_method);
    while (env->CallBooleanMethod(it, hasNext_method)) {
        jobject elem = env->CallObjectMethod(it, next_method);
        if (elem != nullptr) {
            jstring js = static_cast<jstring>(elem);
            DEF_C_STR(env, js, s);
            array.put_string(s);
        } else {
            array.put_null();
        }
    }

    int code = kv->put_array(k, array);
    nokv::kv_array_t::free(array);
    return code == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutInteger(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                             jint value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    return kv->put_int32(k, value) == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutLong(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                          jlong value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    return kv->put_int64(k, value) == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutFloat(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                           jfloat value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    return kv->put_float(k, value) == 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutBoolean(JNIEnv *env, jclass clazz, jlong ptr, jstring key,
                                             jboolean value) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    return kv->put_boolean(k, value) == 0;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_me_chan_nkv_NoKV_nativeGetAll(JNIEnv *env, jclass clazz, jlong ptr) {
    auto kv = (nokv::KV *) ptr;

    jclass map_clazz = env->FindClass("java/util/HashMap");
    jmethodID map_ctor = env->GetMethodID(map_clazz, "<init>", "()V");
    jmethodID put_method = env->GetMethodID(env->FindClass("java/util/Map"), "put",
                                            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jobject map = env->NewObject(map_clazz, map_ctor);
    TO_BOX_TYPE(Boolean, Z)
    TO_BOX_TYPE(Integer, I)
    TO_BOX_TYPE(Float, F)
    TO_BOX_TYPE(Long, J)

    jclass set_clazz = env->FindClass("java/util/HashSet");
    jmethodID set_ctor = env->GetMethodID(set_clazz, "<init>", "()V");
    jmethodID add_method = env->GetMethodID(env->FindClass("java/util/Set"), "add",
                                            "(Ljava/lang/Object;)Z");

    nokv::ScopedLock<nokv::KV, true> lock(*kv);
    kv->read_all([&](const nokv::kv_string_t &key, nokv::Entry *entry) {
        nokv::kv_type_t type = entry->type();
        if (type == nokv::TYPE_NULL) {
            env->CallObjectMethod(map, put_method, env->NewStringUTF(key.str_), (jobject) nullptr);
        } else if (type == nokv::TYPE_INT32) {
            env->CallObjectMethod(map, put_method, env->NewStringUTF(key.str_),
                                  env->CallStaticObjectMethod(Integer_clazz, Integer_valueof,
                                                              entry->as_int32()));
        } else if (type == nokv::TYPE_FLOAT) {
            env->CallObjectMethod(map, put_method, env->NewStringUTF(key.str_),
                                  env->CallStaticObjectMethod(Float_clazz, Float_valueof,
                                                              entry->as_float()));
        } else if (type == nokv::TYPE_INT64) {
            env->CallObjectMethod(map, put_method, env->NewStringUTF(key.str_),
                                  env->CallStaticObjectMethod(Long_clazz, Long_valueof,
                                                              entry->as_int64()));
        } else if (type == nokv::TYPE_BOOLEAN) {
            env->CallObjectMethod(map, put_method, env->NewStringUTF(key.str_),
                                  env->CallStaticObjectMethod(Boolean_clazz, Boolean_valueof,
                                                              entry->as_boolean()));
        } else if (type == nokv::TYPE_STRING) {
            env->CallObjectMethod(map, put_method, env->NewStringUTF(key.str_),
                                  env->NewStringUTF(entry->as_string().str_));
        } else if (type == nokv::TYPE_ARRAY) {
            jobject set = env->NewObject(set_clazz, set_ctor);
            auto array = entry->as_array();
            nokv::kv_array_t::iterator it = array.it();
            nokv::Entry ent;
            while (it.next(&ent)) {
                auto t = ent.type();
                if (t == nokv::TYPE_NULL) {
                    env->CallBooleanMethod(set, add_method, (jobject) nullptr);
                } else if (t == nokv::TYPE_STRING) {
                    env->CallBooleanMethod(set, add_method,
                                           env->NewStringUTF(ent.as_string().str_));
                } else {
                    // todo invalid state
                    LOGD("read map invalid state");
                }
            }
            env->CallObjectMethod(map, put_method, env->NewStringUTF(key.str_), set);
        }
    });
    return map;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_me_chan_nkv_NoKvEditor_nativePutNull(JNIEnv *env, jclass clazz, jlong ptr, jstring key) {
    auto kv = (nokv::KV *) ptr;
    DEF_C_STR(env, key, k);

    return kv->put_null(k) == 0;
}