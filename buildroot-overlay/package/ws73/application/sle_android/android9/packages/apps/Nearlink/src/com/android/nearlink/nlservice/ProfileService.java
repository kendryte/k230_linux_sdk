/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import android.app.Service;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.os.IBinder;
import android.util.Log;

import com.android.nearlink.Utils;

/**
 * Base class for a background service that runs a Nearlink profile
 */
public abstract class ProfileService extends Service {
    private static final boolean DEBUG = true;

    public static final String NEARLINK_ADMIN_PERM = android.Manifest.permission.NEARLINK_ADMIN;
    public static final String NEARLINK_PERM = android.Manifest.permission.NEARLINK;
    public static final String NEARLINK_PRIVILEGED =
            android.Manifest.permission.NEARLINK_PRIVILEGED;

    public interface IProfileServiceBinder extends IBinder {
        /**
         * Called in {@link #onDestroy()}
         */
        void cleanup();
    }

    //Profile services will not be automatically restarted.
    //They must be explicitly restarted by AdapterService
    private static final int PROFILE_SERVICE_MODE = Service.START_NOT_STICKY;
    private NearlinkAdapter mAdapter;
    private IProfileServiceBinder mBinder;
    private final String mName;
    private AdapterService mAdapterService;
    private boolean mProfileStarted = false;

    public final String getName() {
        return getClass().getSimpleName();
    }

    protected boolean isAvailable() {
        return mProfileStarted;
    }

    /**
     * Called in {@link #onCreate()} to init binder interface for this profile service
     *
     * @return initialized binder interface for this profile service
     */
    protected abstract IProfileServiceBinder initBinder();

    /**
     * Called in {@link #onCreate()} to init basic stuff in this service
     */
    protected void create() {
    }

    /**
     * Called in {@link #onStartCommand(Intent, int, int)} when the service is started by intent
     *
     * @return True in successful condition, False otherwise
     */
    protected abstract boolean start();

    /**
     * Called in {@link #onStartCommand(Intent, int, int)} when the service is stopped by intent
     *
     * @return True in successful condition, False otherwise
     */
    protected abstract boolean stop();

    /**
     * Called in {@link #onDestroy()} when this object is completely discarded
     */
    protected void cleanup() {
    }

    protected ProfileService() {
        mName = getName();
    }

    @Override
    public void onCreate() {
        if (DEBUG) {
            Log.d(mName, "onCreate");
        }
        super.onCreate();
        mAdapter = NearlinkAdapter.getDefaultAdapter();
        mBinder = initBinder();
        create();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DEBUG) {
            Log.d(mName, "onStartCommand()");
        }

        if (checkCallingOrSelfPermission(NEARLINK_ADMIN_PERM) != PackageManager.PERMISSION_GRANTED) {
            Log.e(mName, "Permission denied!");
            return PROFILE_SERVICE_MODE;
        }

        if (intent == null) {
            Log.d(mName, "onStartCommand ignoring null intent.");
            return PROFILE_SERVICE_MODE;
        }

        String action = intent.getStringExtra(AdapterService.EXTRA_ACTION);
        if (AdapterService.ACTION_SERVICE_STATE_CHANGED.equals(action)) {
            int state = intent.getIntExtra(NearlinkAdapter.EXTRA_STATE, NearlinkAdapter.ERROR);
            if (state == NearlinkAdapter.STATE_OFF) {
                doStop();
            } else if (state == NearlinkAdapter.STATE_ON) {
                doStart();
            }
        }
        return PROFILE_SERVICE_MODE;
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (DEBUG) {
            Log.d(mName, "onBind");
        }
        if (mAdapter != null && mBinder == null) {
            // initBinder returned null, you can't bind
            throw new UnsupportedOperationException("Cannot bind to " + mName);
        }
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        if (DEBUG) {
            Log.d(mName, "onUnbind");
        }
        return super.onUnbind(intent);
    }

    /**
     * Support dumping profile-specific information for dumpsys
     *
     * @param sb StringBuilder from the profile.
     */
    public void dump(StringBuilder sb) {
        sb.append("\nProfile: ");
        sb.append(mName);
        sb.append("\n");
    }

    /**
     * Append an indented String for adding dumpsys support to subclasses.
     *
     * @param sb StringBuilder from the profile.
     * @param s  String to indent and append.
     */
    public static void println(StringBuilder sb, String s) {
        sb.append("  ");
        sb.append(s);
        sb.append("\n");
    }

    @Override
    public void onDestroy() {
        cleanup();
        if (mBinder != null) {
            mBinder.cleanup();
            mBinder = null;
        }
        mAdapter = null;
        super.onDestroy();
    }

    private void doStart() {
        Log.d(mName, "doStart()");
        if (mAdapter == null) {
            Log.w(mName, "Can't start profile service: device does not have BT");
            return;
        }

        mAdapterService = AdapterService.getAdapterService();
        if (mAdapterService == null) {
            Log.w(mName, "Could not add this profile because AdapterService is null.");
            return;
        }
        mAdapterService.addProfile(this);

        mProfileStarted = start();
        if (!mProfileStarted) {
            Log.e(mName, "Error starting profile. start() returned false.");
            return;
        }
        mAdapterService.onProfileServiceStateChanged(this, NearlinkAdapter.STATE_ON);
    }

    private void doStop() {
        Log.d(mName, "doStop()");
        if (!mProfileStarted) {
            Log.w(mName, "doStop() called, but the profile is not running.");
        }
        mProfileStarted = false;
        if (mAdapterService != null) {
            mAdapterService.onProfileServiceStateChanged(this, NearlinkAdapter.STATE_OFF);
        }
        if (!stop()) {
            Log.e(mName, "Unable to stop profile");
        }
        if (mAdapterService != null) {
            mAdapterService.removeProfile(this);
        }

        stopSelf();
    }

    protected NearlinkDevice getDevice(byte[] address) {
        if (mAdapter != null) {
            return mAdapter.getRemoteDevice(Utils.getAddressStringFromByte(address));
        }
        return null;
    }
}
