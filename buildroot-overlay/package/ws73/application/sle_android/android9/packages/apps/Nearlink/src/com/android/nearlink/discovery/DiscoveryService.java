/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery;

import android.content.Intent;
import android.nearlink.INearlinkAnnounceCallback;
import android.nearlink.INearlinkDiscovery;
import android.nearlink.INearlinkSeekCallback;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAnnounceParam;
import android.nearlink.NearlinkPublicData;
import android.nearlink.NearlinkSeekFilter;
import android.nearlink.NearlinkSeekParams;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

import com.android.nearlink.nlservice.AdapterService;
import com.android.nearlink.nlservice.ProfileService;

import java.text.SimpleDateFormat;
import java.util.List;

/**
 * DD服务，seek、announce入口
 *
 * @since 2023-12-04
 */
public class DiscoveryService extends ProfileService {

    private static final boolean DBG = true;
    private static final String TAG = "DiscoveryService";


    private static final int DISCOVERY_STOP_DELAY_MSG_TYPE = 2;

    private static final int DEFAULT_DISCOVERY_TIMEOUT_MS = 60 * 1000;
    private static final int MAX_DISCOVERY_TIMEOUT_MS = 10 * 60 * 1000;

    private static final int M_OBJECT_WAIT_TIMEOUT_MS = 2 * 1000;

    private static final int DISCOVERING_SEEK_OFF_STATUS = 0;
    private static final int DISCOVERING_SEEK_TURNING_ON_STATUS = 1;
    private static final int DISCOVERING_SEEK_ON_STATUS = 2;
    private static final int DISCOVERING_SEEK_TURNING_OFF_STATUS = 3;
    private static final int DISCOVERING_SEEK_ON_ERROR_STATUS = 4;
    private static final int DISCOVERING_SEEK_OFF_ERROR_STATUS = 5;

    private volatile boolean mProfileStarted = false;
    private volatile int mDiscoveringSeekStatus;
    private volatile int mTimeoutForDiscovery = DEFAULT_DISCOVERY_TIMEOUT_MS;
    private volatile int mAnnounceModeForDiscovery;
    private volatile long mDiscoveryEndMs; //< Time (ms since epoch) that discovery ended or will end.
    private static DiscoveryService sDiscoveryService;

    NearlinkDiscoveryBinder mNearlinkDiscoveryBinder;
    AnnounceManager announceManager;
    SeekManager seekManager;
    private volatile DiscoveryHandler mDiscoveryHandler;
    private final Object mObject = new Object();

    public static final String NEARLINK_ADMIN_PERM = android.Manifest.permission.NEARLINK_ADMIN;
    public static final String NEARLINK_PRIVILEGED = android.Manifest.permission.NEARLINK_PRIVILEGED;
    public static final String NEARLINK_PERM = android.Manifest.permission.NEARLINK;

    @Override
    protected IProfileServiceBinder initBinder() {
        return new NearlinkDiscoveryBinder(this);
    }

    @Override
    protected boolean start() {
        HandlerThread thread = new HandlerThread("DiscoveryService");
        thread.start();
        this.mDiscoveryHandler = new DiscoveryHandler(thread.getLooper());
        mProfileStarted = true;
        synUpdateDiscoveringSeekStatus(DISCOVERING_SEEK_OFF_STATUS);
        mNearlinkDiscoveryBinder = new NearlinkDiscoveryBinder(this);
        announceManager = new AnnounceManager(AdapterService.getAdapterService(), this);
        seekManager = new SeekManager(AdapterService.getAdapterService(), this);
        setDiscoveryService(this);
        return true;
    }

    @Override
    protected boolean stop() {
        setDiscoveryService(null);
        return true;
    }

    @Override
    protected void cleanup() {
        if (this.mDiscoveryHandler != null) {
            mDiscoveryHandler.removeCallbacksAndMessages(null);
            Looper looper = mDiscoveryHandler.getLooper();
            if (looper != null) {
                looper.quitSafely();
            }
            mDiscoveryHandler = null;
        }

        if (this.mNearlinkDiscoveryBinder != null) {
            this.mNearlinkDiscoveryBinder.cleanup();
            this.mNearlinkDiscoveryBinder = null;
        }

        if (this.announceManager != null) {
            announceManager.cleanUp();
            announceManager = null;
        }

        if (this.seekManager != null) {
            this.seekManager.cleanUp();
            this.seekManager = null;
        }
    }

    public static synchronized DiscoveryService getDiscoveryService() {
        Log.d(TAG, "getDiscoveryService() - returning " + sDiscoveryService);
        return sDiscoveryService;
    }

    private static synchronized void setDiscoveryService(DiscoveryService instance) {
        sDiscoveryService = instance;
    }

    public boolean startDiscovery(NearlinkSeekParams nearlinkSeekParams, NearlinkAnnounceParam nearlinkAnnounceParam,
                                  NearlinkPublicData nearlinkAnnounceData, NearlinkPublicData nearlinkSeekRespData) {

        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        Log.e(TAG, "service startDiscovery");

        synchronized (mObject) {

            if (synIsDiscovering()) {
                Log.e(TAG, "startDiscovery, but is already start discovery!!!");
                mDiscoveryHandler.removeMessages(DISCOVERY_STOP_DELAY_MSG_TYPE);
                sendBroadcastDiscoveryStarted();
                return true;
            }

            if (mDiscoveringSeekStatus == DISCOVERING_SEEK_TURNING_ON_STATUS) {
                Log.e(TAG, "startDiscovery, but is DISCOVERING_SEEK_TURNING_ON_STATUS");
                try {
                    // 等待一会儿再查询
                    mObject.wait(M_OBJECT_WAIT_TIMEOUT_MS);
                    if (mDiscoveringSeekStatus != DISCOVERING_SEEK_ON_STATUS) {
                        Log.e(TAG, "after wait, is still not start discovery");
                        return false;
                    }
                    mDiscoveryHandler.removeMessages(DISCOVERY_STOP_DELAY_MSG_TYPE);
                    sendBroadcastDiscoveryStarted();
                    return true;
                } catch (InterruptedException e) {
                    Log.e(TAG, "", e);
                    return false;
                }
            }

            if (mDiscoveringSeekStatus == DISCOVERING_SEEK_TURNING_OFF_STATUS) {
                Log.e(TAG, "startDiscovery, but is DISCOVERING_SEEK_TURNING_OFF_STATUS");
                try {
                    mObject.wait(M_OBJECT_WAIT_TIMEOUT_MS);
                    if (mDiscoveringSeekStatus != DISCOVERING_SEEK_OFF_STATUS) {
                        Log.e(TAG, "after wait, is still not stop seek");
                        return false;
                    }
                } catch (InterruptedException e) {
                    Log.e(TAG, "", e);
                    return false;
                }
            }

            // 设置mDiscovering状态，设置最新的mDiscoveryEndMs
            markTurningOnDiscovery();

        }
        mDiscoveryHandler.removeMessages(DISCOVERY_STOP_DELAY_MSG_TYPE);

        // 启动扫描
        seekManager.startSeekForDiscovery(nearlinkSeekParams);

        // 设置一个超时时间
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        long current = System.currentTimeMillis();
        Log.e(TAG, "send discovery delay message at ms time :"
                + sdf.format(current) + " timeout is "
                + sdf.format(current + mTimeoutForDiscovery));
        mDiscoveryHandler.sendEmptyMessageDelayed(DISCOVERY_STOP_DELAY_MSG_TYPE, mTimeoutForDiscovery);

        return true;
    }


    public boolean cancelDiscovery() {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        return cancelDiscovery(true);
    }

    boolean startAnnounce(int timeoutMillis, NearlinkAnnounceParam nearlinkAnnounceParam,
                          NearlinkPublicData nearlinkAnnounceData,
                          NearlinkPublicData nearlinkSeekRespData,
                          INearlinkAnnounceCallback iNearlinkAnnounceCallback) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        Log.e(TAG, "service startAnnounce");
        return announceManager.startAnnounce(timeoutMillis, nearlinkAnnounceParam,
                nearlinkAnnounceData, nearlinkSeekRespData, iNearlinkAnnounceCallback);
    }

    boolean stopAnnounce(INearlinkAnnounceCallback iNearlinkAnnounceCallback) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        Log.e(TAG, "service stopAnnounce");
        return announceManager.stopAnnounce(iNearlinkAnnounceCallback);
    }

    public boolean startSeek(String seekId, NearlinkSeekParams nearlinkSeekParams, List<NearlinkSeekFilter> filters,
                             INearlinkSeekCallback iNearlinkSeekCallback) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        Log.e(TAG, "service startSeek, seekId = " + seekId);
        SeekClient scanClient = new SeekClient(seekId, SeekClient.START_SEEK_TYPE, nearlinkSeekParams,
                iNearlinkSeekCallback, filters);
        seekManager.startSeekForCallback(scanClient);
        return true;
    }

    public boolean stopSeek(String seekId) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        Log.e(TAG, "service stopSeek, seekId = " + seekId);
        seekManager.stopSeek(seekId);
        return true;
    }

    public void startSeekForIntent() {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        seekManager.startSeekForIntent();
    }

    public void stopSeekForIntent() {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        seekManager.stopSeekForIntent();
    }

    public long getDiscoveryEndMillis() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        Log.e(TAG, "service getDiscoveryEndMillis");
        return mDiscoveryEndMs;

    }

    private boolean synIsDiscovering() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        synchronized (mObject) {
            boolean isAllOnStatus = (mDiscoveringSeekStatus == DISCOVERING_SEEK_ON_STATUS);
            Log.e(TAG, "isAllOnStatus = " + isAllOnStatus + " mDiscoveringSeekStatus = " + mDiscoveringSeekStatus);
            return isAllOnStatus;
        }
    }

    public boolean isDiscoverySeekOn() {
        boolean isDiscoverySeekOnStatus = (mDiscoveringSeekStatus == DISCOVERING_SEEK_ON_STATUS);
        return isDiscoverySeekOnStatus;
    }

    public boolean isDiscoverySeekOffOrTuringOff() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        boolean isDiscoverySeekOffOrTuringOff = ((mDiscoveringSeekStatus == DISCOVERING_SEEK_OFF_STATUS)
                || (mDiscoveringSeekStatus == DISCOVERING_SEEK_TURNING_OFF_STATUS));
        Log.e(TAG,
                "isDiscoverySeekOffOrTuringOff = " + isDiscoverySeekOffOrTuringOff + " mDiscoveringSeekStatus = " + mDiscoveringSeekStatus);
        return isDiscoverySeekOffOrTuringOff;
    }

    protected boolean isAvailable() {
        return mProfileStarted;
    }

    public long getDefaultDiscoveryTimeoutMs() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        return DEFAULT_DISCOVERY_TIMEOUT_MS;
    }

    public boolean setTimeoutForDiscovery(int timeout) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        if ((timeout >= DEFAULT_DISCOVERY_TIMEOUT_MS)
                && (timeout <= MAX_DISCOVERY_TIMEOUT_MS)) {
            this.mTimeoutForDiscovery = timeout;
        }
        return true;
    }

    public int getTimeoutForDiscovery() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        return this.mTimeoutForDiscovery;
    }

    public synchronized boolean setAnnounceMode(int mode, int duration) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK_ADMIN permission");
        mAnnounceModeForDiscovery = mode;
        if ((duration >= DEFAULT_DISCOVERY_TIMEOUT_MS)
                && (duration <= MAX_DISCOVERY_TIMEOUT_MS)) {
            this.mTimeoutForDiscovery = duration;
        }
        Intent intent;
        intent = new Intent(NearlinkAdapter.ACTION_ANNOUNCE_MODE_CHANGED);
        intent.putExtra(NearlinkAdapter.EXTRA_ANNOUNCE_MODE, mAnnounceModeForDiscovery);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        sendBroadcast(intent, AdapterService.NEARLINK_PERM);
        Log.e(TAG, "mAnnounceModeForDiscovery :" + mAnnounceModeForDiscovery);
        return true;
    }

    public int getAnnounceMode() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        Log.e(TAG, "service getAnnounceMode");
        return mAnnounceModeForDiscovery;
    }

    private static class NearlinkDiscoveryBinder extends INearlinkDiscovery.Stub implements IProfileServiceBinder {

        private DiscoveryService mDiscoveryService;

        NearlinkDiscoveryBinder(DiscoveryService svc) {
            mDiscoveryService = svc;
        }

        @Override
        public void cleanup() {
            mDiscoveryService = null;
        }

        private DiscoveryService getService() {
            if (mDiscoveryService != null && mDiscoveryService.isAvailable()) {
                return mDiscoveryService;
            }

            if (mDiscoveryService == null) {
                Log.e(TAG, "mDiscoveryService is null");
            }

            if (!mDiscoveryService.isAvailable()) {
                Log.e(TAG, "isAvailable false");
            }

            Log.e(TAG, "getService() - Service requested, but not available!");

            return null;
        }

        @Override
        public boolean startAnnounce(int timeoutMillis, NearlinkAnnounceParam nearlinkAnnounceParam,
                                     NearlinkPublicData nearlinkAnnounceData,
                                     NearlinkPublicData nearlinkSeekRespData,
                                     INearlinkAnnounceCallback iNearlinkAnnounceCallback) throws RemoteException {
            Log.e(TAG, "binder startAnnounce, announceId = ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder startAnnounce service is null");
                return false;
            }
            return service.startAnnounce(timeoutMillis, nearlinkAnnounceParam,
                    nearlinkAnnounceData, nearlinkSeekRespData, iNearlinkAnnounceCallback);
        }

        @Override
        public boolean stopAnnounce(INearlinkAnnounceCallback iNearlinkAnnounceCallback) throws RemoteException {
            Log.e(TAG, "binder stopAnnounce");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder stopAnnounce service is null");
                return false;
            }
            return service.stopAnnounce(iNearlinkAnnounceCallback);
        }

        @Override
        public boolean startSeek(String seekId, NearlinkSeekParams nearlinkSeekParams,
                                 List<NearlinkSeekFilter> filters, INearlinkSeekCallback iNearlinkSeekCallback) {
            Log.e(TAG, "binder startSeek");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder startSeek service is null");
                return false;
            }
            return service.startSeek(seekId, nearlinkSeekParams, filters, iNearlinkSeekCallback);
        }


        @Override
        public boolean stopSeek(String seekId) {
            Log.e(TAG, "binder stopSeek");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder stopSeek service is null");
                return false;
            }
            return service.stopSeek(seekId);
        }

        @Override
        public int getAnnounceMode() throws RemoteException {
            Log.e(TAG, "binder getAnnounceMode entry ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder getAnnounceMode, but service is null ");
                return NearlinkAdapter.ANNOUNCE_MODE_NONE;
            }
            return service.getAnnounceMode();
        }

        @Override
        public boolean setAnnounceMode(int mode, int duration) throws RemoteException {
            Log.e(TAG, "binder setAnnounceMode entry ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder setAnnounceMode, but service is null ");
                return false;
            }
            return service.setAnnounceMode(mode, duration);
        }

        @Override
        public boolean setTimeoutForDiscovery(int timeout) throws RemoteException {
            Log.e(TAG, "binder setAnnounceTimeoutForDiscovery entry ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder setAnnounceTimeoutForDiscovery, but service is null ");
                return false;
            }
            return service.setTimeoutForDiscovery(timeout);
        }

        @Override
        public long getDiscoveryEndMillis() throws RemoteException {
            Log.e(TAG, "binder getDiscoveryEndMillis entry ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder getDiscoveryEndMillis, but service is null ");
                return NearlinkAdapter.DISCOVERY_END_MILLIS_ERROR;
            }
            return service.getDiscoveryEndMillis();
        }

        @Override
        public boolean isDiscovering() throws RemoteException {
            Log.e(TAG, "binder isDiscovering entry ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder isDiscovering, but service is null ");
                return false;
            }
            return service.synIsDiscovering();
        }

        @Override
        public boolean startDiscovery(NearlinkSeekParams nearlinkSeekParams,
                                      NearlinkAnnounceParam nearlinkAnnounceParam,
                                      NearlinkPublicData nearlinkAnnounceData,
                                      NearlinkPublicData nearlinkSeekRespData) throws RemoteException {
            Log.e(TAG, "binder startDiscovery entry ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder startDiscovery, but service is null ");
                return false;
            }
            return service.startDiscovery(nearlinkSeekParams, nearlinkAnnounceParam,
                    nearlinkAnnounceData, nearlinkSeekRespData);
        }

        @Override
        public boolean cancelDiscovery() throws RemoteException {
            Log.e(TAG, "binder cancelDiscovery entry ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder cancelDiscovery, but service is null ");
                return false;
            }
            return service.cancelDiscovery();
        }

        @Override
        public int getTimeoutForDiscovery() throws RemoteException {
            Log.e(TAG, "binder getAnnounceTimeoutForDiscovery entry ");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder getAnnounceTimeoutForDiscovery, but service is null ");
                return -1;
            }
            return service.getTimeoutForDiscovery();
        }

        @Override
        public void startSeekForIntent() throws RemoteException {
            Log.e(TAG, "binder startSeekForIntent");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder startSeekForIntent, but service is null ");
                return;
            }
            service.startSeekForIntent();
        }

        @Override
        public void stopSeekForIntent() throws RemoteException {
            Log.e(TAG, "binder stopSeekForIntent");
            DiscoveryService service = getService();
            if (service == null) {
                Log.e(TAG, "binder stopSeekForIntent, but service is null ");
                return;
            }
            service.stopSeekForIntent();
        }
    }

    private void debugLog(String msg) {
        Log.e(TAG, msg);
    }

    private class DiscoveryHandler extends Handler {

        DiscoveryHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.e(TAG, "handleMessage,msg.what=" + msg.what);
            switch (msg.what) {
                case DISCOVERY_STOP_DELAY_MSG_TYPE:
                    SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                    Log.e(TAG,
                            "receive stop discovery delay message at ms time :" + sdf.format(System.currentTimeMillis()));
                    cancelDiscovery(false);
                    break;
                default:
                    // Shouldn't happen.
                    Log.e(TAG, "received an unkown message : " + msg.what);
            }
        }
    }

    private void markTurningOnDiscovery() {

        synUpdateDiscoveringSeekStatus(DISCOVERING_SEEK_TURNING_ON_STATUS);
        mDiscoveryEndMs = System.currentTimeMillis() +
                (DEFAULT_DISCOVERY_TIMEOUT_MS > mTimeoutForDiscovery ?
                        DEFAULT_DISCOVERY_TIMEOUT_MS : mTimeoutForDiscovery);

    }

    public void markTurningOffDiscovery() {

        synUpdateDiscoveringSeekStatus(DISCOVERING_SEEK_TURNING_OFF_STATUS);
        mDiscoveryEndMs = System.currentTimeMillis();

    }

    private boolean cancelDiscovery(boolean isRemoveDelayMessageFlag) {
        Log.e(TAG, "service cancelDiscovery");

        // 清除定时消息
        if (isRemoveDelayMessageFlag) {
            mDiscoveryHandler.removeMessages(DISCOVERY_STOP_DELAY_MSG_TYPE);
        }

        synchronized (mObject) {

            if (mDiscoveringSeekStatus == DISCOVERING_SEEK_OFF_STATUS) {
                Log.e(TAG, "cancelDiscovery, is already cancel discovery!!!");
                sendBroadcastDiscoveryFinished();
                return true;
            }

            if (mDiscoveringSeekStatus == DISCOVERING_SEEK_TURNING_OFF_STATUS) {
                Log.e(TAG, "cancelDiscovery, but is DISCOVERING_SEEK_TURNING_OFF_STATUS");
                try {
                    mObject.wait(M_OBJECT_WAIT_TIMEOUT_MS);
                    if (mDiscoveringSeekStatus != DISCOVERING_SEEK_OFF_STATUS) {
                        Log.e(TAG, "after wait, is still not stop seek");
                        return false;
                    }
                    sendBroadcastDiscoveryFinished();
                    return true;
                } catch (InterruptedException e) {
                    Log.e(TAG, "", e);
                    return false;
                }
            }

            if (mDiscoveringSeekStatus == DISCOVERING_SEEK_TURNING_ON_STATUS) {
                Log.e(TAG, "cancelDiscovery, but is DISCOVERING_SEEK_TURNING_ON_STATUS");
                try {
                    mObject.wait(M_OBJECT_WAIT_TIMEOUT_MS);
                    if (mDiscoveringSeekStatus != DISCOVERING_SEEK_ON_STATUS) {
                        Log.e(TAG, "after wait, is still not start discovery");
                        return false;
                    }
                } catch (InterruptedException e) {
                    Log.e(TAG, "", e);
                    return false;
                }
            }

            // 设置mDiscovering状态，设置最新的mDiscoveryEndMs
            markTurningOffDiscovery();

        }
        // 关闭扫描
        seekManager.stopSeekForDiscovery();
        return true;
    }

    public void synMarkSeekOnDiscovery() {
        synchronized (mObject) {
            if (mDiscoveringSeekStatus != DISCOVERING_SEEK_TURNING_ON_STATUS) {
                Log.e(TAG,
                        "synMarkSeekOnDiscovery but mDiscoveringSeekStatus is not turning on status : " + mDiscoveringSeekStatus);
                return;
            }
            synUpdateDiscoveringSeekStatus(DISCOVERING_SEEK_ON_STATUS);
            sendBroadcastDiscoveryStarted();
            mObject.notifyAll();
        }
    }

    public void synMarkSeekOffDiscovery() {
        synchronized (mObject) {
            if (mDiscoveringSeekStatus != DISCOVERING_SEEK_TURNING_OFF_STATUS) {
                Log.e(TAG,
                        "synMarkSeekOffDiscovery but mDiscoveringSeekStatus is not turning off status : " + mDiscoveringSeekStatus);
                return;
            }
            synUpdateDiscoveringSeekStatus(DISCOVERING_SEEK_OFF_STATUS);
            sendBroadcastDiscoveryFinished();
            mObject.notifyAll();
        }
    }

    public void synMarkSeekOnDiscoveryError() {
        synchronized (mObject) {
            if (mDiscoveringSeekStatus != DISCOVERING_SEEK_TURNING_ON_STATUS) {
                Log.e(TAG,
                        "synMarkSeekOnDiscoveryError but mDiscoveringSeekStatus is not turning on status : " + mDiscoveringSeekStatus);
                return;
            }
            synUpdateDiscoveringSeekStatus(DISCOVERING_SEEK_ON_ERROR_STATUS);
            sendBroadcastDiscoveryStarted();
            mObject.notifyAll();
        }
    }

    public void synMarkSeekOffDiscoveryError() {
        synchronized (mObject) {
            if (mDiscoveringSeekStatus != DISCOVERING_SEEK_TURNING_OFF_STATUS) {
                Log.e(TAG,
                        "synMarkSeekOffDiscoveryError but mDiscoveringSeekStatus is not turning off status : " + mDiscoveringSeekStatus);
                return;
            }
            synUpdateDiscoveringSeekStatus(DISCOVERING_SEEK_OFF_ERROR_STATUS);
            sendBroadcastDiscoveryFinished();
            mObject.notifyAll();
        }
    }

    private void sendBroadcastDiscoveryStarted() {
        Intent intent;
        intent = new Intent(NearlinkAdapter.ACTION_DISCOVERY_STARTED);
        sendBroadcast(intent, AdapterService.NEARLINK_PERM);
    }

    private void sendBroadcastDiscoveryFinished() {
        Intent intent;
        intent = new Intent(NearlinkAdapter.ACTION_DISCOVERY_FINISHED);
        sendBroadcast(intent, AdapterService.NEARLINK_PERM);
    }

    private void synUpdateDiscoveringSeekStatus(int status) {
        Log.e(TAG, "mDiscoveringSeekStatus pre: " + mDiscoveringSeekStatus + " new: " + status);
        synchronized (mObject) {
            mDiscoveringSeekStatus = status;
        }
    }

}
