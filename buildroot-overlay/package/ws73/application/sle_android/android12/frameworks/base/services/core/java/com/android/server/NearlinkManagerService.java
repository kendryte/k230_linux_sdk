/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.server;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.LinkedList;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import android.Manifest;
import android.app.ActivityManager;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.UserInfo;
import android.database.ContentObserver;
import android.nearlink.INearlink;
import android.nearlink.INearlinkCallback;
import android.nearlink.INearlinkConnection;
import android.nearlink.INearlinkDiscovery;
import android.nearlink.INearlinkManager;
import android.nearlink.INearlinkManagerCallback;
import android.nearlink.INearlinkSsapClient;
import android.nearlink.INearlinkSsapServer;
import android.nearlink.INearlinkStateChangeCallback;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkProtoEnums;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.UserHandle;
import android.os.UserManager;
import android.provider.Settings;
import android.util.Slog;

import com.android.internal.R;
import com.android.internal.util.DumpUtils;

/**
 * NearlinkManagerService
 *
 * @since 2023-12-01
 */
class NearlinkManagerService extends INearlinkManager.Stub {
    private static final String TAG = "NearlinkManagerService";
    private static final boolean DBG = true;

    private static final String NEARLINK_ADMIN_PERM = android.Manifest.permission.NEARLINK_ADMIN;
    private static final String NEARLINK_PERM = android.Manifest.permission.NEARLINK;

    private static final String SECURE_SETTINGS_NEARLINK_ADDR_VALID = "nearlink_addr_valid";
    private static final String SECURE_SETTINGS_NEARLINK_ADDRESS = "nearlink_address";
    private static final String SECURE_SETTINGS_NEARLINK_NAME = "nearlink_name";

    private static final int ACTIVE_LOG_MAX_SIZE = 20;
    private static final int CRASH_LOG_MAX_SIZE = 100;

    /**
     * Maximum msec to wait for a bind
     */
    private static final int TIMEOUT_BIND_MS = 3000;

    /**
     * Maximum msec to wait for service restart
     */
    private static final int SERVICE_RESTART_TIME_MS = 200;

    /**
     * Maximum msec to wait for restart due to error
     */
    private static final int ERROR_RESTART_TIME_MS = 3000;

    /**
     * Maximum msec to delay MESSAGE_USER_SWITCHED
     */
    private static final int USER_SWITCHED_TIME_MS = 200;

    /**
     * Delay for the addProxy function in msec
     */
    private static final int ADD_PROXY_DELAY_MS = 100;

    private static final int MESSAGE_ENABLE = 1;
    private static final int MESSAGE_DISABLE = 2;
    private static final int MESSAGE_REGISTER_ADAPTER = 20;
    private static final int MESSAGE_UNREGISTER_ADAPTER = 21;
    private static final int MESSAGE_REGISTER_STATE_CHANGE_CALLBACK = 30;
    private static final int MESSAGE_UNREGISTER_STATE_CHANGE_CALLBACK = 31;
    private static final int MESSAGE_NEARLINK_SERVICE_CONNECTED = 40;
    private static final int MESSAGE_NEARLINK_SERVICE_DISCONNECTED = 41;
    private static final int MESSAGE_RESTART_NEARLINK_SERVICE = 42;
    private static final int MESSAGE_NEARLINK_STATE_CHANGE = 60;
    private static final int MESSAGE_TIMEOUT_BIND = 100;
    private static final int MESSAGE_TIMEOUT_UNBIND = 101;
    private static final int MESSAGE_GET_NAME_AND_ADDRESS = 200;
    private static final int MESSAGE_USER_SWITCHED = 300;
    private static final int MESSAGE_USER_UNLOCKED = 301;
    private static final int MESSAGE_ADD_PROXY_DELAYED = 400;
    private static final int MESSAGE_BIND_PROFILE_SERVICE = 401;
    private static final int MESSAGE_RESTORE_USER_SETTING = 500;

    private static final int RESTORE_SETTING_TO_ON = 1;
    private static final int RESTORE_SETTING_TO_OFF = 0;

    private static final int MAX_ERROR_RESTART_RETRIES = 6;

    /**
     * Nearlink persisted setting is off
     * 状态值：星闪关闭
     */
    private static final int NEARLINK_OFF = 0;

    /**
     * Nearlink persisted setting is on
     * and Airplane mode won't affect Nearlink state at start up
     * 状态值：星闪打开，且不受飞行模式打开的影响
     */
    private static final int NEARLINK_ON_NEARLINK = 1;

    /**
     * Nearlink persisted setting is on
     * but Airplane mode will affect Nearlink state at start up
     * and Airplane mode will have higher priority.
     * 状态值：星闪打开，且飞行模式的打开会影响星闪的状态，此状态下飞行模式的优先级更高
     */
    private static final int NEARLINK_ON_AIRPLANE = 2;

    private static final int SERVICE_INEARLINK = 1;
    private static final int SERVICE_INEARLINK_DISCOVERY = 2;
    private static final int SERVICE_INEARLINK_CONNECTION = 3;
    private static final int SERVICE_INEARLINK_SSAP_CLIENT = 4;
    private static final int SERVICE_INEARLINK_SSAP_SERVER = 5;

    private final Context mContext;

    // Locks are not provided for mName and mAddress.
    // They are accessed in handler or broadcast receiver, same thread context.
    // 用户名和地址是使用Handler和广播接收器来获取的，是线程安全的，不需要使用锁等同步机制。
    private String mAddress;
    private String mName;
    private final ContentResolver mContentResolver;
    private final RemoteCallbackList<INearlinkManagerCallback> mCallbacks;
    private final RemoteCallbackList<INearlinkStateChangeCallback> mStateChangeCallbacks;

    private boolean mBinding;
    private boolean mUnbinding;
    private IBinder mNearlinkBinder;
    private INearlink mNearlink;
    private INearlinkDiscovery mNearlinkDiscovery;
    private INearlinkSsapClient mNearlinkSsapClient;
    private INearlinkSsapServer mNearlinkSsapServer;
    private INearlinkConnection mNearlinkConnection;

    private final ReentrantReadWriteLock mNearlinkLock = new ReentrantReadWriteLock();

    // used inside handler thread
    private boolean mQuietEnable = false;
    private boolean mEnable;

    // configuration from external IBinder call which is used to
    // synchronize with broadcast receiver.
    private boolean mQuietEnableExternal;
    private boolean mEnableExternal;

    /**
     * Map of apps registered to keep Nearlink scanning on.
     */
    private Map<IBinder, ClientDeathRecipient> mNearlinkApps = new ConcurrentHashMap<>();

    private int mState;
    private final NearlinkHandler mHandler;
    private int mErrorRecoveryRetryCounter;
    private final int mSystemUiUid;

    private final boolean mWirelessConsentRequired;

    private final LinkedList<ActiveLog> mActiveLogs = new LinkedList<>();
    private final LinkedList<Long> mCrashTimestamps = new LinkedList<>();
    private int mCrashes;
    private long mLastEnabledTime;

    private final INearlinkCallback mNearlinkCallback = new INearlinkCallback.Stub() {
        @Override
        public void onNearlinkStateChange(int prevState, int newState) throws RemoteException {
            Slog.d(TAG, "onNearlinkStateChange()");
            Message msg =
                    mHandler.obtainMessage(MESSAGE_NEARLINK_STATE_CHANGE, prevState, newState);
            mHandler.sendMessage(msg);
        }
    };

    private final ContentObserver mAirplaneModeObserver = new ContentObserver(null) {
        @Override
        public void onChange(boolean unused) {
            if (DBG) {
                Slog.d(TAG, "mAirplaneModeObserver onChange");
            }
        }
    };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (DBG) {
                Slog.d(TAG, "onReceive action: " + action);
            }
            if (NearlinkAdapter.ACTION_LOCAL_NAME_CHANGED.equals(action)) {
                String newName = intent.getStringExtra(NearlinkAdapter.EXTRA_LOCAL_NAME);
                if (DBG) {
                    Slog.d(TAG, "Nearlink Adapter name changed to " + newName);
                }
                if (newName != null) {
                    storeNameAndAddress(newName, null);
                }
            } else if (NearlinkAdapter.ACTION_NEARLINK_ADDRESS_CHANGED.equals(action)) {
                String newAddress = intent.getStringExtra(NearlinkAdapter.EXTRA_NEARLINK_ADDRESS);
                if (newAddress != null) {
                    if (DBG) {
                        Slog.d(TAG, "Nearlink Adapter address changed to " + newAddress);
                    }
                    storeNameAndAddress(null, newAddress);
                } else {
                    if (DBG) {
                        Slog.e(TAG, "No Nearlink Adapter address parameter found");
                    }
                }
            } else if (Intent.ACTION_SETTING_RESTORED.equals(action)) {
                final String name = intent.getStringExtra(Intent.EXTRA_SETTING_NAME);
                if (Settings.Global.NEARLINK_ON.equals(name)) {
                    // The Nearlink On state may be changed during system restore.
                    final String prevValue =
                            intent.getStringExtra(Intent.EXTRA_SETTING_PREVIOUS_VALUE);
                    final String newValue = intent.getStringExtra(Intent.EXTRA_SETTING_NEW_VALUE);

                    if (DBG) {
                        Slog.d(TAG,
                                "ACTION_SETTING_RESTORED with NEARLINK_ON "
                                        + ", prevValue=" + prevValue
                                        + ", newValue=" + newValue);
                    }

                    if ((newValue != null) && (prevValue != null) && !prevValue.equals(newValue)) {
                        int arg1 = newValue.equals("0")
                                ? RESTORE_SETTING_TO_OFF : RESTORE_SETTING_TO_ON;
                        Message msg = mHandler.obtainMessage(MESSAGE_RESTORE_USER_SETTING,
                                arg1, 0);
                        mHandler.sendMessage(msg);
                    }
                }
            }
        }
    };

    NearlinkManagerService(Context context) {
        mHandler = new NearlinkHandler(IoThread.get().getLooper());

        mContext = context;

        mWirelessConsentRequired = context.getResources()
                .getBoolean(R.bool.config_wirelessConsentRequired);
        if (DBG) {
            Slog.d(TAG, "mWirelessConsentRequired = " + mWirelessConsentRequired);
        }

        mCrashes = 0;

        mNearlink = null;
        mNearlinkDiscovery = null;
        mNearlinkConnection = null;
        mNearlinkSsapClient = null;
        mNearlinkSsapServer = null;
        mNearlinkBinder = null;

        mBinding = false;
        mUnbinding = false;
        mEnable = false;
        mState = NearlinkAdapter.STATE_OFF;
        // false表示此次enable需要触发auto connect device和保存状态,
        // NearlinkAdapter::enableNoAutoConnect()可以改变此状态
        mQuietEnableExternal = false;
        mEnableExternal = false;
        mAddress = null;
        mName = null;
        mErrorRecoveryRetryCounter = 0;
        mContentResolver = context.getContentResolver();
        mCallbacks = new RemoteCallbackList<>();
        mStateChangeCallbacks = new RemoteCallbackList<>();

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_LOCAL_NAME_CHANGED);
        filter.addAction(NearlinkAdapter.ACTION_NEARLINK_ADDRESS_CHANGED);
        filter.addAction(Intent.ACTION_SETTING_RESTORED);
        filter.setPriority(IntentFilter.SYSTEM_HIGH_PRIORITY);
        mContext.registerReceiver(mReceiver, filter);

        // 从Setting数据库中加载本机的local name和address
        loadStoredNameAndAddress();
        // 查看上一次关机时,Nearlink是否为enable状态;如果是,这次开机也需要enable
        if (isNearlinkPersistedStateOn()) {
            if (DBG) {
                Slog.d(TAG, "Startup: Nearlink persisted state is ON.");
            }
            mEnableExternal = true;
        }

        String airplaneModeRadios =
                Settings.Global.getString(mContentResolver, Settings.Global.AIRPLANE_MODE_RADIOS);
        if (airplaneModeRadios == null || airplaneModeRadios.contains(
                Settings.Global.RADIO_NEARLINK)) {
            mContentResolver.registerContentObserver(
                    Settings.Global.getUriFor(Settings.Global.AIRPLANE_MODE_ON), true,
                    mAirplaneModeObserver);
        }

        int systemUiUid = -1;
        try {
            // Check if device is configured with no home screen, which implies no SystemUI.
            boolean noHome = mContext.getResources().getBoolean(R.bool.config_noHomeScreen);
            if (!noHome) {
                systemUiUid = mContext.getPackageManager()
                        .getPackageUidAsUser("com.android.systemui", PackageManager.MATCH_SYSTEM_ONLY,
                                UserHandle.USER_SYSTEM);
            }
            Slog.d(TAG, "Detected SystemUiUid: " + systemUiUid);
        } catch (PackageManager.NameNotFoundException e) {
            // Some platforms, such as wearables do not have a system ui.
            Slog.w(TAG, "Unable to resolve SystemUI's UID.", e);
        }
        mSystemUiUid = systemUiUid;
    }

    /**
     * Returns true if airplane mode is currently on
     */
    private boolean isAirplaneModeOn() {
        return Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) == 1;
    }

    private boolean supportNearlinkPersistedState() {
        return mContext.getResources().getBoolean(R.bool.config_supportNearlinkPersistedState);
    }

    /**
     * Returns true if the Nearlink saved state is "on"
     */
    private boolean isNearlinkPersistedStateOn() {
        if (DBG) {
            Slog.d(TAG, "isNearlinkPersistedStateOn()");
        }
        if (!supportNearlinkPersistedState()) {
            return false;
        }
        int state = Settings.Global.getInt(mContentResolver, Settings.Global.NEARLINK_ON, -1);
        if (DBG) {
            Slog.d(TAG, "Nearlink persisted state: " + stateForSetting(state));
        }
        return state != NEARLINK_OFF;
    }

    /**
     * Returns true if the Nearlink saved state is {@link NEARLINK_ON_NEARLINK}
     */
    private boolean isNearlinkPersistedStateOnNearlink() {
        if (DBG) {
            Slog.d(TAG, "isNearlinkPersistedStateOnNearlink()");
        }
        if (!supportNearlinkPersistedState()) {
            return false;
        }
        int state = Settings.Global.getInt(mContentResolver, Settings.Global.NEARLINK_ON,
                NEARLINK_ON_NEARLINK);
        if (DBG) {
            Slog.d(TAG, "Nearlink persisted state: " + stateForSetting(state));
        }
        return state == NEARLINK_ON_NEARLINK;
    }

    /**
     * Save the Nearlink on/off state
     */
    private void persistNearlinkSetting(int value) {
        if (DBG) {
            Slog.d(TAG, "Persisting Nearlink Setting: " + stateForSetting(value));
        }
        // waive WRITE_SECURE_SETTINGS permission check
        long callingIdentity = Binder.clearCallingIdentity();
        Settings.Global.putInt(mContext.getContentResolver(), Settings.Global.NEARLINK_ON, value);
        Binder.restoreCallingIdentity(callingIdentity);
    }

    /**
     * Returns true if the Nearlink Adapter's name and address is
     * locally cached
     *
     * @return
     */
    private boolean isNameAndAddressSet() {
        return mName != null && mAddress != null && mName.length() > 0 && mAddress.length() > 0;
    }

    /**
     * Retrieve the Nearlink Adapter's name and address and save it in in the local cache
     */
    private void loadStoredNameAndAddress() {
        if (DBG) {
            Slog.d(TAG, "Loading stored name and address");
        }
        boolean config = mContext.getResources()
                .getBoolean(R.bool.config_nearlink_address_validation);
        int settingValue = Settings.Secure.getInt(mContentResolver,
                SECURE_SETTINGS_NEARLINK_ADDR_VALID, 0);
        if (config && settingValue == 0) {
            // if the valid flag is not set, don't load the address and name
            if (DBG) {
                Slog.d(TAG, "invalid nearlink name and address stored");
            }
            return;
        }
        mName = Settings.Secure.getString(mContentResolver, SECURE_SETTINGS_NEARLINK_NAME);
        mAddress = Settings.Secure.getString(mContentResolver, SECURE_SETTINGS_NEARLINK_ADDRESS);
        if (DBG) {
            Slog.d(TAG, "Stored nearlink Name=" + mName + ",Address=" + mAddress);
        }
    }

    /**
     * Save the Nearlink name and address in the persistent store.
     * Only non-null values will be saved.
     *
     * @param name
     * @param address
     */
    private void storeNameAndAddress(String name, String address) {
        if (name != null) {
            Settings.Secure.putString(mContentResolver, SECURE_SETTINGS_NEARLINK_NAME, name);
            mName = name;
            if (DBG) {
                Slog.d(TAG, "Stored Nearlink name: " + Settings.Secure.getString(mContentResolver,
                        SECURE_SETTINGS_NEARLINK_NAME));
            }
        }

        if (address != null) {
            Settings.Secure.putString(mContentResolver, SECURE_SETTINGS_NEARLINK_ADDRESS, address);
            mAddress = address;
            if (DBG) {
                Slog.d(TAG,
                        "Stored Nearlink address: " + Settings.Secure.getString(mContentResolver,
                                SECURE_SETTINGS_NEARLINK_ADDRESS));
            }
        }

        if ((name != null) && (address != null)) {
            // name和address都不为空，设置 SECURE_SETTINGS_NEARLINK_ADDR_VALID 标志位为1
            Settings.Secure.putInt(mContentResolver, SECURE_SETTINGS_NEARLINK_ADDR_VALID, 1);
        }
    }

    @Override
    public INearlink registerAdapter(INearlinkManagerCallback callback) {
        if (callback == null) {
            Slog.w(TAG, "Callback is null in registerAdapter");
            return null;
        }
        Message msg = mHandler.obtainMessage(MESSAGE_REGISTER_ADAPTER);
        msg.obj = callback;
        mHandler.sendMessage(msg);
        return mNearlink;
    }

    @Override
    public void unregisterAdapter(INearlinkManagerCallback callback) {
        if (callback == null) {
            Slog.w(TAG, "Callback is null in unregisterAdapter");
            return;
        }

        mContext.enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        Message msg = mHandler.obtainMessage(MESSAGE_UNREGISTER_ADAPTER);
        msg.obj = callback;
        mHandler.sendMessage(msg);
    }

    @Override
    public void registerStateChangeCallback(INearlinkStateChangeCallback callback) {
        mContext.enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (callback == null) {
            Slog.w(TAG, "registerStateChangeCallback: Callback is null!");
            return;
        }
        Message msg = mHandler.obtainMessage(MESSAGE_REGISTER_STATE_CHANGE_CALLBACK);
        msg.obj = callback;
        mHandler.sendMessage(msg);
    }

    @Override
    public void unregisterStateChangeCallback(INearlinkStateChangeCallback callback) {
        mContext.enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (callback == null) {
            Slog.w(TAG, "unregisterStateChangeCallback: Callback is null!");
            return;
        }
        Message msg = mHandler.obtainMessage(MESSAGE_UNREGISTER_STATE_CHANGE_CALLBACK);
        msg.obj = callback;
        mHandler.sendMessage(msg);
    }

    @Override
    public boolean isEnabled() {
        if (DBG) {
            Slog.d(TAG, "isEnabled()");
        }
        if ((Binder.getCallingUid() != Process.SYSTEM_UID) && (!checkIfCallerIsForegroundUser())) {
            Slog.w(TAG, "isEnabled(): not allowed for non-active and non system user");
            return false;
        }

        try {
            mNearlinkLock.readLock().lock();
            if (mNearlink != null) {
                return mNearlink.isEnabled();
            }
        } catch (RemoteException e) {
            Slog.e(TAG, "isEnabled()", e);
        } finally {
            mNearlinkLock.readLock().unlock();
        }
        return false;
    }

    @Override
    public int getState() {
        if (DBG) {
            Slog.d(TAG, "getState()");
        }
        if ((Binder.getCallingUid() != Process.SYSTEM_UID) && (!checkIfCallerIsForegroundUser())) {
            Slog.w(TAG, "getState(): report OFF for non-active and non system user");
            return NearlinkAdapter.STATE_OFF;
        }

        try {
            mNearlinkLock.readLock().lock();
            if (mNearlink != null) {
                return mNearlink.getState();
            }
        } catch (RemoteException e) {
            Slog.e(TAG, "getState()", e);
        } finally {
            mNearlinkLock.readLock().unlock();
        }
        return NearlinkAdapter.STATE_OFF;
    }

    class ClientDeathRecipient implements IBinder.DeathRecipient {
        private String mPackageName;

        ClientDeathRecipient(String packageName) {
            mPackageName = packageName;
        }

        public void binderDied() {
            if (DBG) {
                Slog.d(TAG, "Binder is dead - unregister " + mPackageName);
            }

            for (Map.Entry<IBinder, ClientDeathRecipient> entry : mNearlinkApps.entrySet()) {
                IBinder token = entry.getKey();
                ClientDeathRecipient deathRec = entry.getValue();
                if (deathRec.equals(this)) {
                    updateNearlinkAppCount(token, false, mPackageName);
                    break;
                }
            }
        }

        public String getPackageName() {
            return mPackageName;
        }
    }

    /**
     * Disable Nearlink scan only mode.
     */
    private void disableNearlinkScanMode() {
        Slog.d(TAG, "disableNearlinkScanMode()");
        try {
            mNearlinkLock.writeLock().lock();
            if (mNearlink != null && (mNearlink.getState() != NearlinkAdapter.STATE_ON)) {
                if (DBG) {
                    Slog.d(TAG, "Reseting the mEnable flag for clean disable");
                }
                mEnable = false;
            }
        } catch (RemoteException e) {
            Slog.e(TAG, "getState()", e);
        } finally {
            mNearlinkLock.writeLock().unlock();
        }
    }

    @Override
    public int updateNearlinkAppCount(IBinder token, boolean enable, String packageName) {
        if (DBG) {
            Slog.d(TAG, "updateNearlinkAppCount() token = " + token +
                    " , enable = " + enable + " , packageName = " + packageName);
        }
        ClientDeathRecipient r = mNearlinkApps.get(token);
        if (r == null && enable) {
            ClientDeathRecipient deathRec = new ClientDeathRecipient(packageName);
            try {
                token.linkToDeath(deathRec, 0);
            } catch (RemoteException ex) {
                throw new IllegalArgumentException("Nearlink app (" + packageName + ") already dead!");
            }
            mNearlinkApps.put(token, deathRec);
            if (DBG) {
                Slog.d(TAG, "Registered for death of " + packageName);
            }
        } else if (!enable && r != null) {
            // Unregister death recipient as the app goes away.
            token.unlinkToDeath(r, 0);
            mNearlinkApps.remove(token);
            if (DBG) {
                Slog.d(TAG, "Unregistered for death of " + packageName);
            }
        }
        int appCount = mNearlinkApps.size();
        if (DBG) {
            Slog.d(TAG, appCount + " registered Nearlink Apps");
        }
        if (appCount == 0 && mEnable) {
            disableNearlinkScanMode();
        }
        if (appCount == 0 && !mEnableExternal) {
            sendNearlinkDownCallback();
        }
        return appCount;
    }

    /**
     * Clear all apps using Nearlink scan only mode.
     */
    private void clearNearlinkApps() {
        mNearlinkApps.clear();
    }

    /**
     * @hide
     */
    @Override
    public boolean isNearlinkAppPresent() {
        if (DBG) {
            Slog.d(TAG, "isNearlinkAppPresent() count: " + mNearlinkApps.size());
        }
        return mNearlinkApps.size() > 0;
    }

    /**
     * Call INearlink.onServiceUp() to continue if Nearlink should be on.
     */
    private void continueFromNearlinkOnState() {
        if (DBG) {
            Slog.d(TAG, "continueFromNearlinkOnState()");
        }
        try {
            mNearlinkLock.readLock().lock();
            if (mNearlink == null) {
                Slog.e(TAG, "onNearlinkServiceUp: mNearlink is null!");
                return;
            }
            if (isNearlinkPersistedStateOnNearlink() || !isNearlinkAppPresent()) {
                // This triggers transition to STATE_ON
                mNearlink.onServiceUp();
                persistNearlinkSetting(NEARLINK_ON_NEARLINK);
            }
        } catch (RemoteException e) {
            Slog.e(TAG, "Unable to call onServiceUp", e);
        } finally {
            mNearlinkLock.readLock().unlock();
        }
    }

    /**
     * Inform NearlinkAdapter instances that Nearlink part is down
     * and turn off all service and stack if no Nearlink app needs it
     */
    private void sendNearlinkDownCallback() {
        if (DBG) {
            Slog.d(TAG, "Calling sendNearlinkDownCallback callbacks");
        }

        if (mNearlink == null) {
            Slog.w(TAG, "Nearlink handle is null");
            return;
        }

        try {
            mNearlinkLock.readLock().lock();
            if (mNearlink != null) {
                mNearlink.onServiceDown();
            }
        } catch (RemoteException e) {
            Slog.e(TAG, "Call to onServiceDown() failed.", e);
        } finally {
            mNearlinkLock.readLock().unlock();
        }
    }

    @Override
    public boolean enable(String packageName) throws RemoteException {
        final int callingUid = Binder.getCallingUid();
        final boolean callerSystem = UserHandle.getAppId(callingUid) == Process.SYSTEM_UID;

        if (isNearlinkDisallowed()) {
            if (DBG) {
                Slog.d(TAG, "enable(): not enabling - nearlink disallowed");
            }
            return false;
        }

        if (!callerSystem) {
            if (!checkIfCallerIsForegroundUser()) {
                Slog.w(TAG, "enable(): not allowed for non-active and non system user");
                return false;
            }

            mContext.enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM,
                    "Need NEARLINK ADMIN permission");

            if (!isEnabled() && mWirelessConsentRequired && startConsentUiIfNeeded(packageName,
                    callingUid, NearlinkAdapter.ACTION_REQUEST_ENABLE)) {
                return false;
            }
        }

        if (DBG) {
            Slog.d(TAG, "enable(" + packageName + "):  mNearlink =" + mNearlink + " mBinding = "
                    + mBinding + " mState = " + NearlinkAdapter.nameForState(mState));
        }

        synchronized (mReceiver) {
            mQuietEnableExternal = false;
            mEnableExternal = true;
            // waive WRITE_SECURE_SETTINGS permission check
            sendEnableMsg(false,
                    NearlinkProtoEnums.ENABLE_DISABLE_REASON_APPLICATION_REQUEST, packageName);
        }
        if (DBG) {
            Slog.d(TAG, "enable returning");
        }
        return true;
    }

    @Override
    public boolean disable(String packageName, boolean persist) throws RemoteException {
        final int callingUid = Binder.getCallingUid();
        final boolean callerSystem = UserHandle.getAppId(callingUid) == Process.SYSTEM_UID;

        if (!callerSystem) {
            if (!checkIfCallerIsForegroundUser()) {
                Slog.w(TAG, "disable(): not allowed for non-active and non system user");
                return false;
            }

            mContext.enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM,
                    "Need NEARLINK ADMIN permission");

            if (isEnabled() && mWirelessConsentRequired && startConsentUiIfNeeded(packageName,
                    callingUid, NearlinkAdapter.ACTION_REQUEST_DISABLE)) {
                return false;
            }
        }

        if (DBG) {
            Slog.d(TAG, "disable(): mNearlink = " + mNearlink + " mBinding = " + mBinding);
        }

        synchronized (mReceiver) {
            if (persist) {
                persistNearlinkSetting(NEARLINK_OFF);
            }
            mEnableExternal = false;
            sendDisableMsg(NearlinkProtoEnums.ENABLE_DISABLE_REASON_APPLICATION_REQUEST,
                    packageName);
        }
        return true;
    }

    private boolean startConsentUiIfNeeded(String packageName, int callingUid, String intentAction)
            throws RemoteException {
        if (checkNearlinkPermissionWhenPermissionReviewRequired()) {
            return false;
        }
        try {
            // Validate the package only if we are going to use it
            ApplicationInfo applicationInfo = mContext.getPackageManager()
                    .getApplicationInfoAsUser(packageName,
                            PackageManager.MATCH_DEBUG_TRIAGED_MISSING,
                            UserHandle.getUserId(callingUid));
            if (applicationInfo.uid != callingUid) {
                throw new SecurityException("Package " + packageName
                        + " not in uid " + callingUid);
            }

            Intent intent = new Intent(intentAction);
            intent.putExtra(Intent.EXTRA_PACKAGE_NAME, packageName);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
            try {
                mContext.startActivity(intent);
            } catch (ActivityNotFoundException e) {
                // Shouldn't happen
                Slog.e(TAG, "Intent to handle action " + intentAction + " missing");
                return false;
            }
            return true;
        } catch (PackageManager.NameNotFoundException e) {
            throw new RemoteException(e.getMessage());
        }
    }

    /**
     * Check if the caller must still pass permission check or if the caller is exempted
     * from the consent UI via the MANAGE_NEARLINK_WHEN_PERMISSION_REVIEW_REQUIRED check.
     * <p>
     * Commands from some callers may be exempted from triggering the consent UI when
     * enabling nearlink. This exemption is checked via the
     * MANAGE_NEARLINK_WHEN_PERMISSION_REVIEW_REQUIRED and allows calls to skip
     * the consent UI where it may otherwise be required.
     *
     * @hide
     */
    private boolean checkNearlinkPermissionWhenPermissionReviewRequired() {
        if (!mWirelessConsentRequired) {
            return false;
        }
        return false;
    }

    public void unbindAndFinish() {
        if (DBG) {
            Slog.d(TAG, "unbindAndFinish(): " + mNearlink + " mBinding = " + mBinding
                    + " mUnbinding = " + mUnbinding);
        }

        try {
            mNearlinkLock.writeLock().lock();
            if (mUnbinding) {
                return;
            }
            mUnbinding = true;
            mHandler.removeMessages(MESSAGE_NEARLINK_STATE_CHANGE);
            mHandler.removeMessages(MESSAGE_BIND_PROFILE_SERVICE);
            if (mNearlink != null) {
                //Unregister callback object
                try {
                    mNearlink.unregisterCallback(mNearlinkCallback);
                } catch (RemoteException re) {
                    Slog.e(TAG, "Unable to unregister NearlinkCallback", re);
                }
                mNearlinkBinder = null;
                mNearlink = null;
                mContext.unbindService(mConnection);
                mUnbinding = false;
                mBinding = false;
            } else {
                mUnbinding = false;
            }
            mNearlinkDiscovery = null;
            mNearlinkSsapClient = null;
            mNearlinkSsapServer = null;
            mNearlinkConnection = null;
        } finally {
            mNearlinkLock.writeLock().unlock();
        }
    }

    @Override
    public INearlinkDiscovery getNearlinkDiscovery() throws RemoteException {
        Slog.d(TAG, "getNearlinkDiscovery() mNearlinkDiscovery = " + mNearlinkDiscovery);
        // sync protection
        return mNearlinkDiscovery;
    }

    @Override
    public INearlinkSsapClient getNearlinkSsapClient() throws RemoteException {
        Slog.d(TAG, "getNearlinkSsapClient() mNearlinkSsapClient = " + mNearlinkSsapClient);
        return mNearlinkSsapClient;
    }

    @Override
    public INearlinkSsapServer getNearlinkSsapServer() throws RemoteException {
        throw new IllegalArgumentException("Unsupported now");
    }

    @Override
    public INearlinkConnection getNearlinkConnection() throws RemoteException {
        Slog.d(TAG, "getNearlinkConnection() mNearlinkConnection = " + mNearlinkConnection);
        return mNearlinkConnection;
    }

    /**
     * Send enable message and set adapter name and address. Called when the boot phase becomes
     * PHASE_SYSTEM_SERVICES_READY.
     */
    public void handleOnBootPhase() {
        if (DBG) {
            Slog.d(TAG, "Nearlink boot completed");
        }

        final boolean isNearlinkDisallowed = isNearlinkDisallowed();
        if (isNearlinkDisallowed) {
            if (DBG) {
                Slog.d(TAG, "handleOnBootPhase(): nearlink disallowed , return");
            }
            return;
        }
        // 假设mEnableExternal是初始值，不考虑已经被修改的情况，则默认应该不会进入这个case
        if (mEnableExternal && isNearlinkPersistedStateOnNearlink()) {
            if (DBG) {
                Slog.d(TAG, "Auto-enabling Nearlink.");
            }
            sendEnableMsg(mQuietEnableExternal,
                    NearlinkProtoEnums.ENABLE_DISABLE_REASON_SYSTEM_BOOT,
                    mContext.getPackageName());
        }
    }

    /**
     * Called when user is unlocked.
     */
    public void handleOnUnlockUser(int userHandle) {
        if (DBG) {
            Slog.d(TAG, "User " + userHandle + " unlocked");
        }
        mHandler.obtainMessage(MESSAGE_USER_UNLOCKED, userHandle, 0).sendToTarget();
    }

    private void sendNearlinkStateCallback(boolean isUp) {
        try {
            int n = mStateChangeCallbacks.beginBroadcast();
            if (DBG) {
                Slog.d(TAG, "Broadcasting onNearlinkStateChange(" + isUp + ") to " + n
                        + " receivers.");
            }
            for (int i = 0; i < n; i++) {
                try {
                    mStateChangeCallbacks.getBroadcastItem(i).onNearlinkStateChange(isUp);
                } catch (RemoteException e) {
                    Slog.e(TAG, "Unable to call onNearlinkStateChange() on callback #" + i, e);
                }
            }
        } finally {
            mStateChangeCallbacks.finishBroadcast();
        }
    }

    /**
     * Inform NearlinkAdapter instances that Adapter service is up
     */
    private void sendNearlinkServiceUpCallback() {
        try {
            int n = mCallbacks.beginBroadcast();
            Slog.d(TAG, "Broadcasting onNearlinkServiceUp() to " + n + " receivers.");
            for (int i = 0; i < n; i++) {
                try {
                    mCallbacks.getBroadcastItem(i).onNearlinkServiceUp(mNearlink);
                } catch (RemoteException e) {
                    Slog.e(TAG, "Unable to call onNearlinkServiceUp() on callback #" + i, e);
                }
            }
        } finally {
            mCallbacks.finishBroadcast();
        }
    }

    /**
     * Inform NearlinkAdapter instances that Adapter service is down
     */
    private void sendNearlinkServiceDownCallback() {
        try {
            int n = mCallbacks.beginBroadcast();
            Slog.d(TAG, "Broadcasting onNearlinkServiceDown() to " + n + " receivers.");
            for (int i = 0; i < n; i++) {
                try {
                    mCallbacks.getBroadcastItem(i).onNearlinkServiceDown();
                } catch (RemoteException e) {
                    Slog.e(TAG, "Unable to call onNearlinkServiceDown() on callback #" + i, e);
                }
            }
        } finally {
            mCallbacks.finishBroadcast();
        }
    }

    @Override
    public String getAddress() throws RemoteException {
        mContext.enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");

        if ((Binder.getCallingUid() != Process.SYSTEM_UID) && (!checkIfCallerIsForegroundUser())) {
            Slog.w(TAG, "getAddress(): not allowed for non-active and non system user");
            return null;
        }

        if (mContext.checkCallingOrSelfPermission(Manifest.permission.LOCAL_MAC_ADDRESS)
                != PackageManager.PERMISSION_GRANTED) {
            return NearlinkAdapter.DEFAULT_MAC_ADDRESS;
        }

        try {
            mNearlinkLock.readLock().lock();
            if (mNearlink != null) {
                return mNearlink.getAddress();
            }
        } catch (RemoteException e) {
            Slog.e(TAG, "getAddress(): Unable to retrieve address remotely. Returning cached address", e);
        } finally {
            mNearlinkLock.readLock().unlock();
        }

        // mAddress is accessed from outside.
        // It is alright without a lock. Here, nearlink is off, no other thread is
        // changing mAddress
        return mAddress;
    }

    @Override
    public String getName() throws RemoteException {
        mContext.enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");

        if ((Binder.getCallingUid() != Process.SYSTEM_UID) && (!checkIfCallerIsForegroundUser())) {
            Slog.w(TAG, "getName(): not allowed for non-active and non system user");
            return null;
        }

        try {
            mNearlinkLock.readLock().lock();
            if (mNearlink != null) {
                return mNearlink.getName();
            }
        } catch (RemoteException e) {
            Slog.e(TAG, "getName(): Unable to retrieve name remotely. Returning cached name", e);
        } finally {
            mNearlinkLock.readLock().unlock();
        }

        // mName is accessed from outside.
        // It alright without a lock. Here, nearlink is off, no other thread is
        // changing mName
        return mName;
    }

    private class NearlinkServiceConnection implements ServiceConnection {
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            String name = componentName.getClassName();
            if (DBG) {
                Slog.d(TAG, "NearlinkServiceConnection: " + name);
            }
            Message msg = mHandler.obtainMessage(MESSAGE_NEARLINK_SERVICE_CONNECTED);
            switch (name) {
                case "com.android.nearlink.nlservice.AdapterService":
                    msg.arg1 = SERVICE_INEARLINK;
                    break;
                case "com.android.nearlink.discovery.DiscoveryService":
                    msg.arg1 = SERVICE_INEARLINK_DISCOVERY;
                    break;
                case "com.android.nearlink.connection.ConnectionService":
                    msg.arg1 = SERVICE_INEARLINK_CONNECTION;
                    break;
                case "com.android.nearlink.ssap.SsapClientService":
                    msg.arg1 = SERVICE_INEARLINK_SSAP_CLIENT;
                    break;
                case "com.android.nearlink.ssap.SsapServerService":
                    msg.arg1 = SERVICE_INEARLINK_SSAP_SERVER;
                    break;
                default:
                    Slog.e(TAG, "Unknown service connected: " + name);
                    return;
            }
            msg.obj = service;
            mHandler.sendMessage(msg);
        }

        public void onServiceDisconnected(ComponentName componentName) {
            // Called if we unexpectedly disconnect.
            String name = componentName.getClassName();
            if (DBG) {
                Slog.d(TAG, "NearlinkServiceConnection, disconnected: " + name);
            }
            Message msg = mHandler.obtainMessage(MESSAGE_NEARLINK_SERVICE_DISCONNECTED);
            switch (name) {
                case "com.android.nearlink.nlservice.AdapterService":
                    msg.arg1 = SERVICE_INEARLINK;
                    break;
                case "com.android.nearlink.discovery.DiscoveryService":
                    msg.arg1 = SERVICE_INEARLINK_DISCOVERY;
                    break;
                case "com.android.nearlink.connection.ConnectionService":
                    msg.arg1 = SERVICE_INEARLINK_CONNECTION;
                    break;
                case "com.android.nearlink.ssap.SsapClientService":
                    msg.arg1 = SERVICE_INEARLINK_SSAP_CLIENT;
                    break;
                case "com.android.nearlink.ssap.SsapServerService":
                    msg.arg1 = SERVICE_INEARLINK_SSAP_SERVER;
                    break;
                default:
                    Slog.e(TAG, "Unknown service disconnected: " + name);
                    return;
            }
            mHandler.sendMessage(msg);
        }
    }

    private NearlinkServiceConnection mConnection = new NearlinkServiceConnection();

    private class NearlinkHandler extends Handler {

        NearlinkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_GET_NAME_AND_ADDRESS:
                    if (DBG) {
                        Slog.d(TAG, "MESSAGE_GET_NAME_AND_ADDRESS");
                    }
                    try {
                        mNearlinkLock.writeLock().lock();
                        if (mNearlink != null) {
                            try {
                                storeNameAndAddress(mNearlink.getName(), mNearlink.getAddress());
                            } catch (RemoteException re) {
                                Slog.e(TAG, "Unable to grab names", re);
                            }
                        }
                    } finally {
                        mNearlinkLock.writeLock().unlock();
                    }
                    break;

                case MESSAGE_ENABLE:
                    if (DBG) {
                        Slog.d(TAG, "MESSAGE_ENABLE(" + msg.arg1 + "): mNearlink = " + mNearlink);
                    }
                    mHandler.removeMessages(MESSAGE_RESTART_NEARLINK_SERVICE);
                    mEnable = true;

                    // Use service interface to get the exact state
                    try {
                        mNearlinkLock.readLock().lock();
                        if (mNearlink != null) {
                            int state = mNearlink.getState();
                            if (state == NearlinkAdapter.STATE_ON) {
                                Slog.w(TAG, "Nearlink has Enable and is NEARLINK_ON State");
                                mNearlink.onServiceUp();
                                persistNearlinkSetting(NEARLINK_ON_NEARLINK);
                                break;
                            }
                        }
                    } catch (RemoteException e) {
                        Slog.e(TAG, "", e);
                    } finally {
                        mNearlinkLock.readLock().unlock();
                    }

                    mQuietEnable = (msg.arg1 == 1);
                    if (mNearlink == null) {
                        handleEnable(mQuietEnable);
                    } else {
                        //
                        // We need to wait until transitioned to STATE_OFF and
                        // the previous Nearlink process has exited. The
                        // waiting period has three components:
                        // (a) Wait until the local state is STATE_OFF. This
                        //     is accomplished by "waitForOnOff(false, true)".
                        // (b) Wait until the STATE_OFF state is updated to
                        //     all components.
                        // (c) Wait until the Nearlink process exits, and
                        //     ActivityManager detects it.
                        // The waiting for (b) and (c) is accomplished by
                        // delaying the MESSAGE_RESTART_NEARLINK_SERVICE
                        // message. On slower devices, that delay needs to be
                        // on the order of (2 * SERVICE_RESTART_TIME_MS).
                        //
                        waitForOnOff(false, true);
                        Message restartMsg =
                                mHandler.obtainMessage(MESSAGE_RESTART_NEARLINK_SERVICE);
                        mHandler.sendMessageDelayed(restartMsg, 2 * SERVICE_RESTART_TIME_MS);
                    }
                    break;

                case MESSAGE_DISABLE:
                    if (DBG) {
                        Slog.d(TAG, "MESSAGE_DISABLE: mNearlink = " + mNearlink);
                    }
                    mHandler.removeMessages(MESSAGE_RESTART_NEARLINK_SERVICE);
                    if (mEnable && mNearlink != null) {
                        waitForOnOff(true, false);
                        mEnable = false;
                        handleDisable();
                        waitForOnOff(false, false);
                    } else {
                        mEnable = false;
                        handleDisable();
                    }
                    break;

                case MESSAGE_RESTORE_USER_SETTING:
                    if ((msg.arg1 == RESTORE_SETTING_TO_OFF) && mEnable) {
                        if (DBG) {
                            Slog.d(TAG, "Restore Nearlink state to disabled");
                        }
                        persistNearlinkSetting(NEARLINK_OFF);
                        mEnableExternal = false;
                        sendDisableMsg(
                                NearlinkProtoEnums.ENABLE_DISABLE_REASON_RESTORE_USER_SETTING,
                                mContext.getPackageName());
                    } else if ((msg.arg1 == RESTORE_SETTING_TO_ON) && !mEnable) {
                        if (DBG) {
                            Slog.d(TAG, "Restore Nearlink state to enabled");
                        }
                        mQuietEnableExternal = false;
                        mEnableExternal = true;
                        // waive WRITE_SECURE_SETTINGS permission check
                        sendEnableMsg(false,
                                NearlinkProtoEnums.ENABLE_DISABLE_REASON_RESTORE_USER_SETTING,
                                mContext.getPackageName());
                    }
                    break;

                case MESSAGE_REGISTER_ADAPTER: {
                    INearlinkManagerCallback callback = (INearlinkManagerCallback) msg.obj;
                    mCallbacks.register(callback);
                    break;
                }
                case MESSAGE_UNREGISTER_ADAPTER: {
                    INearlinkManagerCallback callback = (INearlinkManagerCallback) msg.obj;
                    mCallbacks.unregister(callback);
                    break;
                }
                case MESSAGE_REGISTER_STATE_CHANGE_CALLBACK: {
                    INearlinkStateChangeCallback callback = (INearlinkStateChangeCallback) msg.obj;
                    mStateChangeCallbacks.register(callback);
                    break;
                }
                case MESSAGE_UNREGISTER_STATE_CHANGE_CALLBACK: {
                    INearlinkStateChangeCallback callback = (INearlinkStateChangeCallback) msg.obj;
                    mStateChangeCallbacks.unregister(callback);
                    break;
                }
                case MESSAGE_NEARLINK_SERVICE_CONNECTED: {
                    if (DBG) {
                        Slog.d(TAG, "MESSAGE_NEARLINK_SERVICE_CONNECTED: " + msg.arg1);
                    }
                    IBinder service = (IBinder) msg.obj;
                    try {
                        mNearlinkLock.writeLock().lock();
                        if (SERVICE_INEARLINK != msg.arg1) {    // 处理其他非SERVICE_INEARLINK服务
                            if (msg.arg1 == SERVICE_INEARLINK_DISCOVERY) {
                                mNearlinkDiscovery = INearlinkDiscovery.Stub.asInterface(
                                        Binder.allowBlocking(service));
                            } else if (msg.arg1 == SERVICE_INEARLINK_CONNECTION) {
                                mNearlinkConnection = INearlinkConnection.Stub.asInterface(
                                        Binder.allowBlocking(service));
                            } else if (msg.arg1 == SERVICE_INEARLINK_SSAP_CLIENT) {
                                mNearlinkSsapClient = INearlinkSsapClient.Stub.asInterface(
                                        Binder.allowBlocking(service));
                            } else if (msg.arg1 == SERVICE_INEARLINK_SSAP_SERVER) {
                                // todo: 暂不支持
                            }
                            if (mNearlinkDiscovery != null && mNearlinkConnection != null
                                && mNearlinkSsapClient != null) {
                                continueFromNearlinkOnState();
                            }
                            break;
                        }
                        // else must be SERVICE_INEARLINK

                        //Remove timeout
                        mHandler.removeMessages(MESSAGE_TIMEOUT_BIND);

                        mBinding = false;
                        mNearlinkBinder = service;
                        mNearlink = INearlink.Stub.asInterface(Binder.allowBlocking(service));

                        //Register callback object
                        try {
                            mNearlink.registerCallback(mNearlinkCallback);
                        } catch (RemoteException re) {
                            Slog.e(TAG, "Unable to register NearlinkCallback", re);
                        }
                        //Inform NearlinkAdapter instances that service is up
                        sendNearlinkServiceUpCallback();

                        //Do enable request
                        try {
                            boolean enable = mNearlink.enable();
                            Slog.d(TAG, "mNearlink.enable() = " + enable);
                            if (!enable) {
                                Slog.e(TAG, "INearlink.enable() returned false");
                            }
                        } catch (RemoteException e) {
                            Slog.e(TAG, "Unable to call enable()", e);
                        }
                    } finally {
                        mNearlinkLock.writeLock().unlock();
                    }

                    if (!mEnable) {
                        waitForOnOff(true, false);
                        handleDisable();
                        waitForOnOff(false, false);
                    }
                    break;
                }
                case MESSAGE_NEARLINK_STATE_CHANGE: {
                    handleStateChange(msg);
                    break;
                }
                case MESSAGE_NEARLINK_SERVICE_DISCONNECTED: {
                    Slog.e(TAG, "MESSAGE_NEARLINK_SERVICE_DISCONNECTED(" + msg.arg1 + ")");
                    try {
                        mNearlinkLock.writeLock().lock();
                        if (msg.arg1 == SERVICE_INEARLINK) {
                            // if service is unbinded already, do nothing and return
                            if (mNearlink == null) {
                                break;
                            }
                            mNearlink = null;
                        } else if (msg.arg1 == SERVICE_INEARLINK_DISCOVERY) {
                            mNearlinkDiscovery = null;
                            break;
                        } else if (msg.arg1 == SERVICE_INEARLINK_CONNECTION) {
                            mNearlinkConnection = null;
                            break;
                        } else if (msg.arg1 == SERVICE_INEARLINK_SSAP_CLIENT) {
                            mNearlinkSsapClient = null;
                            break;
                        } else if (msg.arg1 == SERVICE_INEARLINK_SSAP_SERVER) {
                            mNearlinkSsapServer = null;
                            break;
                        } else {
                            Slog.e(TAG, "Unknown argument for service disconnect!");
                            break;
                        }
                    } finally {
                        mNearlinkLock.writeLock().unlock();
                    }

                    // log the unexpected crash
                    addCrashLog();
                    addActiveLog(NearlinkProtoEnums.ENABLE_DISABLE_REASON_CRASH,
                            mContext.getPackageName(), false);
                    if (mEnable) {
                        mEnable = false;
                        // Send a Nearlink Restart message
                        Message restartMsg = mHandler.obtainMessage(MESSAGE_RESTART_NEARLINK_SERVICE);
                        mHandler.sendMessageDelayed(restartMsg, SERVICE_RESTART_TIME_MS);
                    }

                    sendNearlinkServiceDownCallback();

                    // Send Nearlink state broadcast to update the Nearlink icon correctly
                    if ((mState == NearlinkAdapter.STATE_TURNING_ON)
                            || (mState == NearlinkAdapter.STATE_ON)) {
                        nearlinkStateChangeHandler(NearlinkAdapter.STATE_ON,
                                NearlinkAdapter.STATE_TURNING_OFF);
                        mState = NearlinkAdapter.STATE_TURNING_OFF;
                    }
                    if (mState == NearlinkAdapter.STATE_TURNING_OFF) {
                        nearlinkStateChangeHandler(NearlinkAdapter.STATE_TURNING_OFF,
                                NearlinkAdapter.STATE_OFF);
                    }

                    mHandler.removeMessages(MESSAGE_NEARLINK_STATE_CHANGE);
                    mState = NearlinkAdapter.STATE_OFF;
                    break;
                }
                case MESSAGE_RESTART_NEARLINK_SERVICE: {
                    Slog.d(TAG, "MESSAGE_RESTART_NEARLINK_SERVICE");
                    /* Enable without persisting the setting as
                     it doesnt change when INearlink
                     service restarts */
                    mEnable = true;
                    addActiveLog(NearlinkProtoEnums.ENABLE_DISABLE_REASON_RESTARTED,
                            mContext.getPackageName(), true);
                    handleEnable(mQuietEnable);
                    break;
                }
                case MESSAGE_TIMEOUT_BIND: {
                    Slog.e(TAG, "MESSAGE_TIMEOUT_BIND");
                    mNearlinkLock.writeLock().lock();
                    mBinding = false;
                    mNearlinkLock.writeLock().unlock();
                    break;
                }
                case MESSAGE_TIMEOUT_UNBIND: {
                    Slog.e(TAG, "MESSAGE_TIMEOUT_UNBIND");
                    mNearlinkLock.writeLock().lock();
                    mUnbinding = false;
                    mNearlinkLock.writeLock().unlock();
                    break;
                }
                case MESSAGE_USER_UNLOCKED: {
                    if (DBG) {
                        Slog.d(TAG, "MESSAGE_USER_UNLOCKED");
                    }
                    mHandler.removeMessages(MESSAGE_USER_SWITCHED);

                    if (mEnable && !mBinding && (mNearlink == null)) {
                        // We should be connected, but we gave up for some
                        // reason; maybe the Nearlink service wasn't encryption
                        // aware, so try binding again.
                        if (DBG) {
                            Slog.d(TAG, "Enabled but not bound; retrying after unlock");
                        }
                        handleEnable(mQuietEnable);
                    }
                }
            }
        }
    }

    private void handleStateChange(Message msg) {
        int prevState = msg.arg1;
        int newState = msg.arg2;
        if (DBG) {
            Slog.d(TAG, "MESSAGE_NEARLINK_STATE_CHANGE: "
                    + NearlinkAdapter.nameForState(prevState) + " > "
                    + NearlinkAdapter.nameForState(newState));
        }
        mState = newState;
        nearlinkStateChangeHandler(prevState, newState);
        // handle error state transition case from TURNING_ON to OFF
        // unbind and rebind nearlink service and enable nearlink
        if ((prevState == NearlinkAdapter.STATE_TURNING_ON)
                && (newState == NearlinkAdapter.STATE_OFF)
                && (mNearlink != null) && mEnable) {
            recoverNearlinkServiceFromError(false);
        }
        // If we tried to enable Nearlink while Nearlink was in the process of shutting down,
        // wait for the Nearlink process to fully tear down and then force a restart here.
        // This is a bit of a hack.
        if ((prevState == NearlinkAdapter.STATE_TURNING_OFF)
                && (newState == NearlinkAdapter.STATE_OFF)) {
            if (mEnable) {
                Slog.d(TAG, "Entering STATE_OFF but mEnabled is true; restarting.");
                waitForOnOff(false, true);
                Message restartMsg =
                        mHandler.obtainMessage(MESSAGE_RESTART_NEARLINK_SERVICE);
                mHandler.sendMessageDelayed(restartMsg, 2 * SERVICE_RESTART_TIME_MS);
            }
        }
        if (newState == NearlinkAdapter.STATE_ON) {
            // nearlink is working, reset the counter
            if (mErrorRecoveryRetryCounter != 0) {
                Slog.w(TAG, "nearlink is recovered from error");
                mErrorRecoveryRetryCounter = 0;
            }

            // 如果设备名和地址任一为空，则从协议栈获取
            if (!isNameAndAddressSet()) {
                if (DBG) {
                    Slog.d(TAG, "Getting adapter name and address");
                }
                Message getMsg = mHandler.obtainMessage(MESSAGE_GET_NAME_AND_ADDRESS);
                mHandler.sendMessage(getMsg);
            }
        }
    }

    private void handleEnable(boolean quietMode) {
        Slog.d(TAG, "handleEnable quietMode(mQuietEnable) = " + quietMode);
        mQuietEnable = quietMode;

        try {
            mNearlinkLock.writeLock().lock();
            if ((mNearlink == null) && (!mBinding)) {
                Slog.d(TAG, "mNearlink == null && mBinding = false");
                //Start bind timeout and bind
                Message timeoutMsg = mHandler.obtainMessage(MESSAGE_TIMEOUT_BIND);
                mHandler.sendMessageDelayed(timeoutMsg, TIMEOUT_BIND_MS);
                // bind INearlink
                Intent i = new Intent(INearlink.class.getName());
                if (!doBind(i, mConnection, Context.BIND_AUTO_CREATE | Context.BIND_IMPORTANT,
                        UserHandle.CURRENT)) {
                    mHandler.removeMessages(MESSAGE_TIMEOUT_BIND);
                } else {
                    mBinding = true;
                }
            } else if (mNearlink != null) {
                Slog.d(TAG, "handleEnable mNearlink = " + mNearlink);
                //Enable Nearlink
                try {
                    boolean enable = mNearlink.enable();
                    Slog.d(TAG, "INearlink.enable() returned " + enable + " , mQuietEnable = " + mQuietEnable);
                    if (!enable) {
                        Slog.e(TAG, "INearlink.enable() returned false");
                    }
                } catch (RemoteException e) {
                    Slog.e(TAG, "Unable to call enable()", e);
                }
            }
        } finally {
            mNearlinkLock.writeLock().unlock();
        }
    }

    boolean doBind(Intent intent, ServiceConnection conn, int flags, UserHandle user) {
        ComponentName comp = intent.resolveSystemService(mContext.getPackageManager(), 0);
        intent.setComponent(comp);
        boolean bindServiceAsUser = mContext.bindServiceAsUser(intent, conn, flags, user);
        Slog.e(TAG, "comp = " + comp + " , bindResult = " + bindServiceAsUser);
        if (comp == null || !bindServiceAsUser) {
            Slog.e(TAG, "Fail to bind to: " + intent);
            return false;
        }
        return true;
    }

    private void handleDisable() {
        Slog.d(TAG, "handleDisable");
        try {
            mNearlinkLock.readLock().lock();
            if (mNearlink != null) {
                if (DBG) {
                    Slog.d(TAG, "Sending off request.");
                }
                boolean disable = mNearlink.disable();
                Slog.d(TAG, "mNearlink.disable() = " + disable);
                if (!disable) {
                    Slog.e(TAG, "INearlink.disable() returned false");
                }
            }
        } catch (RemoteException e) {
            Slog.e(TAG, "Unable to call disable()", e);
        } finally {
            mNearlinkLock.readLock().unlock();
        }
    }

    private boolean checkIfCallerIsForegroundUser() {
        int foregroundUser;
        int callingUser = UserHandle.getCallingUserId();
        int callingUid = Binder.getCallingUid();
        long callingIdentity = Binder.clearCallingIdentity();
        UserManager um = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
        UserInfo ui = um.getProfileParent(callingUser);
        int parentUser = (ui != null) ? ui.id : UserHandle.USER_NULL;
        int callingAppId = UserHandle.getAppId(callingUid);
        boolean valid = false;
        try {
            foregroundUser = ActivityManager.getCurrentUser();
            valid = (callingUser == foregroundUser) || parentUser == foregroundUser
                    || callingAppId == Process.NFC_UID || callingAppId == mSystemUiUid;
            if (DBG && !valid) {
                Slog.d(TAG, "checkIfCallerIsForegroundUser: valid=" + valid + " callingUser="
                        + callingUser + " parentUser=" + parentUser + " foregroundUser="
                        + foregroundUser);
            }
        } finally {
            Binder.restoreCallingIdentity(callingIdentity);
        }
        return valid;
    }

    private void nearlinkStateChangeHandler(int prevState, int newState) {
        if (prevState == newState) { // No change. Nothing to do.
            if (DBG) {
                Slog.d(TAG, "nearlinkStateChangeHandler() No change. Nothing to do.");
            }
            return;
        }
        // Notify all proxy objects first of adapter state change
        if (newState == NearlinkAdapter.STATE_OFF) {
            // If Nearlink is off, send service down event to proxy objects, and unbind
            if (DBG) {
                Slog.d(TAG, "Nearlink is complete send Service Down");
            }
            sendNearlinkServiceDownCallback();
            unbindAndFinish();
        } else if (newState == NearlinkAdapter.STATE_ON) {
            boolean isUp = (newState == NearlinkAdapter.STATE_ON);
            sendNearlinkStateCallback(isUp);

            // bind INearlinkDiscovery
            if (DBG) {
                Slog.d(TAG, "Binding INearlinkDiscovery service");
            }
            Intent i = new Intent(INearlinkDiscovery.class.getName());
            doBind(i, mConnection, Context.BIND_AUTO_CREATE | Context.BIND_IMPORTANT,
                    UserHandle.CURRENT);

            // bind INearlinkConnection
            if (DBG) {
                Slog.d(TAG, "Binding INearlinkConnection service");
            }
            i = new Intent(INearlinkConnection.class.getName());
            doBind(i, mConnection, Context.BIND_AUTO_CREATE | Context.BIND_IMPORTANT,
                    UserHandle.CURRENT);

            // bind INearlinkSsapClient
            if (DBG) {
                Slog.d(TAG, "Binding INearlinkSsapClient service");
            }
            i = new Intent(INearlinkSsapClient.class.getName());
            doBind(i, mConnection, Context.BIND_AUTO_CREATE | Context.BIND_IMPORTANT,
                    UserHandle.CURRENT);
        } else {
            if (DBG) {
                Slog.d(TAG, "nearlinkStateChangeHandler other case: " +
                        "prevState = " + NearlinkAdapter.nameForState(prevState) +
                        ", newState = " + NearlinkAdapter.nameForState(newState));
            }
        }

        Intent intent = new Intent(NearlinkAdapter.ACTION_STATE_CHANGED);
        intent.putExtra(NearlinkAdapter.EXTRA_PREVIOUS_STATE, prevState);
        intent.putExtra(NearlinkAdapter.EXTRA_STATE, newState);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL, NEARLINK_PERM);
    }

    /**
     * if on is true, wait for state become ON
     * if off is true, wait for state become OFF
     * if both on and off are false, wait for state not ON
     */
    private boolean waitForOnOff(boolean on, boolean off) {
        Slog.d(TAG, "waitForOnOff(on = " + on + " , off = " + off + ")");
        int i = 0;
        while (i < 10) {
            try {
                mNearlinkLock.readLock().lock();
                if (mNearlink == null) {
                    break;
                }
                if (on) {
                    if (mNearlink.getState() == NearlinkAdapter.STATE_ON) {
                        return true;
                    }
                } else if (off) {
                    if (mNearlink.getState() == NearlinkAdapter.STATE_OFF) {
                        return true;
                    }
                } else {
                    if (mNearlink.getState() != NearlinkAdapter.STATE_ON) {
                        return true;
                    }
                }
            } catch (RemoteException e) {
                Slog.e(TAG, "getState()", e);
                break;
            } finally {
                mNearlinkLock.readLock().unlock();
            }
            if (on || off) {
                SystemClock.sleep(300);
            } else {
                SystemClock.sleep(50);
            }
            i++;
        }
        Slog.e(TAG, "waitForOnOff time out");
        return false;
    }

    private void sendDisableMsg(int reason, String packageName) {
        Slog.d(TAG, "sendDisableMsg packageName = " + packageName);
        mHandler.sendMessage(mHandler.obtainMessage(MESSAGE_DISABLE));
        addActiveLog(reason, packageName, false);
    }

    private void sendEnableMsg(boolean quietMode, int reason, String packageName) {
        Slog.d(TAG, "sendEnableMsg quietMode = " + quietMode + " , packageName = " + packageName);
        mHandler.sendMessage(mHandler.obtainMessage(MESSAGE_ENABLE, quietMode ? 1 : 0, 0));
        addActiveLog(reason, packageName, true);
        mLastEnabledTime = SystemClock.elapsedRealtime();
    }

    private void addActiveLog(int reason, String packageName, boolean enable) {
        synchronized (mActiveLogs) {
            if (mActiveLogs.size() > ACTIVE_LOG_MAX_SIZE) {
                mActiveLogs.remove();
            }
            mActiveLogs.add(
                    new ActiveLog(reason, packageName, enable, System.currentTimeMillis()));
        }
    }

    private void addCrashLog() {
        synchronized (mCrashTimestamps) {
            if (mCrashTimestamps.size() == CRASH_LOG_MAX_SIZE) {
                mCrashTimestamps.removeFirst();
            }
            mCrashTimestamps.add(System.currentTimeMillis());
            mCrashes++;
        }
    }

    private void recoverNearlinkServiceFromError(boolean clearNearlink) {
        Slog.e(TAG, "recoverNearlinkServiceFromError");
        try {
            mNearlinkLock.readLock().lock();
            if (mNearlink != null) {
                //Unregister callback object
                mNearlink.unregisterCallback(mNearlinkCallback);
            }
        } catch (RemoteException re) {
            Slog.e(TAG, "Unable to unregister", re);
        } finally {
            mNearlinkLock.readLock().unlock();
        }

        SystemClock.sleep(500);

        // disable
        addActiveLog(NearlinkProtoEnums.ENABLE_DISABLE_REASON_START_ERROR,
                mContext.getPackageName(), false);
        handleDisable();

        waitForOnOff(false, true);

        sendNearlinkServiceDownCallback();

        try {
            mNearlinkLock.writeLock().lock();
            if (mNearlink != null) {
                mNearlink = null;
                // Unbind
                mContext.unbindService(mConnection);
            }
            mNearlinkDiscovery = null;
            mNearlinkSsapClient = null;
            mNearlinkSsapServer = null;
            mNearlinkConnection = null;
        } finally {
            mNearlinkLock.writeLock().unlock();
        }

        mHandler.removeMessages(MESSAGE_NEARLINK_STATE_CHANGE);
        mState = NearlinkAdapter.STATE_OFF;

        if (clearNearlink) {
            clearNearlinkApps();
        }

        mEnable = false;

        if (mErrorRecoveryRetryCounter++ < MAX_ERROR_RESTART_RETRIES) {
            // Send a Nearlink Restart message to reenable nearlink
            Message restartMsg = mHandler.obtainMessage(MESSAGE_RESTART_NEARLINK_SERVICE);
            mHandler.sendMessageDelayed(restartMsg, ERROR_RESTART_TIME_MS);
        }
    }

    private boolean isNearlinkDisallowed() {
        long callingIdentity = Binder.clearCallingIdentity();
        try {
            return mContext.getSystemService(UserManager.class)
                    .hasUserRestriction(UserManager.DISALLOW_NEARLINK, UserHandle.SYSTEM);
        } finally {
            Binder.restoreCallingIdentity(callingIdentity);
        }
    }

    @Override
    public void dump(FileDescriptor fd, PrintWriter writer, String[] args) {
        if (!DumpUtils.checkDumpPermission(mContext, TAG, writer)) {
            return;
        }
        String errorMsg = null;

        boolean protoOut = (args.length > 0) && args[0].startsWith("--proto");

        if (!protoOut) {
            writer.println("Nearlink Status");
            writer.println("  enabled: " + isEnabled());
            writer.println("  state: " + NearlinkAdapter.nameForState(mState));
            writer.println("  address: " + mAddress);
            writer.println("  name: " + mName);
            if (mEnable) {
                long onDuration = SystemClock.elapsedRealtime() - mLastEnabledTime;
                String onDurationString = String.format(Locale.US, "%02d:%02d:%02d.%03d",
                        (int) (onDuration / (1000 * 60 * 60)),
                        (int) ((onDuration / (1000 * 60)) % 60), (int) ((onDuration / 1000) % 60),
                        (int) (onDuration % 1000));
                writer.println("  time since enabled: " + onDurationString);
            }

            if (mActiveLogs.size() == 0) {
                writer.println("\nNearlink never enabled!");
            } else {
                writer.println("\nEnable log:");
                for (ActiveLog log : mActiveLogs) {
                    writer.println("  " + log);
                }
            }

            writer.println(
                    "\nNearlink crashed " + mCrashes + " time" + (mCrashes == 1 ? "" : "s"));
            if (mCrashes == CRASH_LOG_MAX_SIZE) {
                writer.println("(last " + CRASH_LOG_MAX_SIZE + ")");
            }
            for (Long time : mCrashTimestamps) {
                writer.println("  " + timeToLog(time));
            }

            writer.println("\n" + mNearlinkApps.size() + " BLE app" + (mNearlinkApps.size() == 1 ? "" : "s")
                    + "registered");
            for (ClientDeathRecipient app : mNearlinkApps.values()) {
                writer.println("  " + app.getPackageName());
            }

            writer.println("");
            writer.flush();
            if (args.length == 0) {
                // Add arg to produce output
                args = new String[1];
                args[0] = "--print";
            }
        }

        if (mNearlinkBinder == null) {
            errorMsg = "Nearlink Service not connected";
        } else {
            try {
                mNearlinkBinder.dump(fd, args);
            } catch (RemoteException re) {
                errorMsg = "RemoteException while dumping Nearlink Service";
            }
        }
        if (errorMsg != null) {
            // Silently return if we are extracting metrics in Protobuf format
            if (protoOut) {
                return;
            }
            writer.println(errorMsg);
        }
    }

    private static String getEnableDisableReasonString(int reason) {
        switch (reason) {
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_APPLICATION_REQUEST:
                return "APPLICATION_REQUEST";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_AIRPLANE_MODE:
                return "AIRPLANE_MODE";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_DISALLOWED:
                return "DISALLOWED";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_RESTARTED:
                return "RESTARTED";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_START_ERROR:
                return "START_ERROR";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_SYSTEM_BOOT:
                return "SYSTEM_BOOT";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_CRASH:
                return "CRASH";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_USER_SWITCH:
                return "USER_SWITCH";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_RESTORE_USER_SETTING:
                return "RESTORE_USER_SETTING";
            case NearlinkProtoEnums.ENABLE_DISABLE_REASON_UNSPECIFIED:
            default:
                return "UNKNOWN[" + reason + "]";
        }
    }

    private static CharSequence timeToLog(long timestamp) {
        return android.text.format.DateFormat.format("MM-dd HH:mm:ss", timestamp);
    }

    /**
     * Used for tracking apps that enabled / disabled Nearlink.
     */
    private class ActiveLog {
        private int mReason;
        private String mPackageName;
        private boolean mEnable;
        private long mTimestamp;

        ActiveLog(int reason, String packageName, boolean enable, long timestamp) {
            mReason = reason;
            mPackageName = packageName;
            mEnable = enable;
            mTimestamp = timestamp;
        }

        public String toString() {
            return timeToLog(mTimestamp) + (mEnable ? "  Enabled " : " Disabled ")
                    + " due to " + getEnableDisableReasonString(mReason) + " by " + mPackageName;
        }
    }

    private String stateForSetting(int state) {
        switch (state) {
            case NEARLINK_OFF:
                return "NEARLINK_OFF";
            case NEARLINK_ON_NEARLINK:
                return "NEARLINK_ON_NEARLINK";
            case NEARLINK_ON_AIRPLANE:
                return "NEARLINK_ON_AIRPLANE";
            default:
                return "?!?!? (" + state + ")";
        }
    }
}
