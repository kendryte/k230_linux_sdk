/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.ParcelUuid;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * 星闪SSAP服务端公开接口
 *
 * <p>通过该类实现星闪SSAP服务端与客户端的通信</p>
 *
 * <p>通过{@link NearlinkManager#openSsapServer}打开SSAP服务，然后等待客户端连接</p>
 *
 * @since 2023-12-01
 */
public class NearlinkSsapServer {
    private static final String TAG = "SSAP-Server-FMW";
    private static final boolean IS_DEBUG = true;

    private static final int INVALID_SERVER_ID = -1;

    private NearlinkAdapter mAdapter;
    private INearlinkSsapServer mService;
    private NearlinkSsapServerCallback mCallback;
    private Object mServerIdLock = new Object();
    private int mServerId = INVALID_SERVER_ID;
    private NearlinkSsapService mPendingService;
    private List<NearlinkSsapService> mSsapServices;

    private final INearlinkSsapServerCallback mSsapServerCallback = new INearlinkSsapServerCallback.Stub() {
        @Override
        public void onServerRegistered(int serverId, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onServerRegistered() = serverId=" + serverId + " Status=" + status);
            }
            synchronized (mServerIdLock) {
                if (mCallback != null) {
                    mServerId = serverId;
                    mServerIdLock.notifyAll();
                } else {
                    // registration timeout
                    Log.e(TAG, "onServerRegistered: mCallback is null");
                }
            }
        }

        @Override
        public void onServiceAdded(NearlinkSsapService service, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onServiceStarted() - serviceHandle=" + service.getHandle() + " status=" + status);
            }
            if (mPendingService == null) {
                return;
            }
            NearlinkSsapService tmp = mPendingService;

            synchronized (NearlinkSsapServer.class) {
                mPendingService = null;
            }

            tmp.setHandle(service.getHandle());
            List<NearlinkSsapProperty> tempProps = tmp.getProperties();
            List<NearlinkSsapProperty> svcProps = service.getProperties();
            for (int i = 0; i < svcProps.size(); i++) {
                NearlinkSsapProperty tempProp = tempProps.get(i);
                NearlinkSsapProperty svcProp = svcProps.get(i);
                tempProp.setHandle(svcProp.getHandle());
            }

            mSsapServices.add(tmp);
            try {
                mCallback.onServiceAdded(tmp, status);
            } catch (Exception ex) {
                Log.e(TAG, "Unhandled exception in callback", ex);
            }
        }

        @Override
        public void onAllServiceDeleted(int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onAllServiceDeleted() - status=" + status);
            }
            try {
                mCallback.onServicesCleared(NearlinkSsapServer.this, status);
            } catch (Exception ex) {
                Log.e(TAG, "Unhandled exception in callback", ex);
            }
        }

        @Override
        public void onServerReadRequest(String address, int requestId, int handle, int type, boolean needResponse,
                                        boolean needAuthorize, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onServerReadRequest() - address=" + address + " handle=" + handle + " type=" + type);
            }
            if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
                return;
            }
            NearlinkDevice device = mAdapter.getRemoteDevice(address);
            if (device == null) {
                return;
            }
            if (type == NearlinkSsapProperty.TYPE_VALUE) {
                NearlinkSsapProperty property = getProperty(device, handle);
                if (property == null) {
                    Log.e(TAG, "onServerReadRequest() no property for handle " + handle);
                    return;
                }
                try {
                    mCallback.onPropertyReadRequest(device, requestId, property, needResponse, needAuthorize);
                } catch (Exception ex) {
                    Log.e(TAG, "Unhandled exception in callback", ex);
                }
            } else {
                NearlinkSsapDescriptor descriptor = getDescriptorByType(device, handle, type);
                if (descriptor == null) {
                    Log.e(TAG, "onServerReadRequest() no descriptor for handle:" + handle + " and type:" + type);
                    return;
                }
                try {
                    mCallback.onDescriptorReadRequest(device, requestId, descriptor, needResponse, needAuthorize);
                } catch (Exception ex) {
                    Log.e(TAG, "Unhandled exception in callback", ex);
                }
            }
        }

        @Override
        public void onServerWriteRequest(String address, int requestId, int handle, int type, boolean needResponse,
                                         boolean needAuthorize, byte[] data, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onServerWriteRequest() - address=" + address + " handle=" + handle + " type="
                        + type + " status=" + status);
            }
            if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
                return;
            }
            NearlinkDevice device = mAdapter.getRemoteDevice(address);
            if (device == null) {
                return;
            }
            if (type == NearlinkSsapProperty.TYPE_VALUE) {
                NearlinkSsapProperty property = getProperty(device, handle);
                if (property == null) {
                    Log.e(TAG, "onServerWriteRequest() no property for handle " + handle);
                    return;
                }
                try {
                    mCallback.onPropertyWriteRequest(device, requestId, property, needResponse, needAuthorize, data);
                } catch (Exception ex) {
                    Log.e(TAG, "Unhandled exception in callback", ex);
                }
            } else {
                NearlinkSsapDescriptor descriptor = getDescriptorByType(device, handle, type);
                if (descriptor == null) {
                    Log.e(TAG, "onServerWriteRequest() no descriptor for handle:" + handle + " and type:" + type);
                    return;
                }
                try {
                    mCallback.onDescriptorWriteRequest(device, requestId, descriptor, needResponse, needAuthorize,
                            data);
                } catch (Exception ex) {
                    Log.e(TAG, "Unhandled exception in callback", ex);
                }
            }
        }

        @Override
        public void onMtuChanged(String address, int mtuSize, int version, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onMtuChanged() - address=" + address + " mtuSize=" + mtuSize + " version=" + version);
            }
            if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
                return;
            }
            NearlinkDevice device = mAdapter.getRemoteDevice(address);
            if (device == null) {
                return;
            }
            try {
                mCallback.onMtuChanged(device, mtuSize, version);
            } catch (Exception ex) {
                Log.e(TAG, "Unhandled exception in callback", ex);
            }
        }
    };

    NearlinkSsapServer(INearlinkSsapServer iServer) {
        mService = iServer;
        mAdapter = NearlinkAdapter.getDefaultAdapter();
        mCallback = null;
        mSsapServices = new ArrayList<>();
    }

    /**
     * 关闭SSAP服务端
     *
     * <p>应用程序在使用完SSAP服务端的功能之后应该尽早调用该方法以释放资源
     */
    public void close() {
        if (IS_DEBUG) {
            Log.d(TAG, "close()");
        }
        unregisterCallback();
    }

    NearlinkSsapService getService(NearlinkDevice device, int handle) {
        for (NearlinkSsapService svc : mSsapServices) {
            if (svc.getDevice().equals(device)
                    && svc.getHandle() == handle) {
                return svc;
            }
        }
        if (IS_DEBUG) {
            Log.d(TAG, "can't getService by handle=" + handle + " device=" + device.getAddress());
        }
        return null;
    }

    NearlinkSsapProperty getProperty(NearlinkDevice device, int handle) {
        for (NearlinkSsapService service : mSsapServices) {
            if (service.getHandle() >= handle) {
                continue;
            }
            for (NearlinkSsapProperty property : service.getProperties()) {
                if (property.getHandle() == handle) {
                    return property;
                }
            }
        }
        if (IS_DEBUG) {
            Log.d(TAG, "can't getProperty by handle=" + handle + " device=" + device.getAddress());
        }
        return null;
    }

    NearlinkSsapDescriptor getDescriptorByType(NearlinkDevice device, int handle, int type) {
        NearlinkSsapProperty property = getProperty(device, handle);
        if (property != null) {
            for (NearlinkSsapDescriptor desc : property.getDescriptors()) {
                if (desc.mType == type) {
                    return desc;
                }
            }
        }
        if (IS_DEBUG) {
            Log.d(TAG, "can't getDescriptorByType by handle=" + handle + " type=" + type + " device="
                    + device.getAddress());
        }
        return null;
    }

    /**
     * 获取SSAP服务端支持的服务列表
     *
     * @return SSAP服务端支持的服务列表
     */
    public List<NearlinkSsapService> getServices() {
        return mSsapServices;
    }

    boolean registerCallback(NearlinkSsapServerCallback callback) {
        if (IS_DEBUG) {
            Log.d(TAG, "registerCallback()");
        }
        if (mService == null) {
            Log.e(TAG, "SSAP Server service not available");
            return false;
        }

        synchronized (mServerIdLock) {
            if (mCallback != null) {
                Log.e(TAG, "App can register callback only once");
                return false;
            }
            mCallback = callback;

            boolean isSuccess;
            try {
                isSuccess = mService.registerServer(new ParcelUuid(UUID.randomUUID()), mSsapServerCallback);
            } catch (RemoteException e) {
                Log.e(TAG, "", e);
                mCallback = null;
                return false;
            }
            if (isSuccess) {
                try {
                    mServerIdLock.wait(10000);
                } catch (InterruptedException e) {
                    Log.e(TAG, "" + e);
                    mCallback = null;
                }
                if (mServerId == INVALID_SERVER_ID) {
                    mCallback = null;
                    return false;
                }
            }
            return isSuccess;
        }
    }

    boolean unregisterCallback() {
        if (IS_DEBUG) {
            Log.d(TAG, "unregisterCallback()");
        }
        if (mService == null) {
            Log.e(TAG, "SSAP Server service not available");
            return false;
        }
        mCallback = null;
        try {
            mService.unregisterServer(mServerId);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        mServerId = 0;
        return true;
    }

    /**
     * 向SSAP服务端添加一个服务
     *
     * <p>该方法是异步调用，执行结果将通过{@link NearlinkSsapServerCallback#onServiceAdded}方法回报</p>
     *
     * <p>需要等待上一个服务添加的回调上报后才能添加下一个服务</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param service   SSAP服务对象
     * @param isPrimary 是否是首要服务
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean addService(NearlinkSsapService service, boolean isPrimary) {
        if (IS_DEBUG) {
            Log.d(TAG, "addService() - service=" + service.getUuid());
        }
        List<NearlinkSsapProperty> properties = service.getProperties();
        if (properties == null || properties.isEmpty()) {
            Log.e(TAG, "Service at least with one property");
            return false;
        }
        for (NearlinkSsapProperty property : properties) {
            if (!property.valid()) {
                Log.e(TAG, "Property(uuid=" + property.getUuid() + ") invalid");
                return false;
            }
        }

        if (mService == null) {
            Log.e(TAG, "SSAP Server service not available");
            return false;
        }

        synchronized (NearlinkSsapServer.class) {
            if (mPendingService != null) {
                Log.e(TAG, "other service is adding, please try later.");
                return false;
            }
            mPendingService = service;
        }

        try {
            return mService.addService(mServerId, isPrimary, service);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            synchronized (NearlinkSsapServer.class) {
                mPendingService = null;
            }
            return false;
        }
    }

    /**
     * 清除所有已添加的服务
     *
     * <p>该方法是异步调用，执行结果将通过{@link NearlinkSsapServerCallback#onServicesCleared}方法回报</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean clearServices() {
        if (IS_DEBUG) {
            Log.d(TAG, "clearServices()");
        }
        if (mService == null) {
            Log.e(TAG, "SSAP Server service not available");
            return false;
        }

        try {
            mSsapServices.clear();
            return mService.deleteAllServices(mServerId);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

    /**
     * 为对端设备的读写请求发送一个响应
     *
     * <p>该方法只能在收到以下请求后调用：<br>
     * {@link NearlinkSsapServerCallback#onPropertyWriteRequest}<br>
     * {@link NearlinkSsapServerCallback#onDescriptorWriteRequest}<br>
     * {@link NearlinkSsapServerCallback#onPropertyReadRequest}<br>
     * {@link NearlinkSsapServerCallback#onDescriptorReadRequest}<br></p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param device    对端设备对象
     * @param requestId 请求ID
     * @param status    错误码
     * @param value     响应值
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean sendResponse(NearlinkDevice device, int requestId, int status, byte[] value) {
        if (IS_DEBUG) {
            Log.d(TAG,
                    "sendResponse() - device=" + device.getAddress() + " requestId=" + requestId + " status=" + status);
        }
        if (mService == null) {
            Log.e(TAG, "SSAP Server service not available");
            return false;
        }

        try {
            return mService.sendResponse(mServerId, device.getAddress(), requestId, status, value);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

    /**
     * 向SSAP客户端发送一个属性值更新的通知或指示
     *
     * <p>向指定客户端发送属性变更通知，如果客户端设置了CCCD为0则不会被通知</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param device   对端设备对象
     * @param property 属性对象
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean notifyPropertyChanged(NearlinkDevice device, NearlinkSsapProperty property) {
        if (IS_DEBUG) {
            Log.d(TAG, "notifyPropertyChanged() - device=" + device.getAddress() + " uuid=" + property.getUuid()
                    + " handle=" + property.getHandle());
        }
        if (mService == null) {
            Log.e(TAG, "SSAP Server service not available");
            return false;
        }

        NearlinkSsapService service = property.getService();
        if (service == null) {
            Log.e(TAG, "service is null.");
            return false;
        }

        if (property.getValue() == null) {
            throw new IllegalArgumentException("Property value is empty. Use NearlinkSsapProperty#setvalue to update");
        }

        try {
            return mService.notifyOrIndicate(mServerId, device.getAddress(), property.getHandle(),
                    NearlinkSsapProperty.TYPE_VALUE, property.getValue());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

    /**
     * 设置服务端基本信息
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param mtuSize MTU大小
     * @param version 版本
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean setInfo(int mtuSize, int version) {
        if (IS_DEBUG) {
            Log.d(TAG, "setInfo() - mtuSize=" + mtuSize + " version=" + version);
        }
        if (mService == null) {
            Log.e(TAG, "SSAP Server service not available");
            return false;
        }
        try {
            return mService.setInfo(mServerId, mtuSize, version);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }
}
