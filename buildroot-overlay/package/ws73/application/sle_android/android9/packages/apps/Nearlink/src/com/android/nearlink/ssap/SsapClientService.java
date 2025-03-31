/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.ssap;

import android.content.BroadcastReceiver;
import android.nearlink.INearlinkSsapClient;
import android.nearlink.INearlinkSsapClientCallback;
import android.nearlink.NearlinkErrorCode;
import android.nearlink.NearlinkSsapDescriptor;
import android.nearlink.NearlinkSsapProperty;
import android.nearlink.NearlinkSsapReadByUuid;
import android.nearlink.NearlinkSsapService;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.util.Log;

import com.android.nearlink.connection.PubTools;
import com.android.nearlink.nlservice.ProfileService;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * 星闪SSAP客户端支撑服务
 *
 * @since 2023-12-01
 */
public class SsapClientService extends ProfileService {
    private static final boolean IS_DEBUG = true;
    private static final String TAG = "SSAP-Client-SRV";

    /**
     * 未连接状态
     */
    private static final int CONNECTION_STATE_NONE = 0;

    /**
     * 已连接状态
     */
    private static final int CONNECTION_STATE_CONNECTED = 1;

    /**
     * 已断连状态
     */
    private static final int CONNECTION_STATE_DISCONNECTED = 2;

    /**
     * 未配对
     */
    private static final int SLE_PAIR_NONE = 1;

    /**
     * 配对中
     */
    private static final int SLE_PAIR_PAIRING = 2;

    /**
     * 已配对
     */
    private static final int SLE_PAIR_PAIRED = 3;

    static class ClientMap extends ContextMap<INearlinkSsapClientCallback> {
    }

    BroadcastReceiver mConnectionStateChangeReceiver = null;
    ClientMap mClientMap = new ClientMap();
    final Object cacheLock = new Object();

    static {
        classInitNative();
    }

    private static SsapClientService sSsapClientService;

    /**
     * 获取SSAP客户端支撑服务单例
     *
     * @return SSAP客户端支撑服务单例
     */
    public static synchronized SsapClientService getSsapClientService() {
        if (sSsapClientService == null) {
            Log.e(TAG, "getSsapClientService(): service is null");
            return null;
        }
        if (!sSsapClientService.isAvailable()) {
            Log.e(TAG, "getSsapClientService(): service is not available");
            return null;
        }
        return sSsapClientService;
    }

    private static synchronized void setSsapClientService(SsapClientService instance) {
        sSsapClientService = instance;
    }

    @Override
    protected IProfileServiceBinder initBinder() {
        return new SsapClientBinder(this);
    }

    @Override
    protected boolean start() {
        if (IS_DEBUG) {
            Log.d(TAG, "start()");
        }
        initializeNative();
        setSsapClientService(this);
        return true;
    }

    @Override
    protected boolean stop() {
        if (IS_DEBUG) {
            Log.d(TAG, "stop()");
        }
        setSsapClientService(null);
        mClientMap.clear();
        return true;
    }

    @Override
    protected void cleanup() {
        if (IS_DEBUG) {
            Log.d(TAG, "cleanup()");
        }
        cleanupNative();
    }

    class ClientDeathRecipient implements IBinder.DeathRecipient {
        int mClientId;

        ClientDeathRecipient(int clientId) {
            mClientId = clientId;
        }

        @Override
        public void binderDied() {
            if (IS_DEBUG) {
                Log.d(TAG, "Binder is dead - unregistering client (" + mClientId + ")!");
            }
            unregisterClient(mClientId);
        }
    }

    static class SsapClientBinder extends INearlinkSsapClient.Stub implements IProfileServiceBinder {

        SsapClientService mService;

        SsapClientBinder(SsapClientService service) {
            mService = service;
        }

        private SsapClientService getService() {
            if (mService != null && mService.isAvailable()) {
                return mService;
            }
            Log.e(TAG, "getService() - Service requested, but not available!");
            return null;
        }

        @Override
        public boolean registerClient(ParcelUuid appUuid, String address, INearlinkSsapClientCallback callback) throws RemoteException {
            clearCallingIdentity();
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.registerClient(appUuid.getUuid(), address, callback);
        }

        @Override
        public boolean unregisterClient(int clientId) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.unregisterClient(clientId);
        }

        @Override
        public boolean connect(int clientId, String address, int addressType) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.clientConnect(clientId, address, addressType);
        }

        @Override
        public void disconnect(int clientId, String address, int addressType) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return;
            }
            service.clientDisconnect(clientId, address, addressType);
        }

        @Override
        public boolean discoverServices(int clientId, String address) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.discoverServices(clientId, address);
        }

        @Override
        public boolean readRequestByUuid(int clientId, String address, int type, int startHandle, int endHandle,
                                         ParcelUuid uuid) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.readRequestByUuid(clientId, address, type, startHandle, endHandle, uuid.getUuid());
        }

        @Override
        public boolean readRequest(int clientId, String address, int handle, int type) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.readRequest(clientId, address, handle, type);
        }

        @Override
        public boolean writeRequest(int clientId, String address, int handle, int type, boolean shouldResponse,
                                    byte[] data) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.writeRequest(clientId, address, handle, type, shouldResponse, data);
        }

        @Override
        public boolean exchangeInfoRequest(int clientId, String address, int mtuSize, int version)
                throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.exchangeInfoRequest(clientId, address, mtuSize, version);
        }

        @Override
        public boolean readRemoteRssi(int clientId, String address) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.readRemoteRssi(clientId, address);
        }

        @Override
        public boolean connectionParameterUpdate(int clientId, String address, int minInterval, int maxInterval,
                                                 int maxLatency, int supervisionTimeout) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.connectionParameterUpdate(clientId, address, minInterval, maxInterval, maxLatency,
                    supervisionTimeout);
        }

        @Override
        public boolean setTxDataLength(int clientId, int txOctets) throws RemoteException {
            SsapClientService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.setTxDataLength(clientId, txOctets);
        }

        @Override
        public void cleanup() {
            mService = null;
        }
    }

    boolean registerClient(UUID uuid, String address, INearlinkSsapClientCallback callback) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "registerClient() - UUID=" + uuid);
        }
        mClientMap.add(uuid, address, callback, this);
        return registerClientNative(uuid.getLeastSignificantBits(),
                uuid.getMostSignificantBits()) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean unregisterClient(int clientId) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "unregisterClient() - clientId=" + clientId);
        }
        mClientMap.remove(clientId);
        return unregisterClientNative(clientId) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean clientConnect(int clientId, String address, int addressType) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG,
                    "clientConnect() - clientId=" + clientId + " address=" + PubTools.macPrint(address)
                            + " addressType=" + addressType);
        }
        return connectNative(clientId, addressType, PubTools.hexToByteArray(address))
                == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    void clientDisconnect(int clientId, String address, int addressType) throws RemoteException {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG,
                    "clientDisconnect() - clientId=" + clientId + " address=" + PubTools.macPrint(address)
                            + " addressType=" + addressType);
        }
        disconnectNative(clientId, addressType, PubTools.hexToByteArray(address));
    }

    ContextMap<INearlinkSsapClientCallback>.App getAppByClientId(int clientId) {
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return null;
        }
        if (app.connId == ContextMap.App.INVALID_CONN_ID) {
            Log.e(TAG, "client " + clientId + " disconnected.");
            return null;
        }
        return app;
    }

    boolean discoverServices(int clientId, String address) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "discoverServices() - address=" + PubTools.macPrint(address));
        }
        ContextMap<INearlinkSsapClientCallback>.App app = getAppByClientId(clientId);
        if (app == null) {
            return false;
        }

        return findStructureNative(clientId, app.connId) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean readRequestByUuid(int clientId, String address, int type, int startHandle, int endHandle, UUID uuid)
            throws IllegalStateException {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "readRequestByUuid() - address=" + PubTools.macPrint(address) + " type=" + type
                    + " startHandle=" + startHandle + " endHandle=" + endHandle + " uuid=" + uuid.toString());
        }
        ContextMap<INearlinkSsapClientCallback>.App app = getAppByClientId(clientId);
        if (app == null) {
            return false;
        }

        synchronized (cacheLock) {
            if (app.isCacheLocked) {
                throw new IllegalStateException("can not send request by uuid now, waiting for previous request done");
            }
            app.isCacheLocked = true;
            app.cache = new ArrayList<NearlinkSsapReadByUuid>();
        }
        return readRequestByUuidNative(clientId, app.connId, type, startHandle, endHandle,
                uuid.getLeastSignificantBits(), uuid.getMostSignificantBits()) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean readRequest(int clientId, String address, int handle, int type) throws IllegalStateException {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "readRequest() - address=" + PubTools.macPrint(address)
                    + " handle=" + handle + " type=" + type);
        }
        ContextMap<INearlinkSsapClientCallback>.App app = getAppByClientId(clientId);
        if (app == null) {
            return false;
        }

        synchronized (cacheLock) {
            if (app.isCacheLocked) {
                throw new IllegalStateException("can not send request by uuid now, waiting for previous request done");
            }
            app.isCacheLocked = true;
        }
        return readRequestNative(clientId, app.connId, handle, type) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean writeRequest(int clientId, String address, int handle, int type, boolean shouldResponse, byte[] data) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "writeRequest() - address=" + PubTools.macPrint(address)
                    + " handle=" + handle + " type=" + type);
        }
        ContextMap<INearlinkSsapClientCallback>.App app = getAppByClientId(clientId);
        if (app == null) {
            return false;
        }
        return writeRequestNative(clientId, app.connId, handle, type,
                shouldResponse, data) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean exchangeInfoRequest(int clientId, String address, int mtuSize, int version) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "exchangeInfoRequest() - address=" + PubTools.macPrint(address)
                    + " mtuSize=" + mtuSize + " version=" + version);
        }
        ContextMap<INearlinkSsapClientCallback>.App app = getAppByClientId(clientId);
        if (app == null) {
            return false;
        }
        return exchangeInfoRequestNative(clientId, app.connId,
                mtuSize, version) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean readRemoteRssi(int clientId, String address) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "readRemoteRssi() - clientId=" + clientId + " address=" + PubTools.macPrint(address));
        }
        ContextMap<INearlinkSsapClientCallback>.App app = getAppByClientId(clientId);
        if (app == null) {
            return false;
        }
        return readRemoteRssiNative(clientId, app.connId) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean connectionParameterUpdate(int clientId, String address, int minInterval, int maxInterval,
                                      int maxLatency, int supervisionTimeout) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "connectionParameterUpdate() - clientId=" + clientId + " address="
                    + PubTools.macPrint(address) + " minInterval=" + minInterval + " maxInterval=" + maxInterval
                    + " maxLatency=" + maxLatency + " supervisionTimeout=" + supervisionTimeout);
        }
        ContextMap<INearlinkSsapClientCallback>.App app = getAppByClientId(clientId);
        if (app == null) {
            return false;
        }
        return connectionParameterUpdateNative(clientId, app.connId, minInterval, maxInterval,
                maxLatency, supervisionTimeout) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean setTxDataLength(int clientId, int txOctets) {
        Log.d(TAG, "setTxDataLength.....clientId:" + clientId + ", txOctets:" + txOctets);
        ContextMap<INearlinkSsapClientCallback>.App app = getAppByClientId(clientId);
        if (app == null) {
            return false;
        }
        int ret = setTxDataLengthNative(clientId, app.connId, txOctets);
        Log.d(TAG, "setTxDataLength.....ret:" + ret);
        return ret == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    /* *************************************************************************
     * JNI回调方法
     * ************************************************************************/
    void onClientRegistered(int clientId, long uuidMsb, long uuidLsb, int status) throws RemoteException {
        UUID uuid = new UUID(uuidMsb, uuidLsb);
        if (IS_DEBUG) {
            Log.d(TAG, "onClientRegistered() - clientId=" + clientId + " uuid=" + uuid + " status=" + status);
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getByUuid(uuid);
        if (app != null) {
            if (status == 0) {
                app.id = clientId;
                app.linkToDeath(new ClientDeathRecipient(clientId));
            }
            app.callback.onRegistered(clientId, status);
        }
    }

    void onConnectionStateChanged(int clientId, int connId, int addressType, byte[] address, int connState,
                                  int pairState, int disConnReason) throws RemoteException {
        String strAddress = PubTools.bytesToHex(address);
        if (IS_DEBUG) {
            Log.d(TAG, "onConnectionStateChanged() - clientId=" + clientId + " connId=" + connId + " addressType="
                    + addressType + " address=" + PubTools.macPrint(strAddress) + " connState=" + connState
                    + " pairState=" + pairState + " disConnReason=" + disConnReason);
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }
        if (connState == CONNECTION_STATE_CONNECTED) {
            app.connId = connId;
        } else {
            app.connId = ContextMap.App.INVALID_CONN_ID;
        }
        if (connState == CONNECTION_STATE_CONNECTED && pairState != SLE_PAIR_PAIRED) { // 只连接成功但尚未配对成功时，先不通知
            return;
        }
        app.callback.onConnectionStateChanged(clientId, strAddress, addressType, connState);
    }

    void onStructureFoundComplete(int clientId, int connId, int status) {
        if (IS_DEBUG) {
            Log.d(TAG, "onStructureFoundComplete() - clientId=" + clientId + " connId=" + connId
                    + " status=" + status);
        }
        if (status == NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            getStructureNative(clientId, connId);
        }
    }

    NearlinkSsapService newSsapService(ArrayList<NearlinkSsapService> services, long uuidMsb, long uuidLsb,
                                       int handle) {
        NearlinkSsapService service = new NearlinkSsapService(new UUID(uuidMsb, uuidLsb));
        service.setHandle(handle);
        services.add(service);
        return service;
    }

    NearlinkSsapProperty newSsapProperty(NearlinkSsapService service, long uuidMsb, long uuidLsb, int handle,
                                         int operateIndication) {
        NearlinkSsapProperty property = new NearlinkSsapProperty(new UUID(uuidMsb, uuidLsb),
                0, operateIndication);
        property.setHandle(handle);
        service.addProperty(property);
        return property;
    }

    void newSsapDescriptor(NearlinkSsapProperty prop, int type) {
        NearlinkSsapDescriptor descriptor = new NearlinkSsapDescriptor(type);
        prop.addDescriptor(descriptor);
    }

    void onStructureGot(int clientId, int connId, ArrayList<NearlinkSsapService> structure)
            throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onStructureGot() - clientId=" + clientId + " connId=" + connId);
        }
        List<NearlinkSsapService> services = new ArrayList<>(structure);
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }
        app.callback.onSearchComplete(app.address, services, NearlinkErrorCode.ERRCODE_SLE_SUCCESS);
    }

    void onReadCfm(int clientId, int connId, int handle, int type, byte[] data, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onReadCfm() - clientId=" + clientId + " connId=" + connId + " handle=" + handle
                    + " type=" + type + " status=" + status);
        }
        if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            return;
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }
        synchronized (cacheLock) {
            //正在处理readByUuid，将其加入到缓存中就返回
            if (app.cache != null) {
                List<NearlinkSsapReadByUuid> list = (List<NearlinkSsapReadByUuid>) app.cache;
                list.add(new NearlinkSsapReadByUuid(handle, type, data));
                return;
            }
            //如果仅仅是处理的readReq，则直接将lock状态置为false并往上回调
            app.isCacheLocked = false;
        }
        app.callback.onReadCfm(app.address, handle, type, data, status);
    }

    void onReadByUuidComplete(int clientId, int connId, long uuidMsb, long uuidLsb, int type, int status)
            throws RemoteException {
        UUID uuid = new UUID(uuidMsb, uuidLsb);
        if (IS_DEBUG) {
            Log.d(TAG, "onReadByUuidComplete() - clientId=" + clientId + " connId=" + connId + " uuid=" + uuid
                    + " type=" + type + " status=" + status);
        }
        if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            return;
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }

        List<NearlinkSsapReadByUuid> data = (List<NearlinkSsapReadByUuid>) app.cache;
        app.callback.onReadByUuidComplete(app.address, new ParcelUuid(uuid), data, status);
        synchronized (cacheLock) {
            app.isCacheLocked = false;
            app.cache = null;
        }
    }

    void onWriteCfm(int clientId, int connId, int handle, int type, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onWriteCfm() - clientId=" + clientId + " connId=" + connId + " handle=" + handle
                    + " type=" + type + " status=" + status);
        }
        if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            return;
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }
        app.callback.onWriteCfm(app.address, handle, type, status);
    }

    void onInfoExchanged(int clientId, int connId, int mtuSize, int version, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onInfoExchanged() - clientId=" + clientId + " connId=" + connId + " mtuSize=" + mtuSize
                    + " version=" + version + " status=" + status);
        }
        if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            return;
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }
        app.callback.onInfoExchanged(app.address, mtuSize, version, status);
    }

    void onNotification(int clientId, int connId, int handle, int type, boolean isNotify, byte[] data, int status)
            throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onNotification() - clientId=" + clientId + " connId=" + connId + " handle=" + handle
                    + " type=" + type + " isNotify=" + isNotify + " status=" + status);
        }
        if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            return;
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }
        app.callback.onNotification(app.address, handle, type, isNotify, data, status);
    }

    void onReadRemoteRssi(int clientId, int connId, int rssi, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onReadRemoteRssi() - clientId=" + clientId + " connId=" + connId + " rssi="
                    + rssi + " status=" + status);
        }
        if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            return;
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }
        app.callback.onReadRemoteRssi(app.address, rssi, status);
    }

    void onConnectionParameterUpdated(int clientId, int connId, int interval, int latency, int timeout, int status)
            throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onReadRemoteRssi() - clientId=" + clientId + " connId=" + connId + " interval="
                    + interval + " latency=" + latency + " timeout=" + timeout + " status=" + status);
        }
        if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            return;
        }
        ContextMap<INearlinkSsapClientCallback>.App app = mClientMap.getById(clientId);
        if (app == null) {
            Log.e(TAG, "client " + clientId + " can not found.");
            return;
        }
        app.callback.onConnectionParameterUpdated(app.address, interval, latency, timeout, status);
    }

    /* *************************************************************************
     * JNI 方法
     * ************************************************************************/
    private static native void classInitNative();

    private native void initializeNative();

    private native void cleanupNative();

    private native int registerClientNative(long uuidLsb, long uuidMsb);

    private native int unregisterClientNative(int clientId);

    private native int connectNative(int clientId, int addressType, byte[] address);

    private native int disconnectNative(int clientId, int addressType, byte[] address);

    private native int findStructureNative(int clientId, int connId);

    private native void getStructureNative(int clientId, int connId);

    private native int readRequestByUuidNative(int clientId, int connId, int type, int startHandle, int endHandle,
                                               long propertyUuidLsb, long propertyUuidMsb);

    private native int readRequestNative(int clientId, int connId, int handle, int type);

    private native int writeRequestNative(int clientId, int connId, int handle, int type, boolean shouldResponse,
                                          byte[] data);

    private native int exchangeInfoRequestNative(int clientId, int connId, int mtuSize, int version);

    private native int readRemoteRssiNative(int clientId, int connId);

    private native int connectionParameterUpdateNative(int clientId, int connId, int minInterval, int maxInterval,
                                                       int latency, int timeout);
                                                       
    /**
     * 设置连接链路上的最大传输payload字节数
     * 
     * @param connId    连接 ID
     * @param txOctets 连接链路上所偏好的最大传输payload字节数
     * @return 执行状态码
     */
    private native int setTxDataLengthNative(int clientId, int connId, int txOctets);
}
