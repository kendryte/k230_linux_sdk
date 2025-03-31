/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import android.app.Service;
import android.content.Intent;
import android.nearlink.INearlink;
import android.nearlink.INearlinkCallback;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAppearance;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.android.nearlink.connection.ConnectionManager;
import com.android.nearlink.connection.ConnectionService;
import com.android.nearlink.connection.PubTools;
import com.android.nearlink.discovery.DiscoveryService;
import com.android.nearlink.ssap.SsapClientService;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;

/**
 * 主入口服务
 *
 * @since 2023-12-04
 */
public class AdapterService extends Service {
    private static final String TAG = "AdapterService";
    private static final boolean DBG = true;
    private static final boolean VERBOSE = false;
    private static final int MIN_ADVT_INSTANCES_FOR_MA = 5;
    private static final int MIN_OFFLOADED_FILTERS = 10;
    private static final int MIN_OFFLOADED_SCAN_STORAGE_BYTES = 1024;

    private final Object mEnergyInfoLock = new Object();
    private int mStackReportedState;
    private long mTxTimeTotalMs;
    private long mRxTimeTotalMs;
    private long mIdleTimeTotalMs;
    private long mEnergyUsedTotalVoltAmpSecMicro;

    private final ArrayList<ProfileService> mRegisteredProfiles = new ArrayList<>();
    private final ArrayList<ProfileService> mRunningProfiles = new ArrayList<>();

    private final int SUCCESS_FLAG = 0;

    private volatile boolean mCleaningUp;
    private volatile boolean mQuietmode = false;
    private volatile boolean mNativeAvailable;

    private NearlinkBinder mNearlinkBinder;
    private NearlinkState nearlinkState;
    private AdapterProperties mAdapterProperties;
    private RemoteDevices mRemoteDevices;
    private String cachedLocalName = "";


    public static final String EXTRA_ACTION = "action";
    public static final String ACTION_SERVICE_STATE_CHANGED =
            "com.android.nearlink.discovery.action.STATE_CHANGED";

    //Only NearlinkManagerService should be registered
    private RemoteCallbackList<INearlinkCallback> mCallbacks;

    private static AdapterService sAdapterService;

    public static final String NEARLINK_ADMIN_PERM = android.Manifest.permission.NEARLINK_ADMIN;
    public static final String NEARLINK_PRIVILEGED = android.Manifest.permission.NEARLINK_PRIVILEGED;
    public static final String NEARLINK_PERM = android.Manifest.permission.NEARLINK;

    static {
        classInitNative();
    }

    public static synchronized AdapterService getAdapterService() {
        Log.d(TAG, "getAdapterService() - returning " + sAdapterService);
        return sAdapterService;
    }

    private static synchronized void setAdapterService(AdapterService instance) {
        Log.d(TAG, "setAdapterService() - trying to set service to " + instance);
        if (instance == null) {
            Log.d(TAG, "setAdapterService() - instance is null ");
            return;
        }
        sAdapterService = instance;
    }

    private static synchronized void clearAdapterService(AdapterService current) {
        if (sAdapterService == current) {
            sAdapterService = null;
        }
    }

    public RemoteDevices getRemoteDevices() {
        return mRemoteDevices;
    }

    /**
     * Register a {@link ProfileService} with AdapterService.
     *
     * @param profile the service being added.
     */
    public void addProfile(ProfileService profile) {
        mHandler.obtainMessage(MESSAGE_PROFILE_SERVICE_REGISTERED, profile).sendToTarget();
    }

    /**
     * Unregister a ProfileService with AdapterService.
     *
     * @param profile the service being removed.
     */
    public void removeProfile(ProfileService profile) {
        mHandler.obtainMessage(MESSAGE_PROFILE_SERVICE_UNREGISTERED, profile).sendToTarget();
    }

    /**
     * Notify AdapterService that a ProfileService has started or stopped.
     *
     * @param profile the service being removed.
     * @param state   {@link NearlinkAdapter#STATE_ON} or {@link NearlinkAdapter#STATE_OFF}
     */
    public void onProfileServiceStateChanged(ProfileService profile, int state) {
        if (state != NearlinkAdapter.STATE_ON && state != NearlinkAdapter.STATE_OFF) {
            throw new IllegalArgumentException(NearlinkAdapter.nameForState(state));
        }
        Message m = mHandler.obtainMessage(MESSAGE_PROFILE_SERVICE_STATE_CHANGED);
        m.obj = profile;
        m.arg1 = state;
        mHandler.sendMessage(m);
    }

    private static final int MESSAGE_PROFILE_SERVICE_STATE_CHANGED = 1;
    private static final int MESSAGE_PROFILE_SERVICE_REGISTERED = 2;
    private static final int MESSAGE_PROFILE_SERVICE_UNREGISTERED = 3;

    class AdapterServiceHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            debugLog("handleMessage() - Message: " + msg.what);

            switch (msg.what) {
                case MESSAGE_PROFILE_SERVICE_STATE_CHANGED:
                    debugLog("handleMessage() - MESSAGE_PROFILE_SERVICE_STATE_CHANGED");
                    processProfileServiceStateChanged((ProfileService) msg.obj, msg.arg1);
                    break;
                case MESSAGE_PROFILE_SERVICE_REGISTERED:
                    debugLog("handleMessage() - MESSAGE_PROFILE_SERVICE_REGISTERED");
                    registerProfileService((ProfileService) msg.obj);
                    break;
                case MESSAGE_PROFILE_SERVICE_UNREGISTERED:
                    debugLog("handleMessage() - MESSAGE_PROFILE_SERVICE_UNREGISTERED");
                    unregisterProfileService((ProfileService) msg.obj);
                    break;
            }
        }

        private void registerProfileService(ProfileService profile) {
            if (mRegisteredProfiles.contains(profile)) {
                Log.e(TAG, profile.getName() + " already registered.");
                return;
            }
            mRegisteredProfiles.add(profile);
        }

        private void unregisterProfileService(ProfileService profile) {
            if (!mRegisteredProfiles.contains(profile)) {
                Log.e(TAG, profile.getName() + " not registered (UNREGISTERED).");
                return;
            }
            mRegisteredProfiles.remove(profile);
        }

        private void processProfileServiceStateChanged(ProfileService profile, int state) {
            switch (state) {
                case NearlinkAdapter.STATE_ON:
                    if (!mRegisteredProfiles.contains(profile)) {
                        Log.e(TAG, profile.getName() + " not registered (STATE_ON).");
                        return;
                    }
                    if (mRunningProfiles.contains(profile)) {
                        Log.e(TAG, profile.getName() + " already running.");
                        return;
                    }
                    mRunningProfiles.add(profile);
                    if (mRegisteredProfiles.size() == Config.getSupportedProfiles().length
                            && mRegisteredProfiles.size() == mRunningProfiles.size()) {
                        mAdapterProperties.onNearlinkReady();
                    }
                    break;
                case NearlinkAdapter.STATE_OFF:
                    if (!mRegisteredProfiles.contains(profile)) {
                        Log.e(TAG, profile.getName() + " not registered (STATE_OFF).");
                        return;
                    }
                    if (!mRunningProfiles.contains(profile)) {
                        Log.e(TAG, profile.getName() + " not running.");
                        return;
                    }
                    mRunningProfiles.remove(profile);
                    if (mRunningProfiles.isEmpty()) {
                        Log.d(TAG, "All profile stop, disableNative()...");
                        disableNative();
                    }
                    break;
                default:
                    Log.e(TAG, "Unhandled profile state: " + state);
            }
        }
    }

    private final AdapterServiceHandler mHandler = new AdapterServiceHandler();

    @Override
    public void onCreate() {
        super.onCreate();
        debugLog("====onCreate====");
        // 首次把mCleaningUp清理
        mCleaningUp = false;
        mNearlinkBinder = new NearlinkBinder(this);
        mCallbacks = new RemoteCallbackList<INearlinkCallback>();
        mAdapterProperties = new AdapterProperties(this);
        nearlinkState = NearlinkState.make(this);
        mRemoteDevices = new RemoteDevices(this, Looper.getMainLooper());

        initNative();
        mNativeAvailable = true;
        setAdapterService(this);
        ConnectionManager.getInstance().restoreConnectedDeviceProfiles();
    }

    @Override
    public IBinder onBind(Intent intent) {
        debugLog("====onBind====");
        return mNearlinkBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        debugLog("onUnbind() - calling cleanup");
        cleanup();
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        debugLog("onDestroy()");
        if (!isMock()) {
            Log.e(TAG, "Force exit to cleanup internal state in Nearlink stack");
            System.exit(0);
        }
    }

    public boolean enable() {
        Log.e(TAG, "===service enable===");
        return enable(false);
    }

    public synchronized boolean enable(boolean quietMode) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK ADMIN permission");
        int state = mAdapterProperties.getState();
        if (state != NearlinkAdapter.STATE_OFF) {
            Log.e(TAG, "NL cannot enable because state is " + state + ", please try later");
            return false;
        }
        debugLog("enable() - Enable called with quiet mode status =  " + quietMode);
        mQuietmode = quietMode;
        debugLog("service enable() - nearlinkState.sendMessage(NearlinkState.NEARLINK_TURN_ON)");
        nearlinkState.sendMessage(NearlinkState.NEARLINK_TURN_ON);
        Log.e(TAG, "sendMessage(NearlinkState.NEARLINK_TURN_ON) finish");
        return true;
    }

    public boolean disable() {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK ADMIN permission");
        int state = mAdapterProperties.getState();
        if (state != NearlinkAdapter.STATE_ON) {
            Log.e(TAG, "NL cannot disable because state is " + state + ", please try later");
            return false;
        }
        debugLog("service disable() - nearlinkState.sendMessage(NearlinkState.NEARLINK_TURN_OFF)");
        nearlinkState.sendMessage(NearlinkState.NEARLINK_TURN_OFF);
        return true;
    }

    public boolean isEnabled() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        return mAdapterProperties.getState() == NearlinkAdapter.STATE_ON;
    }

    private boolean isAvailable() {
        return !mCleaningUp;
    }

    void registerCallback(INearlinkCallback cb) {
        mCallbacks.register(cb);
    }

    void unregisterCallback(INearlinkCallback cb) {
        mCallbacks.unregister(cb);
    }


    public String getName() {

        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        String diskName = null;

        /**
         * 如果缓存中存在name，返回name
         */
        if (!DdUtils.isBlank(cachedLocalName)) {
            return cachedLocalName;
        }

        synchronized (cachedLocalName) {
            if (!DdUtils.isBlank(cachedLocalName)) {
                return cachedLocalName;
            }
            diskName = DiskUtils.readNameOfLocalDeviceFromDisk();
        }

        if (!DdUtils.isBlank(diskName)) {
            cachedLocalName = diskName;
            return cachedLocalName;
        }

        Log.e(TAG, "selGetLocalName start");
        ProtoDeviceName protoDeviceName = new ProtoDeviceName();
        int result = getLocalNameNative(protoDeviceName);
        Log.e(TAG, "getLocalNameNative is result is" + result);
        String localName = new String(protoDeviceName.name, StandardCharsets.UTF_8);
        Log.e(TAG, "LocalName end,name is " + localName);

        return localName;
    }

    public String getAddress() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        ProtoDeviceAddr protoDeviceAddr = new ProtoDeviceAddr();
        getLocalAddrNative(protoDeviceAddr);
        String address = ProtoDeviceAddrUtils.byte2Str(protoDeviceAddr.addr);
        return address;

    }

    public ProtoDeviceAddr getProtoAddress() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        ProtoDeviceAddr protoDeviceAddr = new ProtoDeviceAddr();
        int result = getLocalAddrNative(protoDeviceAddr);
        Log.e(TAG, "getLocalAddrNative end,result is  " + result);
        return protoDeviceAddr;

    }

    public int setProtoLocalAddr(byte type, byte[] addr) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK ADMIN permission");
        Log.e(TAG, "service setLocalAddr start");

        int result = setLocalAddrNative(type, addr);

        Log.e(TAG, "service setLocalAddr end, result is " + result);

        return result;
    }

    public boolean setName(String name) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK ADMIN permission");
        Log.e(TAG, "service setName");
        Intent intent;

        if (DdUtils.isBlank(name)) {
            Log.e(TAG, "setName, but name is blank" + name);
            return false;
        }

        synchronized (cachedLocalName) {
            cachedLocalName = name;
        }

        DiskUtils.writeNameOfLocalDeviceToDisk(name);

        byte[] nameBytes = name.getBytes(StandardCharsets.UTF_8);
        int res = setLocalNameNative((byte) nameBytes.length, nameBytes);

        intent = new Intent(NearlinkAdapter.ACTION_LOCAL_NAME_CHANGED);
        intent.putExtra(NearlinkAdapter.EXTRA_LOCAL_NAME, name);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        sAdapterService.sendBroadcast(intent, sAdapterService.NEARLINK_PERM);

        if (res == SUCCESS_FLAG) {
            return true;
        }
        return false;
    }

    void bringUpSle() {
        mAdapterProperties.init(mRemoteDevices);
    }

    void bringDownSle() {
        stopSleProfileService();
    }

    private void stopSleProfileService() {
        mAdapterProperties.onSleDisable();
        if (mRunningProfiles.size() == 0) {
            debugLog("stopSleProfileService() - No profiles services to stop.");
        }
    }

    public boolean setAddress(String address) {
        enforceCallingOrSelfPermission(NEARLINK_ADMIN_PERM, "Need NEARLINK ADMIN permission");
        ProtoDeviceAddr protoDeviceAddr = this.getProtoAddress();
        byte[] newAddressBytes = PubTools.hexToByteArray(address);
        int res = setProtoLocalAddr((byte) protoDeviceAddr.type, newAddressBytes);
        if (res == SUCCESS_FLAG) {
            Intent intent;
            intent = new Intent(NearlinkAdapter.ACTION_NEARLINK_ADDRESS_CHANGED);
            intent.putExtra(NearlinkAdapter.EXTRA_NEARLINK_ADDRESS, address);
            intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
            sAdapterService.sendBroadcast(intent, sAdapterService.NEARLINK_PERM);
            return true;
        }
        return false;
    }

    public int getState() {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        Log.e(TAG, "service getState");
        return mAdapterProperties.getState();
    }

    public void onServiceUp() {
        Log.e(TAG, "onServiceUp");
    }

    public void onServiceDown() {
        Log.e(TAG, "onServiceDown");
    }

    public boolean isQuietModeEnabled() {
        debugLog("isQuetModeEnabled() - Enabled = " + mQuietmode);
        return mQuietmode;
    }

    /**
     * 获取名称
     *
     * @param device
     * @return
     */
    public String getRemoteName(NearlinkDevice device) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (mRemoteDevices == null) {
            Log.e(TAG, "service getRemoteName, address =" + device.getAddress() + "mRemoteDevices is null");
            return null;
        }
        RemoteDevices.DeviceProperties deviceProp = mRemoteDevices.getDeviceProperties(device);
        if (deviceProp == null) {
            Log.e(TAG, "service getRemoteName, address =" + device.getAddress() + "deviceProp is null");
            return null;
        }
        return deviceProp.getName();
    }

    /**
     * 获取别名
     *
     * @param device
     * @return
     */
    public String getRemoteAlias(NearlinkDevice device) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (mRemoteDevices == null) {
            Log.e(TAG, "service getRemoteAlias, address =" + device.getAddress() + "mRemoteDevices is null");
            return null;
        }
        RemoteDevices.DeviceProperties deviceProp = mRemoteDevices.getDeviceProperties(device);
        if (deviceProp == null) {
            Log.e(TAG, "service getRemoteAlias, address =" + device.getAddress() + "deviceProp is null");
            return null;
        }
        return deviceProp.getAlias();
    }

    /**
     * 设置别名
     *
     * @param device
     * @param alias
     * @return
     */
    public boolean setRemoteAlias(NearlinkDevice device, String alias) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (mRemoteDevices == null) {
            Log.e(TAG, "service setRemoteAlias, address =" + device.getAddress() + "mRemoteDevices is null");
            return false;
        }

        boolean flag = false;
        try{
            int status = ConnectionManager.getInstance()
                    .getPairedState(device.getNearlinkAddress());
            flag = (status == NearlinkConstant.SLE_PAIR_PAIRED);
        }catch (Exception e){
            Log.e(TAG, "setAlias get SLE_PAIR_PAIRED status from ConnectionManager error", e);
            return false;
        }

        if(!flag){
            Log.e(TAG, "service setRemoteAlias, address ="
                    + PubTools.macPrint(device.getAddress()) + " is not SLE_PAIR_PAIRED");
            return false;
        }

        mRemoteDevices.addOrSetAliasByAddress(device, alias);

        return true;
    }


    public int getRemoteAppearence(NearlinkDevice device) {
        enforceCallingOrSelfPermission(NEARLINK_PERM, "Need NEARLINK permission");
        if (mRemoteDevices == null) {
            Log.e(TAG, "service getRemoteAppearence, address =" + device.getAddress() + "mRemoteDevices is null");
            return NearlinkAppearance.UNKNOWN;
        }
        RemoteDevices.DeviceProperties deviceProp = mRemoteDevices.getDeviceProperties(device);
        if (deviceProp == null) {
            Log.e(TAG, "service getRemoteAppearence, address =" + device.getAddress() + "deviceProp is null");
            return NearlinkAppearance.UNKNOWN;
        }
        return deviceProp.getAppearance();
    }

    private static class NearlinkBinder extends INearlink.Stub {

        private AdapterService mAdapterService;

        NearlinkBinder(AdapterService svc) {
            mAdapterService = svc;
        }

        public void cleanup() {
            mAdapterService = null;
        }

        public AdapterService getService() {
            if (mAdapterService != null && mAdapterService.isAvailable()) {
                return mAdapterService;
            }
            return null;
        }

        @Override
        public int getState() {
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "getState binder can not find service");
                return -1;
            }
            return service.getState();
        }

        @Override
        public boolean enable() {
            Log.e(TAG, "binder enable  ");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder enable service is null  ");
                return false;
            }
            return service.enable();
        }

        @Override
        public boolean disable() {
            Log.e(TAG, "binder disable  ");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder disable service is null  ");
                return false;
            }
            return service.disable();
        }

        @Override
        public boolean isEnabled() {
            Log.e(TAG, "binder isEnabled  ");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder isEnabled, but service is null");
                return false;
            }
            return service.isEnabled();
        }

        @Override
        public void registerCallback(INearlinkCallback cb) throws RemoteException {
            Log.e(TAG, "binder registerCallback");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder registerCallback, but service is null");
                return;
            }
            service.registerCallback(cb);
        }

        @Override
        public void unregisterCallback(INearlinkCallback cb) throws RemoteException {
            Log.e(TAG, "binder unregisterCallback");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder unregisterCallback, but service is null");
                return;
            }
            service.unregisterCallback(cb);
        }

        @Override
        public String getName() throws RemoteException {
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder getName,  but service is null");
                return "NO name";
            }
            return service.getName();
        }

        @Override
        public String getAddress() throws RemoteException {
            Log.e(TAG, "binder getAddress");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder getAddress, but service is null");
                return null;
            }
            return service.getAddress();
        }

        @Override
        public boolean setName(String name) throws RemoteException {
            Log.e(TAG, "binder setName");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder setName, but service is null");
                return false;
            }
            return service.setName(name);
        }

        @Override
        public void onServiceUp() throws RemoteException {
            Log.e(TAG, "binder onServiceUp entry ");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder onServiceUp, service is null ");
                return;
            }
            service.onServiceUp();
        }

        @Override
        public void onServiceDown() throws RemoteException {
            Log.e(TAG, "binder onServiceDown entry ");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder onServiceDown, service is null ");
                return;
            }
            service.onServiceDown();
        }

        @Override
        public boolean setAddress(String address) throws RemoteException {
            Log.e(TAG, "binder setAddress entry ");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder setAddress, service is null ");
                return false;
            }
            service.setAddress(address);
            return true;
        }


        @Override
        public ParcelUuid[] getUuids() throws RemoteException {
            return new ParcelUuid[0];
        }


        @Override
        public String getRemoteName(NearlinkDevice device) throws RemoteException {
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder getRemoteName, service is null ");
                return null;
            }
            return service.getRemoteName(device);
        }

        @Override
        public String getRemoteAlias(NearlinkDevice device) throws RemoteException {
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder getRemoteAlias, service is null ");
                return null;
            }
            return service.getRemoteAlias(device);
        }

        @Override
        public boolean setRemoteAlias(NearlinkDevice device, String name) throws RemoteException {
            Log.e(TAG, "binder setRemoteAlias entry ");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder setRemoteAlias, service is null ");
                return false;
            }
            return service.setRemoteAlias(device, name);
        }

        @Override
        public int getRemoteAppearence(NearlinkDevice device) throws RemoteException {
            Log.e(TAG, "binder getRemoteAppearence entry ");
            AdapterService service = getService();
            if (service == null) {
                Log.e(TAG, "binder getRemoteAppearence, service is null ");
                return NearlinkAppearance.UNKNOWN;
            }
            return service.getRemoteAppearence(device);
        }
    }

    public void startNearlink() {
        enableNative();
        bringUpSle();
        setProfileServiceState(DiscoveryService.class, NearlinkAdapter.STATE_ON);
        setProfileServiceState(ConnectionService.class, NearlinkAdapter.STATE_ON);
        setProfileServiceState(SsapClientService.class, NearlinkAdapter.STATE_ON);
        startProfileServices();
    }

    public void stopNearlinkNative() {
        stopNearlinkService();
    }

    public void stopNearlinkService() {
        bringDownSle();
        stopProfileServices();
        setProfileServiceState(SsapClientService.class, NearlinkAdapter.STATE_OFF);
        setProfileServiceState(ConnectionService.class, NearlinkAdapter.STATE_OFF);
        setProfileServiceState(DiscoveryService.class, NearlinkAdapter.STATE_OFF);
    }

    private void setAllProfileServiceStates(Class[] services, int state) {
        debugLog("setAllProfileServiceStates() services = " + Arrays.toString(services));
        for (Class service : services) {
            setProfileServiceState(service, state);
        }
    }

    private void setProfileServiceState(Class service, int state) {
        Intent intent = new Intent(this, service);
        intent.putExtra(EXTRA_ACTION, ACTION_SERVICE_STATE_CHANGED);
        intent.putExtra(NearlinkAdapter.EXTRA_STATE, state);
        startService(intent);
    }

    void startProfileServices() {
        debugLog("startCoreServices()");
        Class[] supportedProfileServices = Config.getSupportedProfiles();
        mAdapterProperties.onNearlinkReady();
        setAllProfileServiceStates(supportedProfileServices, NearlinkAdapter.STATE_ON);
    }

    void stopProfileServices() {
        debugLog("stopProfileServices()");
        Class[] supportedProfileServices = Config.getSupportedProfiles();
        setAllProfileServiceStates(supportedProfileServices, NearlinkAdapter.STATE_OFF);
    }

    public void updateAdapterState(int prevState, int newState) {
        mAdapterProperties.setState(newState);
        if (mCallbacks != null) {
            int n = mCallbacks.beginBroadcast();
            debugLog("updateAdapterState() - Broadcasting state " + NearlinkAdapter.nameForState(
                    newState) + " to " + n + " receivers.");
            for (int i = 0; i < n; i++) {
                try {
                    mCallbacks.getBroadcastItem(i).onNearlinkStateChange(prevState, newState);
                } catch (RemoteException e) {
                    debugLog("updateAdapterState() - Callback #" + i + " failed (" + e + ")");
                }
            }
            mCallbacks.finishBroadcast();
        }
    }

    private void debugLog(String msg) {
        Log.e(TAG, msg);
    }

    private void errorLog(String msg) {
        Log.e(TAG, msg);
    }


    public boolean isMock() {
        return false;
    }

    // NL ready start callback from jni
    void onEnabledCallback(int status) {
        Log.e(TAG, "onEnabledCallback,status is  " + status);
        nearlinkState.sendMessage(NearlinkState.NEARLINK_STARTED);
    }

    // NL ready stop callback from jni
    void onDisabledCallback(int status) {
        Log.e(TAG, "onDisabledCallback,status is  " + status);
        nearlinkState.sendMessage(NearlinkState.NEARLINK_STOPPED);
    }

    void cleanup() {
        debugLog("cleanup()");

        if (mCleaningUp) {
            errorLog("cleanup() - Service already starting to cleanup, ignoring request...");
            return;
        }

        clearAdapterService(this);

        mCleaningUp = true;

        if (nearlinkState != null) {
            nearlinkState.doQuit();
        }

        if (mRemoteDevices != null) {
            mRemoteDevices.cleanup();
        }

        if (mNativeAvailable) {
            debugLog("cleanup() - Cleaning up adapter native");
            cleanupNative();
            mNativeAvailable = false;
        }

        if (mAdapterProperties != null) {
            mAdapterProperties.cleanup();
        }

        if (mNearlinkBinder != null) {
            mNearlinkBinder.cleanup();
            mNearlinkBinder = null;  //Do not remove. Otherwise Binder leak!
        }

        if (mCallbacks != null) {
            mCallbacks.kill();
        }
    }

    static native void classInitNative();

    native boolean initNative();

    native void cleanupNative();

    native void enableNative();

    native void disableNative();

    native int getLocalNameNative(ProtoDeviceName protoDeviceName);

    native int setLocalNameNative(byte nameLen, byte[] name);

    native int getLocalAddrNative(ProtoDeviceAddr addrObj);

    native int setLocalAddrNative(byte type, byte[] addr);

}
