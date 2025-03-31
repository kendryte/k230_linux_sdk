/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Handler;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.util.Log;

import android.nearlink.NearlinkUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * 星闪SSAP客户端公开接口
 *
 * <p>通过该类实现星闪SSAP客户端与服务端的连接与通信</p>
 *
 * <p>为了连接星闪外围设备，你需要创建一个{@link NearlinkSsapClientCallback}，然后通过
 * {@link NearlinkDevice#connectSsap}来创建一个SSAP客户端实例。</p>
 *
 * @since 2023-12-01
 */
public class NearlinkSsapClient {
    private static final String TAG = "SSAP-Client-FMW";
    private static final boolean IS_DEBUG = true;
    private static final boolean IS_VDBG = false;

    /**
     * 无效的Client ID
     */
    private static final int INVALID_CLIENT_ID = -1;

    private static final int CONN_STATE_IDLE = 0;
    private static final int CONN_STATE_CONNECTING = 1;
    private static final int CONN_STATE_CONNECTED = 2;
    private static final int CONN_STATE_DISCONNECTING = 3;
    private static final int CONN_STATE_CLOSED = 4;

    private INearlinkSsapClient mService;
    private NearlinkSsapClientCallback mCallback;
    private Handler mHandler;
    private int mClientId = INVALID_CLIENT_ID;
    private NearlinkDevice mDevice;
    private int mConnState;
    private final Object mStateLock = new Object();
    private boolean mDeviceBusy = false;
    private boolean isReadingPropertyByUuid = false;
    private List<NearlinkSsapService> mSsapServices = null;

    INearlinkSsapClientCallback mNearlinkSsapClientCallback = new INearlinkSsapClientCallback.Stub() {
        @Override
        public void onRegistered(int clientId, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onClientRegistered() = clientId=" + clientId + " Status=" + status);
            }
            mClientId = clientId;
            if (status != NearlinkErrorCode.ERRCODE_SLE_SUCCESS) {
                runOrQueueCallback(new Runnable() {
                    @Override
                    public void run() {
                        final NearlinkSsapClientCallback callback = mCallback;
                        if (callback != null) {
                            callback.onConnectionStateChange(NearlinkSsapClient.this,
                                    NearlinkProfile.STATE_DISCONNECTED);
                        }
                    }
                });

                synchronized (mStateLock) {
                    mConnState = CONN_STATE_IDLE;
                }
                return;
            }

            if (IS_VDBG) {
                synchronized (mStateLock) {
                    if (mConnState != CONN_STATE_CONNECTING) {
                        Log.e(TAG, "Bad connection state: " + mConnState);
                    }
                }
            }

            try {
                mService.connect(clientId, mDevice.getAddress(), mDevice.getAddressType());
            } catch (RemoteException e) {
                Log.e(TAG, "", e);
            }
        }

        @Override
        public void onConnectionStateChanged(int clientId, String address, int addressType, int state)
                throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onClientConnectionState() - clientId=" + clientId + " state=" + state + " device="
                        + address);
            }
            if (!address.equals(mDevice.getAddress()) || addressType != mDevice.getAddressType()) {
                return;
            }
            runOrQueueCallback(new Runnable() {
                @Override
                public void run() {
                    final NearlinkSsapClientCallback callback = mCallback;
                    if (callback != null) {
                        callback.onConnectionStateChange(NearlinkSsapClient.this, state);
                    }
                }
            });

            synchronized (mStateLock) {
                if (state == NearlinkAdapter.STATE_CONNECTED) {
                    mConnState = CONN_STATE_CONNECTED;
                } else {
                    mConnState = CONN_STATE_IDLE;
                }
            }
        }

        @Override
        public void onSearchComplete(String address, List<NearlinkSsapService> services, int status)
                throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onSearchComplete() = Device=" + address + " Status=" + status);
            }
            if (!address.equals(mDevice.getAddress())) {
                return;
            }

            for (NearlinkSsapService service : services) {
                service.setDevice(mDevice);
            }

            mSsapServices.addAll(services);

            runOrQueueCallback(() -> {
                final NearlinkSsapClientCallback callback = mCallback;
                if (callback != null) {
                    callback.onServicesDiscovered(NearlinkSsapClient.this, status);
                }
            });
        }

        @Override
        public void onReadCfm(String address, int handle, int type, byte[] data, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onReadCfm() = Device=" + address + " handle=" + handle + " type=" + type + " Status="
                        + status);
            }
            if (!address.equals(mDevice.getAddress())) {
                return;
            }

            synchronized (NearlinkSsapClient.this) {
                mDeviceBusy = false;
            }

            if (type == NearlinkSsapProperty.TYPE_VALUE) {
                NearlinkSsapProperty property = getProperty(mDevice, handle);
                if (property == null) {
                    return;
                }
                property.setValue(data);
                runOrQueueCallback(() -> {
                    final NearlinkSsapClientCallback callback = mCallback;
                    if (callback != null) {
                        callback.onPropertyRead(NearlinkSsapClient.this, property, status);
                    }
                });
            } else {
                NearlinkSsapDescriptor descriptor = getDescriptorByType(mDevice, handle, type);
                if (descriptor == null) {
                    return;
                }
                descriptor.setValue(data);
                runOrQueueCallback(() -> {
                    final NearlinkSsapClientCallback callback = mCallback;
                    if (callback != null) {
                        callback.onDescriptorRead(NearlinkSsapClient.this, descriptor, status);
                    }
                });
            }
        }

        @Override
        public void onReadByUuidComplete(String address, ParcelUuid uuid, List<NearlinkSsapReadByUuid> data,
                                         int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onReadByUuidComplete() = Device=" + address + " uuid=" + uuid.getUuid()
                        + " Status=" + status);
            }
            if (!address.equals(mDevice.getAddress())) {
                return;
            }
            synchronized (NearlinkSsapClient.this) {
                mDeviceBusy = false;
            }

            if (isReadingPropertyByUuid) {
                List<NearlinkSsapProperty> properties = new ArrayList<>();
                if (data == null || data.isEmpty()) {
                    return;
                }
                for (NearlinkSsapReadByUuid item : data) {
                    if (item.type != NearlinkSsapProperty.TYPE_VALUE) {
                        continue;
                    }
                    NearlinkSsapProperty property = getProperty(mDevice, item.getHandle());
                    if (property != null) {
                        property.setValue(item.getData());
                        properties.add(property);
                    }
                }
                runOrQueueCallback(() -> {
                    final NearlinkSsapClientCallback callback = mCallback;
                    if (callback != null) {
                        callback.onPropertyReadByUuid(NearlinkSsapClient.this, properties, status);
                    }
                });
            } else {
                List<NearlinkSsapDescriptor> descriptors = new ArrayList<>();
                if (data == null || data.isEmpty()) {
                    return;
                }
                for (NearlinkSsapReadByUuid item : data) {
                    if (item.type == NearlinkSsapProperty.TYPE_VALUE) {
                        continue;
                    }
                    NearlinkSsapDescriptor descriptor = getDescriptorByType(mDevice, item.getHandle(),
                            item.getType());
                    if (descriptor != null) {
                        descriptor.setValue(item.getData());
                        descriptors.add(descriptor);
                    }
                }
                runOrQueueCallback(() -> {
                    final NearlinkSsapClientCallback callback = mCallback;
                    if (callback != null) {
                        callback.onDescriptorReadByUuid(NearlinkSsapClient.this, descriptors, status);
                    }
                });
            }
        }

        @Override
        public void onWriteCfm(String address, int handle, int type, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onWriteCfm() = Device=" + address + " handle=" + handle + " type=" + type + " Status="
                        + status);
            }
            if (!address.equals(mDevice.getAddress())) {
                return;
            }
            synchronized (NearlinkSsapClient.this) {
                mDeviceBusy = false;
            }

            if (type == NearlinkSsapProperty.TYPE_VALUE) {
                NearlinkSsapProperty property = getProperty(mDevice, handle);
                if (property == null) {
                    return;
                }
                runOrQueueCallback(() -> {
                    final NearlinkSsapClientCallback callback = mCallback;
                    if (callback != null) {
                        callback.onPropertyWrite(NearlinkSsapClient.this, property, status);
                    }
                });
            } else {
                NearlinkSsapDescriptor descriptor = getDescriptorByType(mDevice, handle, type);
                if (descriptor == null) {
                    return;
                }
                runOrQueueCallback(() -> {
                    final NearlinkSsapClientCallback callback = mCallback;
                    if (callback != null) {
                        callback.onDescriptorWrite(NearlinkSsapClient.this, descriptor, status);
                    }
                });
            }
        }

        @Override
        public void onInfoExchanged(String address, int mtuSize, int version, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onInfoExchanged() = Device=" + address + " mtuSize=" + mtuSize + " version=" + version
                        + " Status=" + status);
            }
            if (!address.equals(mDevice.getAddress())) {
                return;
            }
            synchronized (NearlinkSsapClient.this) {
                mDeviceBusy = false;
            }
            runOrQueueCallback(() -> {
                final NearlinkSsapClientCallback callback = mCallback;
                if (callback != null) {
                    callback.onExchangeInfo(NearlinkSsapClient.this, mtuSize, version, status);
                }
            });
        }

        @Override
        public void onNotification(String address, int handle, int type, boolean isNotify, byte[] data, int status)
                throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onNotification() = Device=" + address + " handle=" + handle + " type=" + type
                        + " isNotify=" + isNotify + " Status=" + status);
            }
            if (!address.equals(mDevice.getAddress())) {
                return;
            }

            if (type == NearlinkSsapProperty.TYPE_VALUE) {
                NearlinkSsapProperty property = getProperty(mDevice, handle);
                if (property == null) {
                    return;
                }
                property.setValue(data);

                runOrQueueCallback(() -> {
                    final NearlinkSsapClientCallback callback = mCallback;
                    if (callback != null) {
                        callback.onPropertyChanged(NearlinkSsapClient.this, property);
                    }
                });
            }
        }

        @Override
        public void onReadRemoteRssi(String address, int rssi, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onReadRemoteRssi() = Device=" + address + " rssi=" + rssi + " Status=" + status);
            }
            if (!address.equals(mDevice.getAddress())) {
                return;
            }
            runOrQueueCallback(() -> {
                final NearlinkSsapClientCallback callback = mCallback;
                if (callback != null) {
                    callback.onReadRemoteRssi(NearlinkSsapClient.this, rssi, status);
                }
            });
        }

        @Override
        public void onConnectionParameterUpdated(String address, int interval, int latency, int timeout, int status) throws RemoteException {
            if (IS_DEBUG) {
                Log.d(TAG, "onConnectionParameterUpdated() = Device=" + address + " interval=" + interval
                        + " latency=" + latency + " timeout=" + timeout + " Status=" + status);
            }
            if (!address.equals(mDevice.getAddress())) {
                return;
            }
            runOrQueueCallback(() -> {
                final NearlinkSsapClientCallback callback = mCallback;
                if (callback != null) {
                    callback.onConnectionParameterUpdated(NearlinkSsapClient.this, interval, latency, timeout, status);
                }
            });
        }
    };

    NearlinkSsapClient(INearlinkSsapClient iSsapClient, NearlinkDevice device) {
        mService = iSsapClient;
        mDevice = device;
        mSsapServices = new ArrayList<>();
        mConnState = CONN_STATE_IDLE;
    }

    /**
     * 关闭SSAP客户端
     *
     * <p>应用程序在使用完SSAP客户端的功能之后应该尽早调用该方法以释放资源
     */
    public void close() {
        if (IS_DEBUG) {
            Log.d(TAG, "close()");
        }
        unregisterClient();
        mConnState = CONN_STATE_CLOSED;
    }

    NearlinkSsapService getService(NearlinkDevice device, int handle) {
        for (NearlinkSsapService svc : mSsapServices) {
            if (svc.getDevice().equals(device) && svc.getHandle() == handle) {
                return svc;
            }
        }
        if (IS_DEBUG) {
            Log.d(TAG, "can't getService by handle=" + handle + " device=" + device.getAddress());
        }
        return null;
    }

    NearlinkSsapProperty getProperty(NearlinkDevice device, int handle) {
        for (NearlinkSsapService svc : mSsapServices) {
            if (svc.getHandle() >= handle || !svc.getDevice().equals(device)) {
                continue;
            }
            for (NearlinkSsapProperty property : svc.getProperties()) {
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
        return getDescriptorByType(property, type);
    }

    NearlinkSsapDescriptor getDescriptorByType(NearlinkSsapProperty property, int type) {
        if (property != null) {
            for (NearlinkSsapDescriptor desc : property.getDescriptors()) {
                if (desc.mType == type) {
                    return desc;
                }
            }
        }
        if (IS_DEBUG) {
            Log.d(TAG, "can't getDescriptorByType by handle=" + property.getHandle() + " type=" + type);
        }
        return null;
    }

    private void runOrQueueCallback(final Runnable cb) {
        if (mHandler == null) {
            try {
                cb.run();
            } catch (Exception ex) {
                Log.w(TAG, "Unhandled exception in callback", ex);
            }
        } else {
            mHandler.post(cb);
        }
    }

    private boolean registerClient(NearlinkSsapClientCallback callback, Handler handler) {
        if (IS_DEBUG) {
            Log.d(TAG, "registerApp()");
        }
        if (mService == null) {
            return false;
        }

        mCallback = callback;
        mHandler = handler;
        try {
            return mService.registerClient(new ParcelUuid(UUID.randomUUID()), mDevice.getAddress(),
                    mNearlinkSsapClientCallback);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

    private void unregisterClient() {
        if (IS_DEBUG) {
            Log.d(TAG, "unregister()");
        }
        try {
            mCallback = null;
            mService.unregisterClient(mClientId);
            mClientId = INVALID_CLIENT_ID;
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
    }

    boolean connect(NearlinkSsapClientCallback callback, Handler handler) {
        if (IS_DEBUG) {
            Log.d(TAG, "connect() - device: " + mDevice.getAddress());
        }
        synchronized (mStateLock) {
            if (mConnState != CONN_STATE_IDLE) {
                throw new IllegalStateException("Not idle");
            }
            mConnState = CONN_STATE_CONNECTING;
        }
        if (!registerClient(callback, handler)) {
            synchronized (mStateLock) {
                mConnState = CONN_STATE_IDLE;
            }
            Log.e(TAG, "Failed to register callback");
            return false;
        }
        return true;
    }

    /**
     * 断开与对端设备的连接
     */
    public void disconnect() {
        if (IS_DEBUG) {
            Log.d(TAG, "disconnect() - device: " + mDevice.getAddress());
        }
        if (mService == null) {
            Log.e(TAG, "SSAP Client service not available");
            return;
        }

        synchronized (mStateLock) {
            mConnState = CONN_STATE_DISCONNECTING;
        }
        try {
            mService.disconnect(mClientId, mDevice.getAddress(), mDevice.getAddressType());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
    }

    /**
     * 连接到对端设备
     *
     * <p>向协议栈下达一个连接指令，连接状态将通过回调接口
     * {@link NearlinkSsapClientCallback#onConnectionStateChange(NearlinkSsapClient, int)}来呈现。</p>
     *
     * <p>该方法仅用于重连已断开的连接</p>
     *
     * @return 连接指令下达成功则返回true，否则返回false
     */
    public boolean connect() {
        try {
            return mService.connect(mClientId, mDevice.getAddress(), mDevice.getAddressType());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

    /**
     * 发现对端设备提供的服务、属性和描述符
     *
     * <p>该方法是异步调用，一旦服务发现完毕，将通过
     * {@link NearlinkSsapClientCallback#onServicesDiscovered(NearlinkSsapClient, int)}
     * 上报结果</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @return 如果服务发现流程成功启动则返回true，否则返回false
     */
    public boolean discoverServices() {
        if (IS_DEBUG) {
            Log.d(TAG, "discoverServices() - device=" + mDevice.getAddress());
        }
        if (mService == null) {
            Log.e(TAG, "SSAP Client service not available");
            return false;
        }

        mSsapServices.clear();

        try {
            return mService.discoverServices(mClientId, mDevice.getAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
    }

    /**
     * 返回对端设备提供的服务
     *
     * <p>该方法需要在服务发现完成后调用</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @return 对端设备的服务列表，如果尚未进行服务发现则返回空列表
     */
    public List<NearlinkSsapService> getServices() {
        List<NearlinkSsapService> result = new ArrayList<NearlinkSsapService>();

        for (NearlinkSsapService service : mSsapServices) {
            if (service.getDevice().equals(mDevice)) {
                result.add(service);
            }
        }
        return result;
    }

    /**
     * 从已发现的服务列表中查找UUID相等的{@link NearlinkSsapService}
     *
     * <p>该方法需要在服务发现完成后调用</p>
     *
     * <p>如果已发现的服务中存在多个UUID相等的服务，则返回第一个匹配的</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param uuid 服务的UUID
     * @return 如果对端设备支持该UUID的服务，则返回NearlinkSsapService对象，否则返回null
     */
    public NearlinkSsapService getService(UUID uuid) {
        for (NearlinkSsapService service : mSsapServices) {
            if (service.getDevice().equals(mDevice) && service.getUuid().equals(uuid)) {
                return service;
            }
        }
        return null;
    }

    private boolean checkServiceAndLockDevice(NearlinkSsapService service) {
        if (service == null) {
            Log.e(TAG, "service is null.");
            return false;
        }

        NearlinkDevice device = service.getDevice();
        if (device == null) {
            return false;
        }
        return checkServiceAndLockDevice();
    }

    private boolean checkServiceAndLockDevice() {
        if (mService == null) {
            Log.e(TAG, "SSAP Client service not available");
            return false;
        }
        synchronized (NearlinkSsapClient.this) {
            if (mDeviceBusy) {
                Log.e(TAG, ">>>>>device busy<<<<<");
                return false;
            }
            mDeviceBusy = true;
        }
        return true;
    }

    private boolean doRead(NearlinkSsapService service, int handle, int type) {
        try {
            boolean isSuccess = mService.readRequest(mClientId, service.getDevice().getAddress(), handle, type);
            if (!isSuccess) {
                synchronized (NearlinkSsapClient.this) {
                    mDeviceBusy = false;
                }
            }
            return isSuccess;
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            synchronized (NearlinkSsapClient.this) {
                mDeviceBusy = false;
            }
            return false;
        }
    }

    private boolean doReadByUuid(int startHandle, int endHandle, UUID uuid, int type) {
        try {
            boolean isSuccess = mService.readRequestByUuid(mClientId, mDevice.getAddress(),
                    type, startHandle, endHandle, new ParcelUuid(uuid));
            if (!isSuccess) {
                synchronized (NearlinkSsapClient.this) {
                    mDeviceBusy = false;
                    isReadingPropertyByUuid = false;
                }
            }
            return isSuccess;
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            mDeviceBusy = false;
            isReadingPropertyByUuid = false;
            return false;
        }
    }

    private boolean isReadable(int opInd) {
        if ((opInd & NearlinkSsapProperty.PROPERTY_READ) == 0) {
            if (IS_DEBUG) {
                Log.d(TAG, ">>>>>no read indication<<<<<");
            }
            return false;
        }
        return true;
    }

    /**
     * 从对端设备读取属性值
     *
     * <p>该方法是异步调用，读取结果将通过{@link NearlinkSsapClientCallback#onPropertyRead}方法上报</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param property 需要从对端设备读取其值的属性对象
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean readProperty(NearlinkSsapProperty property) {
        if (IS_DEBUG) {
            Log.d(TAG, "readProperty() - uuid=" + property.getUuid() + " handle=" + property.getHandle());
        }
        if (!isReadable(property.getOperateIndication())) {
            return false;
        }
        if (!checkServiceAndLockDevice(property.getService())) {
            return false;
        }
        return doRead(property.getService(), property.getHandle(), NearlinkSsapProperty.TYPE_VALUE);
    }

    /**
     * 从对端设备通过UUID读取属性值
     *
     * <p>该方法是异步调用，读取结果将通过{@link NearlinkSsapClientCallback#onPropertyReadByUuid}方法上报</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param uuid        要从对端设备读取的属性UUID
     * @param startHandle 属性句柄范围起始值
     * @param endHandle   属性句柄范围结束值
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean readPropertyByUuid(UUID uuid, int startHandle, int endHandle) {
        if (IS_DEBUG) {
            Log.d(TAG, "readPropertyByUuid() - uuid=" + uuid + " startHandle=" + startHandle + " endHandle="
                    + endHandle);
        }
        if (!checkServiceAndLockDevice()) {
            return false;
        }
        isReadingPropertyByUuid = true;
        return doReadByUuid(startHandle, endHandle, uuid, NearlinkSsapProperty.TYPE_VALUE);
    }

    /**
     * 从对端设备读取描述符值
     *
     * <p>该方法是异步调用，读取结果将通过{@link NearlinkSsapClientCallback#onDescriptorRead}方法上报</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param descriptor 需要从对端设备读取其值的描述符对象
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean readDescriptor(NearlinkSsapDescriptor descriptor) {
        NearlinkSsapProperty property = descriptor.getProperty();
        if (property == null) {
            if (IS_DEBUG) {
                Log.d(TAG, "readDescriptor() - property is null");
            }
            return false;
        }
        if (IS_DEBUG) {
            Log.d(TAG, "readDescriptor() - property uuid=" + property.getUuid() + " handle="
                    + property.getHandle());
        }
        if (!isReadable(property.getOperateIndication())) {
            return false;
        }
        if (!checkServiceAndLockDevice(property.getService())) {
            return false;
        }
        return doRead(property.getService(), property.getHandle(), descriptor.getType());
    }

    /**
     * 从对端设备通过UUID读取描述符值
     *
     * <p>该方法是异步调用，读取结果将通过{@link NearlinkSsapClientCallback#onDescriptorReadByUuid}方法上报</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param uuid        要从对端设备读取的属性UUID
     * @param startHandle 属性句柄范围起始值
     * @param endHandle   属性句柄范围结束值
     * @param type        描述符类型
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean readDescriptorByUuid(UUID uuid, int startHandle, int endHandle, int type) {
        if (IS_DEBUG) {
            Log.d(TAG, "readDescriptorByUuid() - uuid=" + uuid + " startHandle=" + startHandle + " endHandle="
                    + endHandle + " type=" + type);
        }
        if (!checkServiceAndLockDevice()) {
            return false;
        }
        isReadingPropertyByUuid = false;
        return doReadByUuid(startHandle, endHandle, uuid, type);
    }

    private boolean isWriteShouldResponse(int opInd) {
        return (opInd & NearlinkSsapProperty.PROPERTY_WRITE) == NearlinkSsapProperty.PROPERTY_WRITE;
    }

    private boolean isWriteNoResponse(int opInd) {
        return (opInd & NearlinkSsapProperty.PROPERTY_WRITE_NO_RSP) == NearlinkSsapProperty.PROPERTY_WRITE_NO_RSP;
    }

    private boolean isWritable(int opInd) {
        if (!isWriteShouldResponse(opInd) && !isWriteNoResponse(opInd)) {
            if (IS_DEBUG) {
                Log.d(TAG, ">>>>>no write indication<<<<<");
            }
            return false;
        }
        return true;
    }

    private boolean doWrite(NearlinkSsapService service, int handle, int type, boolean shouldResponse, byte[] data) {
        if (data == null) {
            if (IS_DEBUG) {
                Log.d(TAG, ">>>>>no value to write<<<<<");
            }
            return false;
        }

        if (!checkServiceAndLockDevice(service)) {
            return false;
        }

        try {
            boolean isSuccess = mService.writeRequest(mClientId, mDevice.getAddress(), handle, type, shouldResponse,
                    data);
            if (!isSuccess) {
                synchronized (NearlinkSsapClient.this) {
                    mDeviceBusy = false;
                }
            }
            return isSuccess;
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            mDeviceBusy = false;
            return false;
        } finally {
            if (!shouldResponse) {
                mDeviceBusy = false;
                if (type == NearlinkSsapProperty.TYPE_VALUE) {
                    mCallback.onPropertyWrite(NearlinkSsapClient.this, getProperty(mDevice, handle), 0);
                } else {
                    mCallback.onDescriptorWrite(NearlinkSsapClient.this, getDescriptorByType(mDevice, handle, type), 0);
                }
            }
        }
    }

    /**
     * 向对端设备的属性写入值
     *
     * <p>该方法是异步调用，写入结果将通过{@link NearlinkSsapClientCallback#onPropertyWrite}方法上报</p>
     *
     * <p>但需要注意的是，如果属性的操作指示位是{@link NearlinkSsapProperty#PROPERTY_WRITE_NO_RSP}，
     * 则无需等待回调方法</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param property 需要往对端设备写入其值的属性对象
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean writeProperty(NearlinkSsapProperty property) {
        if (IS_DEBUG) {
            Log.d(TAG, "writeProperty() - uuid=" + property.getUuid() + " handle=" + property.getHandle());
        }

        int opInd = property.getOperateIndication();
        if (!isWritable(opInd)) {
            return false;
        }
        return doWrite(property.getService(), property.getHandle(), NearlinkSsapProperty.TYPE_VALUE,
                isWriteShouldResponse(opInd), property.getValue());
    }

    /**
     * 向对端设备的描述符写入值
     *
     * <p>该方法是异步调用，写入结果将通过{@link NearlinkSsapClientCallback#onDescriptorWrite}方法上报</p>
     *
     * <p>但需要注意的是，如果描述符所属的属性操作指示位是
     * {@link NearlinkSsapProperty#PROPERTY_WRITE_NO_RSP}，则无需等待回调方法</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param descriptor 需要往对端设备写入其值的描述符对象
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean writeDescriptor(NearlinkSsapDescriptor descriptor) {
        NearlinkSsapProperty property = descriptor.getProperty();
        if (property == null) {
            if (IS_DEBUG) {
                Log.d(TAG, "writeDescriptor() - property is null");
            }
            return false;
        }
        if (IS_DEBUG) {
            Log.d(TAG, "writeDescriptor() - property uuid=" + property.getUuid() + " handle="
                    + property.getHandle() + " type=" + descriptor.getType());
        }
        int opInd = property.getOperateIndication();
        if (!isWritable(opInd)) {
            if (IS_DEBUG) {
                Log.d(TAG, ">>>>>no write indication<<<<<");
            }
            return false;
        }
        return doWrite(property.getService(), property.getHandle(), descriptor.getType(), isWriteShouldResponse(opInd),
                property.getValue());
    }

    /**
     * 开启或关闭属性的通知/指示功能
     *
     * <p>该方法是异步调用，一旦开启了属性的通知/指示功能，当对端设备的属性值发生变更时，本端将会收到
     * {@link NearlinkSsapClientCallback#onPropertyChanged}方法的上报</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param property           需要设置通知/指示开关的属性
     * @param shouldNotification true表示需要通知/指示，false表示不需要
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean setPropertyNotification(NearlinkSsapProperty property, boolean shouldNotification) {
        NearlinkSsapDescriptor descriptor = getDescriptorByType(property,
                NearlinkSsapDescriptor.TYPE_CLIENT_CONFIGURATION);
        if (descriptor == null) {
            Log.e(TAG, "property don't support change CCCD, no descriptor type: "
                    + NearlinkSsapDescriptor.TYPE_CLIENT_CONFIGURATION);
            return false;
        }
        descriptor.setValue(new byte[]{(byte) (shouldNotification ? 1 : 0)});
        return writeDescriptor(descriptor);
    }

    /**
     * 与对端设备交换信息
     *
     * <p>用于设置传输过程中的MTU值和版本信息，MTU的取值范围是[251,520]</p>
     *
     * <p>该方法是异步调用，执行结果将通过{@link NearlinkSsapClientCallback#onExchangeInfo}方法上报</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @param mtu     MTU值
     * @param version 版本
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean exchangeInfo(int mtu, int version) {
        if (IS_DEBUG) {
            Log.d(TAG, "exchangeInfo() - device=" + mDevice.getAddress() + " mtu=" + mtu);
        }
        if (!checkServiceAndLockDevice()) {
            return false;
        }
        try {
            boolean isSuccess = mService.exchangeInfoRequest(mClientId, mDevice.getAddress(), mtu, version);
            if (!isSuccess) {
                synchronized (NearlinkSsapClient.this) {
                    mDeviceBusy = false;
                }
            }
            return isSuccess;
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            mDeviceBusy = false;
            return false;
        }
    }

    /**
     * 读取对端设备的RSSI
     *
     * <p>方法是异步调用，执行结果将通过{@link NearlinkSsapClientCallback#onReadRemoteRssi}方法上报</p>
     *
     * <p>需要{@link android.Manifest.permission#NEARLINK}权限</p>
     *
     * @return 如果指令下达成功则返回true，否则返回false
     * */
    public boolean readRemoteRssi() {
        if (IS_DEBUG) {
            Log.d(TAG, "readRemoteRssi() - device: " + mDevice.getAddress());
        }
        if (mService == null || mClientId == INVALID_CLIENT_ID) {
            Log.e(TAG, "SSAP Client service not available");
            return false;
        }

        try {
            mService.readRemoteRssi(mClientId, mDevice.getAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
        return true;
    }

    /**
     * 更新连接参数
     *
     * <p>发送一个更新连接参数的请求到对端设备</p>
     *
     * @param minConnectionInterval 最小间隔
     * @param maxConnectionInterval 最大间隔
     * @param slaveLatency          时延值
     * @param supervisionTimeout    超时值
     *
     * @return 如果指令下达成功则返回true，否则返回false
     * @hide
     */
    public boolean updateConnectionParameter(int minConnectionInterval, int maxConnectionInterval,
                                             int slaveLatency, int supervisionTimeout) {
        if (IS_DEBUG) {
            Log.d(TAG, "updateConnectionParameter() - min=(" + minConnectionInterval
                    + ")" + (1.25 * minConnectionInterval)
                    + "msec, max=(" + maxConnectionInterval + ")"
                    + (1.25 * maxConnectionInterval) + "msec, latency=" + slaveLatency
                    + ", timeout=" + supervisionTimeout + "msec");
        }
        if (mService == null || mClientId == INVALID_CLIENT_ID) {
            Log.e(TAG, "SSAP Client service not available");
            return false;
        }

        try {
            mService.connectionParameterUpdate(mClientId, mDevice.getAddress(), minConnectionInterval,
                    maxConnectionInterval, slaveLatency, supervisionTimeout);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
        return true;
    }

    /**
     * 设置连接链路上的最大传输payload字节数
     * 
     * @param txOctets 连接链路上所偏好的最大传输payload字节数
     * @return 如果指令下达成功则返回true，否则返回false
     */
    public boolean setTxDataLength(int txOctets) {
        if (IS_DEBUG) {
            Log.d(TAG, "setTxDataLength() - device: " + NearlinkUtils.anonymizeAddr(mDevice.getAddress()) +
                    ", txDataLen: " + txOctets);
        }
        if (mService == null || mClientId == INVALID_CLIENT_ID) {
            Log.e(TAG, "SSAP Client service not available");
            return false;
        }
        try {
            mService.setTxDataLength(mClientId, txOctets);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return false;
        }
        return true;
    }
}
