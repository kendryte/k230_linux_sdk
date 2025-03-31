/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import android.content.Context;
import android.util.Log;

/**
 * LocalNearlinkManager provides a simplified interface on top of a subset of
 * the Nearlink API. Note that {@link #getInstance} will return null
 * if there is no Nearlink adapter on this device, and callers must be
 * prepared to handle this case.
 *
 * @since 2023-12-04
 */
public class LocalNearlinkManager {
    private static final String TAG = "LocalNearlinkManager";

    /**
     * Singleton instance.
     */
    private static LocalNearlinkManager sInstance;

    private final Context mContext;

    /**
     * If a NL-related activity is in the foreground, this will be it.
     */
    private Context mForegroundActivity;

    private final LocalNearlinkAdapter mLocalAdapter;

    private final CachedNearlinkDeviceManager mCachedDeviceManager;

    /**
     * The Nearlink profile manager.
     */
    private final LocalNearlinkProfileManager mProfileManager;

    /**
     * The broadcast receiver event manager.
     */
    private final NearlinkEventManager mEventManager;

    public static synchronized LocalNearlinkManager getInstance(Context context,
                                                                NearlinkManagerCallback onInitCallback) {
        if (sInstance == null) {
            LocalNearlinkAdapter adapter = LocalNearlinkAdapter.getInstance();
            if (adapter == null) {
                return null;
            }
            // This will be around as long as this process is
            Context appContext = context.getApplicationContext();
            sInstance = new LocalNearlinkManager(adapter, appContext);
            if (onInitCallback != null) {
                onInitCallback.onNearlinkManagerInitialized(appContext, sInstance);
            }
        }

        return sInstance;
    }

    private LocalNearlinkManager(LocalNearlinkAdapter adapter, Context context) {
        mContext = context;
        mLocalAdapter = adapter;

        mCachedDeviceManager = new CachedNearlinkDeviceManager(context, this);
        mEventManager = new NearlinkEventManager(mLocalAdapter,
                mCachedDeviceManager, context);
        mProfileManager = new LocalNearlinkProfileManager(context,
                mLocalAdapter, mCachedDeviceManager, mEventManager);
        mEventManager.readPairedDevices();
    }

    public LocalNearlinkAdapter getNearlinkAdapter() {
        return mLocalAdapter;
    }

    public Context getContext() {
        return mContext;
    }

    public Context getForegroundActivity() {
        return mForegroundActivity;
    }

    public boolean isForegroundActivity() {
        return mForegroundActivity != null;
    }

    public synchronized void setForegroundActivity(Context context) {
        if (context != null) {
            Log.d(TAG, "setting foreground activity to non-null context");
            mForegroundActivity = context;
        } else {
            if (mForegroundActivity != null) {
                Log.d(TAG, "setting foreground activity to null");
                mForegroundActivity = null;
            }
        }
    }

    public CachedNearlinkDeviceManager getCachedDeviceManager() {
        return mCachedDeviceManager;
    }

    public NearlinkEventManager getEventManager() {
        return mEventManager;
    }

    public LocalNearlinkProfileManager getProfileManager() {
        return mProfileManager;
    }

    public interface NearlinkManagerCallback {
        void onNearlinkManagerInitialized(Context appContext,
                                          LocalNearlinkManager NearlinkManager);
    }
}
