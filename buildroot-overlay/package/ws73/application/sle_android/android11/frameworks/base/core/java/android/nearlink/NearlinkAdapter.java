/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.Manifest;
import android.annotation.IntDef;
import android.annotation.RequiresPermission;
import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.annotation.SystemApi;
import android.app.ActivityThread;
import android.content.Context;
import android.os.Binder;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * Nearlink适配类器类，Nearlink主要功能由此发起
 *
 * @since 2023.12.25
 */
public class NearlinkAdapter {
    private static final String TAG = "NearlinkAdapter";
    private static final boolean DBG = true;
    private static final boolean VDBG = false;

    /**
     * 广播最大长度
     */
    public static final int MAX_PUBLIC_DATA_LEN = 256;

    /**
     * Default MAC address reported to a client that does not have the
     * android.permission.LOCAL_MAC_ADDRESS permission.
     *
     * @hide
     */
    public static final String DEFAULT_MAC_ADDRESS = "02:00:00:00:00:00";

    /**
     * 错误
     */
    public static final int ERROR = Integer.MIN_VALUE;

    /**
     * 状态变化
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_STATE_CHANGED = "android.nearlink.adapter.action.STATE_CHANGED";

    /**
     * 连接状态变化
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_CONNECTION_STATE_CHANGED = "android.nearlink.adapter.action" +
            ".CONNECTION_STATE_CHANGED";

    /**
     * 配对状态变化
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_PAIR_STATE_CHANGED = "android.nearlink.adapter.action.PAIR_STATE_CHANGED";

    /**
     * discovery 启动
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_DISCOVERY_STARTED = "android.nearlink.adapter.action.DISCOVERY_STARTED";

    /**
     * discovery结束
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_DISCOVERY_FINISHED = "android.nearlink.adapter.action.DISCOVERY_FINISHED";

    /**
     * 广播状态修改成功
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_ANNOUNCE_MODE_CHANGED = "android.nearlink.adapter.action.ANNOUNCE_MODE_CHANGED";

    /**
     * 广播状态修改广播的extra值
     */
    public static final String EXTRA_ANNOUNCE_MODE = "android.nearlink.adapter.extra.ANNOUNCE_MODE";


    /**
     * @hide
     */
    @IntDef(prefix = {"ANNOUNCE_"}, value = {
            ANNOUNCE_MODE_NONE,
            ANNOUNCE_MODE_CONNECTABLE,
            ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface AnnounceMode {
    }

    /**
     * 设备不能被扫描也不能被发现，例如没有启动SLE协议站
     */
    public static final int ANNOUNCE_MODE_NONE = 20;

    /**
     * 可链接
     */
    public static final int ANNOUNCE_MODE_CONNECTABLE = 21;

    /**
     * 设备即可以被扫描也可以被链接
     */
    public static final int ANNOUNCE_MODE_CONNECTABLE_DISCOVERABLE = 23;


    /**
     * 异常的DISCOVERY_END_MILLIS值
     */
    public static final int DISCOVERY_END_MILLIS_ERROR = -1;

    /**
     * adv data 事件类型
     */
    public static final int ANNOUNCE_DATA_EVENT_TYPE = 3;

    /**
     * seek resp data 事件类型
     */
    public static final int SEEK_RSP_DATA_EVENT_TYPE = 11;


    /**
     * 超过最大广播路数
     */
    public static final int EXCEED_MAX_ANNOUNCE_USING_NUM = -1;

    /**
     * 最大本地设备名称
     */
    public static final int MAX_LOCAL_DEVICE_NAME_LEN = 30;


    /**
     * Used as an int extra field in {@link #ACTION_STATE_CHANGED}
     * intents to request the current power state. Possible values are:
     * {@link #STATE_OFF},
     * {@link #STATE_TURNING_ON},
     * {@link #STATE_ON},
     * {@link #STATE_TURNING_OFF},
     */
    public static final String EXTRA_STATE = "android.nearlink.adapter.extra.STATE";

    /**
     * Used as an int extra field in {@link #ACTION_STATE_CHANGED}
     * intents to request the previous power state. Possible values are:
     * {@link #STATE_OFF},
     * {@link #STATE_TURNING_ON},
     * {@link #STATE_ON},
     * {@link #STATE_TURNING_OFF}
     */
    public static final String EXTRA_PREVIOUS_STATE = "android.nearlink.adapter.extra.PREVIOUS_STATE";

    /**
     * Extra used by {@link #ACTION_CONNECTION_STATE_CHANGED}
     * <p>
     * This extra represents the current connection state.
     */
    public static final String EXTRA_CONNECTION_STATE = "android.nearlink.adapter.extra.CONNECTION_STATE";

    /**
     * Extra used by {@link #ACTION_CONNECTION_STATE_CHANGED}
     * <p>
     * This extra represents the previous connection state.
     */
    public static final String EXTRA_PREVIOUS_CONNECTION_STATE =
            "android.nearlink.adapter.extra.PREVIOUS_CONNECTION_STATE";


    /**
     * Extra used by {@link #ACTION_CONNECTION_STATE_CHANGED}
     * <p>
     * This extra represents the connection pair state.
     */
    public static final String EXTRA_PAIR_STATE = "android.nearlink.adapter.extra.PAIR_STATE";

    /**
     * profile type
     */
    public static final String EXTRA_PROFILE_TYPE = "android.nearlink.adapter.extra.PROFILE_TYPE";


    /**
     * disc reason
     */
    public static final String EXTRA_DISC_REASON = "android.nearlink.adapter.extra.DISC_REASON";

    /**
     * @hide
     */
    public static final String NEARLINK_MANAGER_SERVICE = "nearlink_manager";

    /**
     * 服务关闭，协议栈关闭
     */
    public static final int STATE_OFF = 3;

    /**
     * 协议栈开启中
     */
    public static final int STATE_TURNING_ON = 4;

    /**
     * 协议栈启动完毕
     */
    public static final int STATE_ON = 7;

    /**
     * 服务已经关闭，协议栈关闭中
     */
    public static final int STATE_TURNING_OFF = 8;

    /**
     * 状态到字符串翻译
     *
     * @param state Nearlink 状态机状态
     * @return 状态对应的字符名称
     */
    public static String nameForState(int state) {
        switch (state) {
            case STATE_OFF:
                return "OFF";
            case STATE_TURNING_ON:
                return "TURNING_ON";
            case STATE_ON:
                return "ON";
            case STATE_TURNING_OFF:
                return "TURNING_OFF";
            default:
                return "UNKNOWN STATE(" + state + ")";
        }
    }

    /**
     * Activity Action: Show a system activity that requests discoverable mode.
     * This activity will also request the user to turn on Nearlink if it
     * is not currently enabled.
     * <p>Discoverable mode is equivalent to {@link
     * #SCAN_MODE_CONNECTABLE_DISCOVERABLE}. It allows remote devices to see
     * this Nearlink adapter when they perform a discovery.
     * <p>For privacy, Android is not discoverable by default.
     * <p>The sender of this Intent can optionally use extra field {@link
     * #EXTRA_DISCOVERABLE_DURATION} to request the duration of
     * discoverability. Currently the default duration is 120 seconds, and
     * maximum duration is capped at 300 seconds for each request.
     * <p>Notification of the result of this activity is posted using the
     * {@link android.app.Activity#onActivityResult} callback. The
     * <code>resultCode</code>
     * will be the duration (in seconds) of discoverability or
     * {@link android.app.Activity#RESULT_CANCELED} if the user rejected
     * discoverability or an error has occurred.
     * <p>Applications can also listen for {@link #ACTION_SCAN_MODE_CHANGED}
     * for global notification whenever the scan mode changes. For example, an
     * application can be notified when the device has ended discoverability.
     * <p>Requires {@link android.Manifest.permission#NEARLINK}
     */
    @SdkConstant(SdkConstantType.ACTIVITY_INTENT_ACTION)
    public static final String ACTION_REQUEST_DISCOVERABLE = "android.nearlink.adapter.action.REQUEST_DISCOVERABLE";

    /**
     * Used as an optional int extra field in {@link
     * #ACTION_REQUEST_DISCOVERABLE} intents to request a specific duration
     * for discoverability in seconds. The current default is 120 seconds, and
     * requests over 300 seconds will be capped. These values could change.
     */
    public static final String EXTRA_DISCOVERABLE_DURATION = "android.nearlink.adapter.extra.DISCOVERABLE_DURATION";

    /**
     * Activity Action: Show a system activity that allows the user to turn on
     * Nearlink.
     * <p>This system activity will return once Nearlink has completed turning
     * on, or the user has decided not to turn Nearlink on.
     * <p>Notification of the result of this activity is posted using the
     * {@link android.app.Activity#onActivityResult} callback. The
     * <code>resultCode</code>
     * will be {@link android.app.Activity#RESULT_OK} if Nearlink has been
     * turned on or {@link android.app.Activity#RESULT_CANCELED} if the user
     * has rejected the request or an error has occurred.
     * <p>Applications can also listen for {@link #ACTION_STATE_CHANGED}
     * for global notification whenever Nearlink is turned on or off.
     * <p>Requires {@link Manifest.permission#Nearlink}
     */
    @SdkConstant(SdkConstantType.ACTIVITY_INTENT_ACTION)
    public static final String
            ACTION_REQUEST_ENABLE = "android.nearlink.adapter.action.REQUEST_ENABLE";

    /**
     * Activity Action: Show a system activity that allows the user to turn off
     * Nearlink. This is used only if permission review is enabled which is for
     * apps targeting API less than 23 require a permission review before any of
     * the app's components can run.
     * <p>This system activity will return once Nearlink has completed turning
     * off, or the user has decided not to turn Nearlink off.
     * <p>Notification of the result of this activity is posted using the
     * {@link android.app.Activity#onActivityResult} callback. The
     * <code>resultCode</code>
     * will be {@link android.app.Activity#RESULT_OK} if Nearlink has been
     * turned off or {@link android.app.Activity#RESULT_CANCELED} if the user
     * has rejected the request or an error has occurred.
     * <p>Applications can also listen for {@link #ACTION_STATE_CHANGED}
     * for global notification whenever Nearlink is turned on or off.
     * <p>Requires {@link Manifest.permission#Nearlink}
     *
     * @hide
     */
    @SdkConstant(SdkConstantType.ACTIVITY_INTENT_ACTION)
    public static final String
            ACTION_REQUEST_DISABLE = "android.nearlink.adapter.action.REQUEST_DISABLE";

    /**
     * Broadcast Action: The local Nearlink adapter has changed its friendly
     * Nearlink name.
     * <p>This name is visible to remote Nearlink devices.
     * <p>Always contains the extra field {@link #EXTRA_LOCAL_NAME} containing
     * the name.
     * <p>Requires {@link Manifest.permission#NEARLINK} to receive.
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String
            ACTION_LOCAL_NAME_CHANGED = "android.nearlink.adapter.action.LOCAL_NAME_CHANGED";

    /**
     * Used as a String extra field in {@link #ACTION_LOCAL_NAME_CHANGED}
     * intents to request the local Nearlink name.
     */
    public static final String EXTRA_LOCAL_NAME = "android.nearlink.adapter.extra.LOCAL_NAME";

    /**
     * Broadcast Action: The Nearlink adapter state has changed in LE only mode.
     *
     * @hide
     */
    @SystemApi
    public static final String ACTION_NEARLINK_STATE_CHANGED =
            "android.nearlink.adapter.action.NEARLINK_STATE_CHANGED";

    /**
     * Intent used to broadcast the change in the Nearlink address
     * of the local Nearlink adapter.
     * <p>Always contains the extra field {@link
     * #EXTRA_NEARLINK_ADDRESS} containing the Nearlink address.
     * <p>
     * Note: only system level processes are allowed to send this
     * defined broadcast.
     *
     * @hide
     */
    public static final String ACTION_NEARLINK_ADDRESS_CHANGED =
            "android.nearlink.adapter.action.NEARLINK_ADDRESS_CHANGED";

    /**
     * Used as a String extra field in {@link
     * #ACTION_NEARLINK_ADDRESS_CHANGED} intent to store the local
     * Nearlink address.
     *
     * @hide
     */
    public static final String EXTRA_NEARLINK_ADDRESS =
            "android.nearlink.adapter.extra.NEARLINK_ADDRESS";

    private final IBinder mToken;

    private static final int ADDRESS_LENGTH = 17;

    private static NearlinkAdapter sAdapter;
    private static NearlinkAnnouncer sNearlinkAnnouncer;
    private static NearlinkSeeker sNearlinkSeeker;
    private static NearlinkConnection sNearlinkConnection;

    private INearlinkManager mManagerService;
    private INearlink mService;
    private final ReentrantReadWriteLock mServiceLock = new ReentrantReadWriteLock();

    private final static Object mLock = new Object();

    /**
     * 获取NearlinkAdapter对象
     *
     * @return NearlinkAdapter对象
     */
    public static synchronized NearlinkAdapter getDefaultAdapter() {
        if (sAdapter == null) {
            IBinder b = ServiceManager.getService(NEARLINK_MANAGER_SERVICE);
            if (b != null) {
                INearlinkManager managerService = INearlinkManager.Stub.asInterface(b);
                sAdapter = new NearlinkAdapter(managerService);
            } else {
                Log.e(TAG, "Nearlink binder is null");
            }
        }
        return sAdapter;
    }

    /**
     * 构造器
     *
     * @param managerService INearlinkManager 对象
     */
    NearlinkAdapter(INearlinkManager managerService) {
        if (managerService == null) {
            throw new IllegalArgumentException("nearlink manager service is null");
        }
        try {
            mServiceLock.writeLock().lock();
            mService = managerService.registerAdapter(mManagerCallback);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.writeLock().unlock();
        }
        mManagerService = managerService;
        mToken = new Binder();
    }

    /**
     * 获取设备对象
     *
     * @param address 地址
     * @return 设备对象
     */
    public NearlinkDevice getRemoteDevice(String address) {
        return new NearlinkDevice(address);
    }

    /**
     * 获取设备对象
     *
     * @param address 地址对象
     * @return 设备对象
     */
    public NearlinkDevice getRemoteDevice(NearlinkAddress address) {
        return new NearlinkDevice(address);
    }

    /**
     * 获取远端设备
     *
     * @param address 地址
     * @return 设备对象
     */
    public NearlinkDevice getRemoteDevice(byte[] address) {
        if (address == null || address.length != 6) {
            throw new IllegalArgumentException("Nearlink address must have 6 bytes");
        }
        return new NearlinkDevice(
                String.format(Locale.US, "%02X:%02X:%02X:%02X:%02X:%02X", address[0], address[1],
                        address[2], address[3], address[4], address[5]));
    }

    /**
     * 获取广播管理器
     *
     * @return 广播管理器
     */
    @RequiresPermission(Manifest.permission.NEARLINK_ADMIN)
    public NearlinkAnnouncer getNearlinkAnnouncer() {
        synchronized (mLock) {
            if (sNearlinkAnnouncer == null) {
                sNearlinkAnnouncer = new NearlinkAnnouncer(mManagerService);
            }
        }
        return sNearlinkAnnouncer;
    }

    /**
     * 获取扫描管理器
     *
     * @return 描管理器
     */
    @RequiresPermission(Manifest.permission.NEARLINK_ADMIN)
    public NearlinkSeeker getNearlinkSeeker() {
        synchronized (mLock) {
            if (sNearlinkSeeker == null) {
                sNearlinkSeeker = new NearlinkSeeker(mManagerService);
            }
        }
        return sNearlinkSeeker;
    }

    /**
     * 获取nearlink链接
     *
     * @return Nearlink 链接
     */
    public NearlinkConnection getNearlinkConnection() {
        synchronized (mLock) {
            if (sNearlinkConnection == null) {
                sNearlinkConnection = new NearlinkConnection(mManagerService);
            }
        }
        return sNearlinkConnection;
    }

    /**
     * 检查Nearlink是否处于STATE_ON
     *
     * @return 是 ture 否 false
     */
    @RequiresPermission(Manifest.permission.NEARLINK)
    public boolean isEnabled() {
        try {
            mServiceLock.readLock().lock();
            if (mService != null) {
                return mService.isEnabled();
            }
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }

        return false;
    }

    /**
     * 获取Nearlink Adapter的状态
     *
     * @return STATE_OFF = 3, STATE_TURNING_ON = 4, STATE_ON = 7, STATE_TURNING_OFF = 8;
     */
    @RequiresPermission(Manifest.permission.NEARLINK)
    public int getState() {
        int state = NearlinkAdapter.STATE_OFF;

        try {
            mServiceLock.readLock().lock();
            if (mService != null) {
                state = mService.getState();
            }
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }

        if (VDBG) {
            Log.d(TAG, "" + hashCode() + ": getState(). Returning " + NearlinkAdapter.nameForState(state));
        }
        return state;
    }

    /**
     * 启动Nearlink协议栈
     *
     * @return true 启动指令已下发成功 false 启动指令下发失败
     */
    @RequiresPermission(Manifest.permission.NEARLINK_ADMIN)
    public boolean enable() {
        if (isEnabled()) {
            if (DBG) {
                Log.d(TAG, "enable(): Nearlink already enabled!");
            }
            return true;
        }
        try {
            return mManagerService.enable(ActivityThread.currentPackageName());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return false;
    }

    /**
     * 停止Nearlink协议栈
     *
     * @return true 关闭指令下发成功 false 关闭指令下发失败
     */
    @RequiresPermission(Manifest.permission.NEARLINK_ADMIN)
    public boolean disable() {
        try {
            return mManagerService.disable(ActivityThread.currentPackageName(), true);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return false;
    }

    /**
     * 获取当前设备的地址，地址格式例如： "00:11:22:AA:BB:CC"
     *
     * @return 地址
     */
    @RequiresPermission(Manifest.permission.NEARLINK)
    public String getAddress() {
        try {
            return mManagerService.getAddress();
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return null;
    }

    /**
     * 获取当前设备名称
     *
     * @return 名称
     */
    public String getName() {
        try {
            return mManagerService.getName();
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return null;
    }

    /**
     * 设置本地设备名称
     *
     * @param name 名称
     * @return true 成功 false 失败
     */
    @RequiresPermission(Manifest.permission.NEARLINK_ADMIN)
    public boolean setName(String name) {
        if (getState() != STATE_ON) {
            return false;
        }
        if (name == null || name.length() >= MAX_LOCAL_DEVICE_NAME_LEN) {
            Log.e(TAG, "name is null or exceed MAX_LOCAL_DEVICE_NAME_LEN");
            return false;
        }
        try {
            mServiceLock.readLock().lock();
            if (mService != null) {
                return mService.setName(name);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return false;
    }

    /**
     * 设置discovery的超时时间
     *
     * @param timeout startDiscovery的超时时间
     * @return true 成功 false 失败
     * @hide
     */
    public boolean setDiscoverableTimeout(int timeout) {
        if (getState() != STATE_ON) {
            return false;
        }
        try {
            mServiceLock.readLock().lock();
            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "setDiscoverableTimeout, but iNearlinkDiscovery is null");
                return false;
            }
            return iNearlinkDiscovery.setTimeoutForDiscovery(timeout);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return false;
    }

    /**
     * 获取当前discovery的剩余时间
     *
     * @return 剩余discovery的时间
     * @hide
     */
    public long getDiscoveryEndMillis() {
        try {
            mServiceLock.readLock().lock();
            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "getDiscoveryEndMillis, but iNearlinkDiscovery is null");
                return DISCOVERY_END_MILLIS_ERROR;
            }
            return iNearlinkDiscovery.getDiscoveryEndMillis();
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return -1;
    }

    /**
     * 获取startDiscovery的广播模式
     *
     * @return 广播模式
     */
    @RequiresPermission(Manifest.permission.NEARLINK)
    public int getAnnounceMode() {
        if (getState() != STATE_ON) {
            return ANNOUNCE_MODE_NONE;
        }
        try {
            mServiceLock.readLock().lock();

            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "iNearlinkDiscovery is null");
                return ANNOUNCE_MODE_NONE;
            }
            return iNearlinkDiscovery.getAnnounceMode();
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }

        return ANNOUNCE_MODE_NONE;
    }

    /**
     * 设置discovery时，可发现、链接的模式
     *
     * @param mode 广播模式
     * @return true 成功 false 失败
     * @hide
     */
    public boolean setAnnounceMode(@AnnounceMode int mode) {
        if (getState() != STATE_ON) {
            return false;
        }
        try {
            mServiceLock.readLock().lock();

            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "setScanMode, but iNearlinkDiscovery is null");
                return false;
            }

            return iNearlinkDiscovery.setAnnounceMode(mode, getDiscoverableTimeout());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return false;
    }

    /**
     * 设置discovery时，可发现、链接的模式
     *
     * @param mode     设置模式
     * @param duration discovery 的超时间
     * @return true 成功 false 失败
     */
    public boolean setAnnounceMode(@AnnounceMode int mode, int duration) {
        if (getState() != STATE_ON) {
            return false;
        }
        try {
            mServiceLock.readLock().lock();

            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "setAnnounceMode, but iNearlinkDiscovery is null");
                return false;
            }

            return iNearlinkDiscovery.setAnnounceMode(mode, duration);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return false;
    }

    /**
     * @hide
     */
    public int getDiscoverableTimeout() {
        if (getState() != STATE_ON) {
            return -1;
        }
        try {
            mServiceLock.readLock().lock();
            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "getDiscoverableTimeout, but iNearlinkDiscovery is null");
                return -1;
            }
            return iNearlinkDiscovery.getTimeoutForDiscovery();
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return -1;
    }

    /**
     * 获取discovery状态
     *
     * @return true 成功 false 失败
     */
    @RequiresPermission(Manifest.permission.NEARLINK_ADMIN)
    public boolean isDiscovering() {
        if (getState() != STATE_ON) {
            return false;
        }
        try {
            mServiceLock.readLock().lock();

            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "isDiscovering, but iNearlinkDiscovery is null");
                return false;
            }

            return iNearlinkDiscovery.isDiscovering();
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }

        return false;
    }

    /**
     * 获取announce公开数据最大长度，为announceData,respSeekData总和
     *
     * @return announce公开数据最大长度
     */
    public int getMaxPublicDataTotalLength() {
        return MAX_PUBLIC_DATA_LEN;
    }

    /**
     * 暂不支持
     */
    @RequiresPermission(Manifest.permission.NEARLINK)
    public int getMaxConnectedAudioDevices() {
        return 0;
    }

    /**
     * 暂不支持
     */
    public ParcelUuid[] getUuids() {
        if (getState() != STATE_ON) {
            return null;
        }
        try {
            mServiceLock.readLock().lock();
            if (mService != null) {
                return mService.getUuids();
            }
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return null;
    }

    /**
     * 同时启动Nearlink的扫描和广播，启动成功后，android系统广播ACTION_DISCOVERY_STARTED  扫描到的设备结果以android系统广播ACTION_FOUND
     *
     * @return true 启动指令下发成功 false 启动指令下发失败
     */
    @RequiresPermission(Manifest.permission.NEARLINK_ADMIN)
    public boolean startDiscovery() {
        if (getState() != STATE_ON) {
            return false;
        }
        try {
            mServiceLock.readLock().lock();

            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "startDiscovery, but iNearlinkDiscovery is null");
                return false;
            }

            NearlinkSeekParams nearlinkSeekParams = NearlinkSeekParams.builder().build();
            NearlinkAnnounceParam nearlinkAnnounceParam = NearlinkAnnounceParam.builder().build();
            NearlinkPublicData nearlinkAnnounceData = NearlinkPublicData.builder().build();
            NearlinkPublicData nearlinkSeekRespData = NearlinkPublicData.builder().build();

            return iNearlinkDiscovery.startDiscovery(nearlinkSeekParams, nearlinkAnnounceParam,
                    nearlinkAnnounceData, nearlinkSeekRespData);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return false;
    }

    /**
     * 停止discovery操作，停止后会广播 ACTION_DISCOVERY_FINISHED
     *
     * @return true 指令下发成功 false 指令下发失败
     */
    @RequiresPermission(Manifest.permission.NEARLINK_ADMIN)
    public boolean cancelDiscovery() {
        if (getState() != STATE_ON) {
            return false;
        }
        try {
            mServiceLock.readLock().lock();

            INearlinkDiscovery iNearlinkDiscovery = mManagerService.getNearlinkDiscovery();
            if (iNearlinkDiscovery == null) {
                Log.e(TAG, "cancelDiscovery, but iNearlinkDiscovery is null");
                return false;
            }
            return iNearlinkDiscovery.cancelDiscovery();
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return false;
    }

    /**
     * 获取支持的profiles值
     *
     * @return 支持的profiles值
     */
    public List<Integer> getSupportedProfiles() {
        final ArrayList<Integer> supportedProfiles = new ArrayList<Integer>();
        // TODO 需要协议栈支持，当前先写死只支持HID
        supportedProfiles.add(NearlinkProfile.HID_HOST);
        return supportedProfiles;
    }

    /**
     * 获取profile是否存在
     *
     * @param context  上下文
     * @param listener 监听器
     * @param profile  profile值
     * @return true 存在 false 不存在
     */
    public boolean getProfileProxy(Context context, NearlinkProfile.ServiceListener listener,
                                   int profile) {
        if (context == null || listener == null) {
            return false;
        }
        if (profile == NearlinkProfile.HID_HOST) {
            NearlinkHidHost iDev = new NearlinkHidHost(context, listener);
            return true;
        }
        return false;
    }

    /**
     * Close the connection of the profile proxy to the Service.
     *
     * <p> Clients should call this when they are no longer using
     * the proxy obtained from {@link #getProfileProxy}.
     * Profile can be one of  {@link NearlinkProfile#HEALTH}, {@link NearlinkProfile#HEADSET} or
     * {@link NearlinkProfile#A2DP}
     *
     * @param profile 配置profile值
     * @param proxy   Profile proxy object
     */
    public void closeProfileProxy(int profile, NearlinkProfile proxy) {
        if (proxy == null) {
            return;
        }

        switch (profile) {
            case NearlinkProfile.HID_HOST:
                NearlinkHidHost iDev = (NearlinkHidHost) proxy;
                iDev.close();
                break;
        }
    }

    private final INearlinkManagerCallback mManagerCallback =
            new INearlinkManagerCallback.Stub() {
                public void onNearlinkServiceUp(INearlink nearlinkService) {
                    if (DBG) {
                        Log.d(TAG, "onNearlinkServiceUp: " + nearlinkService);
                    }

                    mServiceLock.writeLock().lock();
                    mService = nearlinkService;
                    mServiceLock.writeLock().unlock();

                    synchronized (mProxyServiceStateCallbacks) {
                        for (INearlinkManagerCallback cb : mProxyServiceStateCallbacks) {
                            try {
                                if (cb != null) {
                                    cb.onNearlinkServiceUp(nearlinkService);
                                } else {
                                    Log.d(TAG, "onNearlinkServiceUp: cb is null!");
                                }
                            } catch (Exception e) {
                                Log.e(TAG, "", e);
                            }
                        }
                    }
                }

                public void onNearlinkServiceDown() {
                    if (DBG) {
                        Log.d(TAG, "onNearlinkServiceDown: " + mService);
                    }

                    try {
                        mServiceLock.writeLock().lock();
                        mService = null;
                    } finally {
                        mServiceLock.writeLock().unlock();
                    }

                    synchronized (mProxyServiceStateCallbacks) {
                        for (INearlinkManagerCallback cb : mProxyServiceStateCallbacks) {
                            try {
                                if (cb != null) {
                                    cb.onNearlinkServiceDown();
                                } else {
                                    Log.d(TAG, "onNearlinkServiceDown: cb is null!");
                                }
                            } catch (Exception e) {
                                Log.e(TAG, "", e);
                            }
                        }
                    }
                }
            };

    /**
     * 释放资源
     *
     * @throws Throwable 释放资源异常
     */
    protected void finalize() throws Throwable {
        try {
            mManagerService.unregisterAdapter(mManagerCallback);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            super.finalize();
        }
    }

    /**
     * 检查地址是否合法
     *
     * @param address 地址
     * @return true 合法 false 不合法
     */
    public static boolean checkNearlinkAddress(String address) {
        if (address == null || address.length() != ADDRESS_LENGTH) {
            return false;
        }
        for (int i = 0; i < ADDRESS_LENGTH; i++) {
            char c = address.charAt(i);
            switch (i % 3) {
                case 0:
                case 1:
                    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
                        // hex character, OK
                        break;
                    }
                    return false;
                case 2:
                    if (c == ':') {
                        break;  // OK
                    }
                    return false;
                default:
                    Log.e(TAG, "un handle val: " + i);
            }
        }
        return true;
    }

    INearlinkManager getNearlinkManager() {
        return mManagerService;
    }

    private final ArrayList<INearlinkManagerCallback> mProxyServiceStateCallbacks =
            new ArrayList<INearlinkManagerCallback>();

    INearlink getNearlinkService(INearlinkManagerCallback cb) {
        synchronized (mProxyServiceStateCallbacks) {
            if (cb == null) {
                Log.w(TAG, "getNearlinkService() called with no NearlinkManagerCallback");
            } else if (!mProxyServiceStateCallbacks.contains(cb)) {
                mProxyServiceStateCallbacks.add(cb);
            } else {
                Log.e(TAG, "unknown INearlinkManagerCallback");
            }
        }
        return mService;
    }

    void removeServiceStateCallback(INearlinkManagerCallback cb) {
        synchronized (mProxyServiceStateCallbacks) {
            mProxyServiceStateCallbacks.remove(cb);
        }
    }

    /**
     * 获取配对设备列表
     *
     * @return 配对设备列表
     */
    public Set<NearlinkDevice> getPairedDevices() {
        if (getState() != STATE_ON) {
            return toDeviceSet(new NearlinkDevice[0]);
        }
        try {
            mServiceLock.readLock().lock();
            if (mService != null && sNearlinkConnection != null) {
                return toDeviceSet(sNearlinkConnection.getPairedDevices().toArray(new NearlinkDevice[0]));
            }
            return toDeviceSet(new NearlinkDevice[0]);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return null;
    }

    /**
     * ACB-未连接
     */
    public static final int STATE_NONE = NearlinkConstant.SLE_ACB_STATE_NONE;

    /**
     * ACB-连接中
     */
    public static final int STATE_CONNECTING = NearlinkConstant.SLE_ACB_STATE_CONNECTING;

    /**
     * ACB-已连接
     */
    public static final int STATE_CONNECTED = NearlinkConstant.SLE_ACB_STATE_CONNECTED;

    /**
     * ACB-断连中
     */
    public static final int STATE_DISCONNECTING = NearlinkConstant.SLE_ACB_STATE_DISCONNECTING;

    /**
     * ACB-已断连
     */
    public static final int STATE_DISCONNECTED = NearlinkConstant.SLE_ACB_STATE_DISCONNECTED;

    /**
     * 获取本端设备连接状态
     *
     * @return 本端设备连接状态
     */
    public int getConnectionState() {
        if (getState() != STATE_ON) {
            return NearlinkAdapter.STATE_DISCONNECTED;
        }
        try {
            mServiceLock.readLock().lock();
            if (mService != null && sNearlinkConnection != null) {
                return sNearlinkConnection.getAdapterConnectionState();
            }
        } catch (RemoteException e) {
            Log.e(TAG, "getConnectionState:", e);
        } finally {
            mServiceLock.readLock().unlock();
        }
        return NearlinkAdapter.STATE_DISCONNECTED;
    }

    private Set<NearlinkDevice> toDeviceSet(NearlinkDevice[] devices) {
        Set<NearlinkDevice> deviceSet = new HashSet<NearlinkDevice>(Arrays.asList(devices));
        return Collections.unmodifiableSet(deviceSet);
    }
}
