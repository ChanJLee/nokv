//
// Created by chan on 2022/11/23.
//

#ifndef NKV_JNI_UTILS_H
#define NKV_JNI_UTILS_H

#include <jni.h>

struct ScopeCString {
private:
    JNIEnv *mEnv;
    const jstring mJString;
    const char *mCString;

public:
    ScopeCString(JNIEnv *env, const jstring str) : mEnv(env), mJString(str) {
        if (mJString != NULL) {
            mCString = mEnv->GetStringUTFChars(mJString, 0);
        }
    }

    virtual ~ScopeCString() {
        if (mCString != NULL) {
            mEnv->ReleaseStringUTFChars(mJString, mCString);
        }
    }

    const char *getCString() const {
        return mCString;
    }

    const jstring &getJString() const {
        return mJString;
    }
};

struct ScopeJString {
private:
    JNIEnv *mEnv;
    jstring mJString;
    const char *mCString;

public:
    ScopeJString(JNIEnv *env, const char *str) : mEnv(env), mCString(str) {
        if (mCString != NULL) {
            mJString = mEnv->NewStringUTF(mCString);
        }
    }

    virtual ~ScopeJString() {
        if (mCString != NULL) {
            mEnv->DeleteLocalRef(mJString);
        }
    }

    const char *getCString() const {
        return mCString;
    }

    jstring &getJString() {
        return mJString;
    }
};

#define DEF_NAME(prefix, suffix) prefix##suffix

#define DEF_C_STR(env, jstr, cstr) \
    ScopeCString DEF_NAME(___scope_str_, jstr)(env, jstr); \
    const char* cstr = DEF_NAME(___scope_str_, jstr).getCString()

#define DEF_J_STR(env, cstr, jstr) \
    ScopeJString DEF_NAME(___scope_str_, cstr)(env, cstr); \
    jstring jstr = DEF_NAME(___scope_str_, cstr).getJString()

#endif //NKV_JNI_UTILS_H
