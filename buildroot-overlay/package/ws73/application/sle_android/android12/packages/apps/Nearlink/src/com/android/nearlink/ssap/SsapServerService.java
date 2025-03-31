/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.ssap;

import android.nearlink.INearlinkSsapServer;
import android.nearlink.INearlinkSsapServerCallback;
import android.nearlink.NearlinkErrorCode;
import android.nearlink.NearlinkSsapDescriptor;
import android.nearlink.NearlinkSsapProperty;
import android.nearlink.NearlinkSsapService;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.util.Log;

import com.android.nearlink.nlservice.ProfileService;

import java.util.UUID;

/**
 * 星闪SSAP服务端支撑服务
 *
 * @since 2023-12-01
 */
public class SsapServerService extends ProfileService {
    private static final boolean IS_DEBUG = true;
    private static final String TAG = "SSAP-Server-SRV";

    static class ServerMap extends ContextMap<INearlinkSsapServerCallback> {
    }

    ServerMap mServerMap = new ServerMap();

    // If support multi server instance, then put follows into server instance.
    private static final int INVALID_HANDLE = -1;

    NearlinkSsapService mPendingService = null;
    int mPendingServiceHandle = INVALID_HANDLE;
    NearlinkSsapProperty mPendingProperty = null;
    int mPendingPropertyHandle = INVALID_HANDLE;

    static {
        classInitNative();
    }

    private static SsapServerService sSsapServerService;

    public static synchronized SsapServerService getSsapServerService() {
        if (sSsapServerService == null) {
            Log.e(TAG, "getSsapClientService(): service is null");
            return null;
        }
        if (!sSsapServerService.isAvailable()) {
            Log.e(TAG, "getSsapServerService(): service is not available");
            return null;
        }
        return sSsapServerService;
    }

    private static synchronized void setSsapServerService(SsapServerService instance) {
        sSsapServerService = instance;
    }

    @Override
    protected IProfileServiceBinder initBinder() {
        return new SsapServerBinder(this);
    }

    @Override
    protected boolean start() {
        if (IS_DEBUG) {
            Log.d(TAG, "start()");
        }
        initializeNative();
        setSsapServerService(this);
        return true;
    }

    @Override
    protected boolean stop() {
        if (IS_DEBUG) {
            Log.d(TAG, "stop()");
        }
        setSsapServerService(null);
        mServerMap.clear();
        return true;
    }

    @Override
    protected void cleanup() {
        if (IS_DEBUG) {
            Log.d(TAG, "cleanup()");
        }
        cleanupNative();
    }

    class ServerDeathRecipient implements IBinder.DeathRecipient {
        int mServerId;

        ServerDeathRecipient(int serverId) {
            mServerId = serverId;
        }

        @Override
        public void binderDied() {
            if (IS_DEBUG) {
                Log.d(TAG, "Binder is dead - unregistering server (" + mServerId + ")!");
            }
            unregisterServer(mServerId);
        }
    }

    static class SsapServerBinder extends INearlinkSsapServer.Stub implements IProfileServiceBinder {
        SsapServerService mService;

        SsapServerBinder(SsapServerService service) {
            mService = service;
        }

        private SsapServerService getService() {
            if (mService != null && mService.isAvailable()) {
                return mService;
            }
            Log.e(TAG, "getService() - Service requested, but not available!");
            return null;
        }

        @Override
        public boolean registerServer(ParcelUuid appId, INearlinkSsapServerCallback callback) throws RemoteException {
            SsapServerService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.registerServer(appId.getUuid(), callback);
        }

        @Override
        public boolean unregisterServer(int serverId) throws RemoteException {
            SsapServerService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.unregisterServer(serverId);
        }

        @Override
        public boolean addService(int serverId, boolean isPrimary, NearlinkSsapService ssapService)
                throws RemoteException {
            SsapServerService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.addService(serverId, isPrimary, ssapService);
        }

        @Override
        public boolean deleteAllServices(int serverId) throws RemoteException {
            SsapServerService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.deleteAllServices(serverId);
        }

        @Override
        public boolean sendResponse(int serverId, String address, int requestId, int status, byte[] value)
                throws RemoteException {
            SsapServerService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.sendResponse(serverId, address, requestId, status, value);
        }

        @Override
        public boolean notifyOrIndicate(int serverId, String address, int handle, int type, byte[] value)
                throws RemoteException {
            SsapServerService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.notifyOrIndicate(serverId, address, handle, type, value);
        }

        @Override
        public boolean setInfo(int serverId, int mtuSize, int version) throws RemoteException {
            SsapServerService service = getService();
            if (service == null) {
                Log.e(TAG, "Service not available!");
                return false;
            }
            return service.setInfo(serverId, mtuSize, version);
        }

        @Override
        public void cleanup() {
            mService = null;
        }
    }

    boolean registerServer(UUID uuid, INearlinkSsapServerCallback callback) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "registerServer()");
        }
        mServerMap.add(uuid, null, callback, this);
        return registerServerNative(uuid.getLeastSignificantBits(),
                uuid.getMostSignificantBits()) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean unregisterServer(int serverId) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "unregisterServer() - serverId=" + serverId);
        }
        mServerMap.remove(serverId);
        return unregisterServerNative(serverId) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean addService(int serverId, boolean isPrimary, NearlinkSsapService ssapService) throws RemoteException {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "addService() - serverId= " + serverId + " isPrimary=" + isPrimary);
        }
        synchronized (this) {
            if (mPendingService != null) {
                Log.e(TAG, "other service is adding, please try later.");
                return false;
            }
            mPendingService = ssapService;
        }
        UUID uuid = mPendingService.getUuid();
        if (addServiceSyncNative(serverId, uuid.getLeastSignificantBits(), uuid.getMostSignificantBits(), isPrimary)
                != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
            Log.e(TAG, "addServiceSyncNative failed.");
            return false;
        }
        int serviceHandle = mPendingServiceHandle;
        mPendingServiceHandle = INVALID_HANDLE;
        mPendingService.setHandle(serviceHandle);
        for (NearlinkSsapProperty property : mPendingService.getProperties()) {
            mPendingProperty = property;
            if (addPropertySyncNative(serverId, serviceHandle, property.getUuid().getLeastSignificantBits(),
                    property.getUuid().getMostSignificantBits(), property.getPermissions(),
                    property.getOperateIndication(), property.getValue()) != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
                Log.e(TAG, "addPropertySyncNative failed.");
                return false;
            }
            int propertyHandle = mPendingPropertyHandle;
            mPendingPropertyHandle = INVALID_HANDLE;
            property.setHandle(propertyHandle);
            for (NearlinkSsapDescriptor descriptor : property.getDescriptors()) {
                if (addDescriptorSyncNative(serverId, serviceHandle, propertyHandle,
                        property.getUuid().getLeastSignificantBits(), property.getUuid().getMostSignificantBits(),
                        property.getPermissions(), property.getOperateIndication(), descriptor.getType(),
                        descriptor.getValue()) != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
                    Log.e(TAG, "addDescriptorSyncNative failed.");
                    return false;
                }
            }
            mPendingProperty = null;
        }
        return startServiceNative(serverId, serviceHandle) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean deleteAllServices(int serverId) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "deleteAllServices() - serverId=" + serverId);
        }
        return deleteAllServicesNative(serverId) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean sendResponse(int serverId, String address, int requestId, int status, byte[] value) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "sendResponse() - serverId= " + serverId + " address=" + address + " requestId="
                    + requestId + " status=" + status);
        }
        // todo: connId = getConnIdByAddress(address);
        int connId = 0;
        return sendResponseNative(serverId, connId, requestId, status, value) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean notifyOrIndicate(int serverId, String address, int handle, int type, byte[] value) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "notifyOrIndicate() - serverId=" + serverId + " address=" + address + " handle="
                    + handle + " type=" + type);
        }
        // todo: connId = getConnIdByAddress(address);
        int connId = 0;
        return notifyOrIndicateNative(serverId, connId, handle, type, value) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    boolean setInfo(int serverId, int mtuSize, int version) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (IS_DEBUG) {
            Log.d(TAG, "setInfo() - serverId=" + serverId + " mtuSize=" + mtuSize + " version=" + version);
        }
        return setInfoNative(serverId, mtuSize, version) == NearlinkErrorCode.ERRCODE_SLE_SUCCESS;
    }

    /* *************************************************************************
     * JNI回调方法
     * ************************************************************************/
    void onServerRegistered(int serverId, long uuidMsb, long uuidLsb, int status) throws RemoteException {
        UUID uuid = new UUID(uuidMsb, uuidLsb);
        if (IS_DEBUG) {
            Log.d(TAG, "onServerRegistered() - serverId=" + serverId + " uuid=" + uuid + " status=" + status);
        }
        ContextMap<INearlinkSsapServerCallback>.App app = mServerMap.getByUuid(uuid);
        if (app != null) {
            if (status == 0) {
                app.id = serverId;
                app.linkToDeath(new ServerDeathRecipient(serverId));
            }
            app.callback.onServerRegistered(serverId, status);
        } else {
            Log.e(TAG, "Can't find app: " + uuid);
        }
    }

    void onServiceAdded(int serverId, int serviceHandle, long srvUuidLsb, long srvUuidMsb) {
        ContextMap<INearlinkSsapServerCallback>.App app = mServerMap.getById(serverId);
        if (app == null) {
            Log.e(TAG, "server " + serverId + " can not found.");
            return;
        }
        UUID uuid = new UUID(srvUuidMsb, srvUuidLsb);
        if (mPendingService == null || !mPendingService.getUuid().equals(uuid)) {
            Log.e(TAG, "mPendingService(uuid=" + (mPendingService == null ? "null" : mPendingService.getUuid())
                    + ") not equals callback service(uuid=" + uuid + "), so add service failed.");
            return;
        }
        mPendingServiceHandle = serviceHandle;
    }

    void onPropertyAdded(int serverId, int propertyHandle, long propUuidLsb, long propUuidMsb) {
        ContextMap<INearlinkSsapServerCallback>.App app = mServerMap.getById(serverId);
        if (app == null) {
            Log.e(TAG, "server " + serverId + " can not found.");
            return;
        }
        UUID uuid = new UUID(propUuidMsb, propUuidLsb);
        if (mPendingProperty == null || !mPendingProperty.getUuid().equals(uuid)) {
            Log.e(TAG, "mPendingProperty(uuid=" + (mPendingProperty == null ? "null" : mPendingProperty.getUuid())
                    + ") not equals callback property(uuid=" + uuid + "), so add property failed.");
            return;
        }
        mPendingPropertyHandle = propertyHandle;
    }

    void onServiceStarted(int serverId, int serviceHandle, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onServiceStarted() - serverId=" + serverId + " serviceHandle=" + serviceHandle
                            + " status=" + status);
        }
        ContextMap<INearlinkSsapServerCallback>.App app = mServerMap.getById(serverId);
        if (app == null) {
            Log.e(TAG, "server " + serverId + " can not found.");
            return;
        }
        NearlinkSsapService service = mPendingService;
        synchronized (this) {
            mPendingService = null;
        }
        app.callback.onServiceAdded(service, status);
    }

    void onAllServiceDeleted(int serverId, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onAllServiceDeleted() - serverId=" + serverId + " status=" + status);
        }
        ContextMap<INearlinkSsapServerCallback>.App app = mServerMap.getById(serverId);
        if (app == null) {
            Log.e(TAG, "server " + serverId + " can not found.");
            return;
        }
        app.callback.onAllServiceDeleted(status);
    }

    void onServerReadRequest(int serverId, int connId, int requestId, int handle, int type,
                             boolean shouldResponse, boolean shouldAuthorize, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onServerReadRequest() - serverId=" + serverId + " connId=" + connId
                    + " requestId=" + requestId + " handle=" + handle + " type=" + type + " shouldResponse="
                    + shouldResponse + " shouldAuthorize=" + shouldAuthorize + " status=" + status);
        }
        // todo: address = getAddressByConnId(connId);
        String address = null;
        if (address == null) {
            return;
        }
        ContextMap<INearlinkSsapServerCallback>.App app = mServerMap.getById(serverId);
        if (app == null) {
            Log.e(TAG, "server " + serverId + " can not found.");
            return;
        }
        app.callback.onServerReadRequest(address, requestId, handle, type, shouldResponse, shouldAuthorize, status);
    }

    void onServerWriteRequest(int serverId, int connId, int requestId, int handle, int type, boolean shouldResponse,
                              boolean shouldAuthorize, byte[] data, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onServerWriteRequest() - serverId=" + serverId + " connId=" + connId + " requestId="
                    + requestId + " handle=" + handle + " type=" + type + " shouldResponse=" + shouldResponse
                    + " shouldAuthorize=" + shouldAuthorize + " status=" + status);
        }
        // todo: address = getAddressByConnId(connId);
        String address = null;
        if (address == null) {
            return;
        }
        ContextMap<INearlinkSsapServerCallback>.App app = mServerMap.getById(serverId);
        if (app == null) {
            Log.e(TAG, "server " + serverId + " can not found.");
            return;
        }
        app.callback.onServerWriteRequest(address, requestId, handle, type, shouldResponse, shouldAuthorize,
                data, status);
    }

    void onMtuChanged(int serverId, int connId, int mtuSize, int version, int status) throws RemoteException {
        if (IS_DEBUG) {
            Log.d(TAG, "onMtuChanged() - serverId=" + serverId + " connId=" + connId
                    + " mtuSize=" + mtuSize + " version=" + version + " status=" + status);
        }
        // todo: address = getAddressByConnId(connId);
        String address = null;
        if (address == null) {
            return;
        }
        ContextMap<INearlinkSsapServerCallback>.App app = mServerMap.getById(serverId);
        if (app == null) {
            Log.e(TAG, "server " + serverId + " can not found.");
            return;
        }
        app.callback.onMtuChanged(address, mtuSize, version, status);
    }

    /* *************************************************************************
     * JNI 方法
     * ************************************************************************/
    private static native void classInitNative();

    private native void initializeNative();

    private native void cleanupNative();

    private native int registerServerNative(long uuidLsb, long uuidMsb);

    private native int unregisterServerNative(int serverId);

    private native int addServiceSyncNative(int serverId, long srvUuidLsb, long srvUuidMsb, boolean isPrimary);

    private native int addPropertySyncNative(int serverId, int serviceHandle, long propUuidLsb, long propUuidMsb,
                                             int permission, int operateIndication, byte[] value);

    private native int addDescriptorSyncNative(int serverId, int serviceHandle, int propertyHandle, long propUuidLsb,
                                               long propUuidMsb, int permission, int operateIndication, int type,
                                               byte[] value);

    private native int startServiceNative(int serverId, int serviceHandle);

    private native int deleteAllServicesNative(int serverId);

    private native int sendResponseNative(int serverId, int connId, int requestId, int status, byte[] value);

    private native int notifyOrIndicateNative(int serverId, int connId, int handle, int type, byte[] value);

    private native int setInfoNative(int serverId, int mtuSize, int version);
}
