/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
#ifndef COM_ANDROID_NEARLINK_H
#define COM_ANDROID_NEARLINK_H

#include <nativehelper/JNIHelp.h>
#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/Log.h"
#include "jni.h"
#include "nativehelper/ScopedLocalRef.h"
#include "utils/Log.h"

namespace android {

JNIEnv *getCallbackEnv();

class CallbackEnv {
public:
    explicit CallbackEnv(const char *methodName) : mName(methodName)
    {
        mCallbackEnv = getCallbackEnv();
    }

    ~CallbackEnv()
    {
        if (mCallbackEnv && mCallbackEnv->ExceptionCheck()) {
            ALOGE("An exception was thrown by callback '%s'.", mName);
            LOGE_EX(mCallbackEnv);
            mCallbackEnv->ExceptionClear();
        }
    }

    bool valid() const
    {
        JNIEnv *env = AndroidRuntime::getJNIEnv();
        if (!mCallbackEnv || (mCallbackEnv != env)) {
            ALOGE("%s: Callback env fail: env: %p, callback: %p", mName, env, mCallbackEnv);
            return false;
        }
        return true;
    }

    JNIEnv *operator->() const
    {
        return mCallbackEnv;
    }

    JNIEnv *get() const
    {
        return mCallbackEnv;
    }

private:
    JNIEnv *mCallbackEnv;
    const char *mName;

    DISALLOW_COPY_AND_ASSIGN(CallbackEnv);
};

int register_com_android_nearLink_AdapterService(JNIEnv *env);
int register_com_android_nearLink_AnnounceManager(JNIEnv *env);
int register_com_android_nearLink_SeekManager(JNIEnv *env);
int register_com_android_nearlink_ssap_client(JNIEnv *env);
int register_com_android_nearlink_ConnectionService(JNIEnv *env);
int register_com_android_nearlink_hid_host(JNIEnv *env);

}  // namespace android
#endif  // COM_ANDROID_NEARLINK_H
