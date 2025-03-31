/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import android.app.Application;
import android.util.Log;

public class AdapterApp extends Application {
    private static final String TAG = "NearlinkAdapterApp";
    private static final boolean DBG = false;
    //For Debugging only
    private static int sRefCount = 0;

    static {
        Log.e(TAG, "Loading JNI Library");
        System.loadLibrary("nearlink_jni");
        Log.e(TAG, "Loading JNI Library End");
    }

    public AdapterApp() {
        super();
        if (DBG) {
            synchronized (AdapterApp.class) {
                sRefCount++;
                Log.d(TAG, "REFCOUNT: Constructed " + this + " Instance Count = " + sRefCount);
            }
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if (DBG) {
            Log.d(TAG, "onCreate");
        }
        Config.init(this);
    }

    @Override
    protected void finalize() {
        if (DBG) {
            synchronized (AdapterApp.class) {
                sRefCount--;
                Log.d(TAG, "REFCOUNT: Finalized: " + this + ", Instance Count = " + sRefCount);
            }
        }
    }
}
