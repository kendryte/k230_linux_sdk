/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.annotation.SystemApi;
import android.os.*;
import android.text.TextUtils;
import android.util.Log;

import static android.nearlink.NearlinkConstant.SLE_ACB_STATE_NONE;
import static android.nearlink.NearlinkConstant.SLE_PAIR_NONE;

/**
 * 设备对象
 *
 * @since 20231227
 */
public final class NearlinkDevice implements Parcelable {
    private static final String TAG = "NearlinkDevice";
    private static final boolean DBG = false;

    /**
     * Sentinel error value for this class. Guaranteed to not equal any other
     * integer constant in this class. Provided as a convenience for functions
     * that require a sentinel error value, for example:
     * <p>
     * <code>Intent.getIntExtra(NearlinkDevice.EXTRA_PAIR_STATE,
     * NearlinkDevice.ERROR)</code>
     */
    public static final int ERROR = Integer.MIN_VALUE;

    /**
     * Used as a Parcelable {@link NearlinkDevice} extra field in every intent
     * broadcast by this class. It contains the {@link NearlinkDevice} that
     * the intent applies to.
     */
    public static final String EXTRA_DEVICE = "android.nearlink.device.extra.DEVICE";
    public static final String EXTRA_ADDRESS = "android.nearlink.device.extra.ADDRESS";
    public static final String EXTRA_CONN_ID = "android.nearlink.device.extra.CONN_ID";

    /**
     * Used as a String extra field in {@link #ACTION_NAME_CHANGED} and {@link
     * #ACTION_FOUND} intents. It contains the friendly Nearlink name.
     */
    public static final String EXTRA_NAME = "android.nearlink.device.extra.NAME";

    /**
     * Used as a String extra field in  {@link
     * #ACTION_FOUND、ACTION_DETECTED} intents. It contains the  Nearlink alias.
     */
    public static final String EXTRA_ALIAS = "android.nearlink.device.extra.ALIAS";

    /**
     * Used as an optional short extra field in {@link #ACTION_FOUND} intents.
     * Contains the RSSI value of the remote device as reported by the
     * Nearlink hardware.
     */
    public static final String EXTRA_RSSI = "android.nearlink.device.extra.RSSI";

    /**
     * Used as a Parcelable {@link NearlinkClass} extra field in {@link
     * #ACTION_FOUND} and {@link #ACTION_CLASS_CHANGED} intents.
     */
    public static final String EXTRA_CLASS = "android.nearlink.device.extra.CLASS";

    /**
     * 设备外观，显示图标
     */
    public static final String EXTRA_APPEARANCE = "android.nearlink.device.extra.APPEARANCE";

    /**
     * Used as an int extra field in {@link #ACTION_BOND_STATE_CHANGED} intents.
     * Contains the bond state of the remote device.
     * <p>
     * Possible values are:
     * {@link #SLE_PAIR_NONE},
     * {@link #SLE_PAIR_PAIRING},
     * {@link #SLE_PAIR_PAIRED}.
     */
    public static final String EXTRA_PAIR_STATE = "android.nearlink.device.extra.PAIR_STATE";

    /**
     * Used as an int extra field in {@link #ACTION_BOND_STATE_CHANGED} intents.
     * Contains the previous bond state of the remote device.
     * <p>
     * Possible values are:
     * {@link #SLE_PAIR_NONE},
     * {@link #SLE_PAIR_PAIRING},
     * {@link #SLE_PAIR_PAIRED}.
     */
    public static final String EXTRA_PREVIOUS_PAIR_STATE = "android.nearlink.device.extra.PREVIOUS_PAIR_STATE";

    /**
     * Used as an int extra field in {@link #ACTION_PAIRING_REQUEST}
     * intents for unbond reason.
     *
     * @hide
     */
    public static final String EXTRA_REASON = "android.nearlink.device.extra.REASON";

    /**
     * For {@link #getPhonebookAccessPermission},
     * {@link #setPhonebookAccessPermission},
     * {@link #getMessageAccessPermission} and {@link #setMessageAccessPermission}.
     *
     * @hide
     */
    @SystemApi
    public static final int ACCESS_UNKNOWN = 0;

    /**
     * For {@link #getPhonebookAccessPermission},
     * {@link #setPhonebookAccessPermission},
     * {@link #getMessageAccessPermission} and {@link #setMessageAccessPermission}.
     *
     * @hide
     */
    @SystemApi
    public static final int ACCESS_ALLOWED = 1;

    /**
     * For {@link #getPhonebookAccessPermission},
     * {@link #setPhonebookAccessPermission},
     * {@link #getMessageAccessPermission} and {@link #setMessageAccessPermission}.
     *
     * @hide
     */
    @SystemApi
    public static final int ACCESS_REJECTED = 2;

    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_FOUND = "android.nearlink.device.action.FOUND";
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_DETECTED = "android.nearlink.device.action.DETECTED";
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_DISAPPEARED = "android.nearlink.device.action.DISAPPEARED";
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_NAME_CHANGED = "android.nearlink.device.action.NAME_CHANGED";
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_ALIAS_CHANGED = "android.nearlink.device.action.ALIAS_CHANGED";
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_CLASS_CHANGED = "android.nearlink.device.action.CLASS_CHANGED";
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_UUID = "android.nearlink.device.action.UUID";
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_BATTERY_LEVEL_CHANGED = "android.nearlink.device.action.BATTERY_LEVEL_CHANGED";

    /**
     * @hide
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_CONNECTION_ACCESS_REQUEST = "android.nearlink.device.action.CONNECTION_ACCESS_REQUEST";

    /**
     * @hide
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_CONNECTION_ACCESS_REPLY = "android.nearlink.device.action.CONNECTION_ACCESS_REPLY";

    /**
     * @hide
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_CONNECTION_ACCESS_CANCEL = "android.nearlink.device.action.CONNECTION_ACCESS_CANCEL";

    /**
     * Used as an extra field in {@link #ACTION_CONNECTION_ACCESS_REQUEST} intent.
     *
     * @hide
     */
    public static final String EXTRA_ACCESS_REQUEST_TYPE = "android.nearlink.device.extra.ACCESS_REQUEST_TYPE";

    /**
     * Used as an extra field in {@link #ACTION_CONNECTION_ACCESS_REPLY} intents,
     * Contains boolean to indicate if the allowed response is once-for-all so that
     * next request will be granted without asking user again.
     *
     * @hide
     */
    public static final String EXTRA_ALWAYS_ALLOWED = "android.nearlink.device.extra.ALWAYS_ALLOWED";

    /**
     * @hide
     */
    public static final int REQUEST_TYPE_PROFILE_CONNECTION = 1;

    /**
     * @hide
     */
    public static final int REQUEST_TYPE_PHONEBOOK_ACCESS = 2;

    /**
     * @hide
     */
    public static final int REQUEST_TYPE_MESSAGE_ACCESS = 3;

    /**
     * @hide
     */
    public static final int REQUEST_TYPE_SIM_ACCESS = 4;

    /**
     * Used as an extra field in {@link #ACTION_CONNECTION_ACCESS_REQUEST} intents,
     * Contains package name to return reply intent to.
     *
     * @hide
     */
    public static final String EXTRA_PACKAGE_NAME = "android.nearlink.device.extra.PACKAGE_NAME";

    /**
     * Used as an extra field in {@link #ACTION_CONNECTION_ACCESS_REQUEST} intents,
     * Contains class name to return reply intent to.
     *
     * @hide
     */
    public static final String EXTRA_CLASS_NAME = "android.nearlink.device.extra.CLASS_NAME";

    /**
     * Used as an extra field in {@link #ACTION_CONNECTION_ACCESS_REPLY} intent.
     *
     * @hide
     */
    public static final String EXTRA_CONNECTION_ACCESS_RESULT = "android.nearlink.device.extra.CONNECTION_ACCESS_RESULT";

    /**
     * @hide
     */
    public static final int CONNECTION_ACCESS_YES = 1;

    /**
     * @hide
     */
    public static final int CONNECTION_ACCESS_NO = 2;

    /**
     * Used as the unknown value for {@link #EXTRA_BATTERY_LEVEL} and
     * {@link #getBatteryLevel()}
     *
     * @hide
     */
    public static final int BATTERY_LEVEL_UNKNOWN = -1;

    public static final int MAX_ALIAS_LEN = 128;

    private static volatile INearlink sService;

    private final String mAddress;

    // 获取设备mac地址类型
    private int mType = 0;

    public int getAddressType() {
        return mType;
    }

    public NearlinkAddress getNearlinkAddress() {
        return new NearlinkAddress(this.mAddress, this.mType);
    }

    static INearlink getService() {
        synchronized (NearlinkDevice.class) {
            if (sService == null) {
                NearlinkAdapter adapter = NearlinkAdapter.getDefaultAdapter();
                sService = adapter.getNearlinkService(sStateChangeCallback);
            }
        }
        return sService;
    }

    static INearlinkManagerCallback sStateChangeCallback = new INearlinkManagerCallback.Stub() {
        public void onNearlinkServiceUp(INearlink nearlinkService) throws RemoteException {
            synchronized (NearlinkDevice.class) {
                if (sService == null) {
                    sService = nearlinkService;
                }
            }
        }

        public void onNearlinkServiceDown() throws RemoteException {
            synchronized (NearlinkDevice.class) {
                sService = null;
            }
        }
    };

    /**
     * @hide
     */
    NearlinkDevice(String address) {
        getService(); // ensures sService is initialized
        if (!NearlinkAdapter.checkNearlinkAddress(address)) {
            throw new IllegalArgumentException(address + " is not a valid Nearlink address");
        }

        mAddress = address;
    }

    NearlinkDevice(String address, int type) {
        getService(); // ensures sService is initialized
        if (!NearlinkAdapter.checkNearlinkAddress(address)) {
            throw new IllegalArgumentException(address + " is not a valid Nearlink address");
        }

        mAddress = address;
        mType = type;
    }

    NearlinkDevice(NearlinkAddress nearlinkAddress) {
        getService(); // ensures sService is initialized
        if (!NearlinkAdapter.checkNearlinkAddress(nearlinkAddress.getAddress())) {
            throw new IllegalArgumentException(nearlinkAddress.getAddress() + " is not a valid Nearlink address");
        }

        mAddress = nearlinkAddress.getAddress();
        mType = nearlinkAddress.getType();
    }

    @Override
    public boolean equals(Object o) {
        if (o instanceof NearlinkDevice) {
            return mAddress.equals(((NearlinkDevice) o).getAddress());
        }
        return false;
    }

    @Override
    public int hashCode() {
        return mAddress.hashCode();
    }

    @Override
    public String toString() {
        return mAddress;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<NearlinkDevice> CREATOR = new Creator<NearlinkDevice>() {
        public NearlinkDevice createFromParcel(Parcel in) {
            return new NearlinkDevice(in.readString(), in.readInt());
        }

        public NearlinkDevice[] newArray(int size) {
            return new NearlinkDevice[size];
        }
    };

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mAddress);
        out.writeInt(mType);
    }

    public String getAddress() {
        if (DBG) {
            Log.d(TAG, "mAddress: " + mAddress);
        }
        return mAddress;
    }

    public String getName() {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "SLE not enabled. Cannot get Remote Device name");
            return null;
        }
        try {
            return service.getRemoteName(this);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return null;
    }

    /**
     * @hide
     */
    public String getAlias() {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "SLE not enabled. Cannot get Remote Device Alias");
            return null;
        }
        try {
            return service.getRemoteAlias(this);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return null;
    }

    /**
     * @hide
     */
    public boolean setAlias(String alias) {
        if(alias == null || alias.length() > MAX_ALIAS_LEN){
            Log.e(TAG, "alias is null or exceed MAX_ALIAS_LEN");
            return false;
        }
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "SLE not enabled. Cannot set Remote Device name");
            return false;
        }
        try {
            return service.setRemoteAlias(this, alias);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return false;
    }

    /**
     * @hide
     */
    public String getAliasName() {
        String name = getAlias();
        if (name == null || TextUtils.isEmpty(name)) {
            name = getName();
        }
        return name;
    }

    /**
     * create connect
     *
     * @return the method is exec success? true or false
     * @throws RemoteException RemoteException
     */
    public boolean createConnect() throws RemoteException {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "NL not enabled. Cannot cancel Remote Device bond");
            return false;
        }
        try {
            return NearlinkAdapter.getDefaultAdapter().getNearlinkConnection().createConnect(getNearlinkAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return false;
    }

    /**
     * dis connect
     *
     * @return the method is exec success? true or false
     * @throws RemoteException RemoteException
     */
    public boolean disConnect() throws RemoteException {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "NL not enabled. Cannot cancel Remote Device bond");
            return false;
        }
        try {
            return NearlinkAdapter.getDefaultAdapter().getNearlinkConnection().disConnect(getNearlinkAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return false;
    }

    public boolean cancelPairProcess() {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "NL not enabled. Cannot cancel Remote Device bond");
            return false;
        }
        try {
            return NearlinkAdapter.getDefaultAdapter().getNearlinkConnection().cancelPairProcess(getNearlinkAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return false;
    }

    public boolean removePair() {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "SLE not enabled. Cannot remove Remote Device pair");
            return false;
        }
        try {
            return NearlinkAdapter.getDefaultAdapter().getNearlinkConnection().removePair(getNearlinkAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return false;
    }

    public int getPairState() {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "SLE not enabled. Cannot get pair state");
            return SLE_PAIR_NONE;
        }
        try {
            return NearlinkAdapter.getDefaultAdapter().getNearlinkConnection().getPairState(getNearlinkAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return SLE_PAIR_NONE;
    }

    public int getConnectionState() {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "SLE not enabled. Cannot get conn state");
            return SLE_ACB_STATE_NONE;
        }
        try {
            return NearlinkAdapter.getDefaultAdapter().getNearlinkConnection().getConnectionState(getNearlinkAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return SLE_ACB_STATE_NONE;
    }

    public boolean isConnected() {
        return getConnectionState() == NearlinkConstant.SLE_ACB_STATE_CONNECTED;
    }

    public boolean isPaired() {
        return getPairState() == NearlinkConstant.SLE_PAIR_PAIRED;
    }

    public int getNearlinkAppearance() {
        final INearlink service = sService;
        if (service == null) {
            Log.e(TAG, "Nearlink not enabled. Cannot get Nearlink appearence");
            return NearlinkAppearance.UNKNOWN;
        }
        try {
            return service.getRemoteAppearence(this);
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return NearlinkAppearance.UNKNOWN;
    }

    public ParcelUuid[] getUuids() {
        return null;
    }

    boolean isNearlinkEnabled() {
        boolean ret = false;
        NearlinkAdapter adapter = NearlinkAdapter.getDefaultAdapter();
        if (adapter != null && adapter.isEnabled()) {
            ret = true;
        }
        return ret;
    }

    /**
     * @hide
     */
    public boolean isPairingInitiatedLocally() {
        final INearlink service = sService;
        if (service == null) {
            Log.w(TAG, "NL not enabled, isPairingInitiatedLocally failed");
            return false;
        }
        try {
            return NearlinkAdapter.getDefaultAdapter().getNearlinkConnection()
                    .isPairingInitiatedLocally(getNearlinkAddress());
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return false;
    }

    /**
     * Connect to SSAP Server hosted by this device. Caller acts as SSAP client.
     * The callback is used to deliver results to Caller, such as connection status
     * as well
     * as any further SSAP client operations.
     * The method returns a NearlinkSsapClient instance. You can use
     * NearlinkSsapClient to conduct
     * SSAP client operations.
     *
     * @param callback SSAP callback handler that will receive asynchronous
     *            callbacks.
     * @throws NullPointerException if callback is null
     */
    public NearlinkSsapClient connectSsap(NearlinkSsapClientCallback callback) {
        return (connectSsap(callback, null));
    }

    /**
     * Connect to SSAP Server hosted by this device. Caller acts as SSAP client.
     * The callback is used to deliver results to Caller, such as connection status
     * as well
     * as any further SSAP client operations.
     * The method returns a NearlinkSsapClient instance. You can use
     * NearlinkSsapClient to conduct
     * SSAP client operations.
     *
     * @param callback SSAP callback handler that will receive asynchronous
     *            callbacks.
     * @param handler The handler to use for the callback. If {@code null},
     *            callbacks will happen on
     *            an un-specified background thread.
     * @throws NullPointerException if callback is null
     */
    public NearlinkSsapClient connectSsap(NearlinkSsapClientCallback callback, Handler handler) {
        if (callback == null) {
            throw new NullPointerException("callback is null");
        }

        NearlinkAdapter adapter = NearlinkAdapter.getDefaultAdapter();
        INearlinkManager managerService = adapter.getNearlinkManager();
        try {
            INearlinkSsapClient iClient = managerService.getNearlinkSsapClient();
            if (iClient == null) {
                Log.e(TAG, "SLE is not supported.");
                return null;
            }
            NearlinkSsapClient client = new NearlinkSsapClient(iClient, this);
            client.connect(callback, handler);
            return client;
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
        }
        return null;
    }

    /**
     * @return Whether the phonebook access is allowed to this device. Can be {@link
     *         #ACCESS_UNKNOWN}, {@link #ACCESS_ALLOWED} or
     *         {@link #ACCESS_REJECTED}.
     * @hide
     */
    public int getPhonebookAccessPermission() {
        return ACCESS_UNKNOWN;
    }

    /**
     * Sets whether the phonebook access is allowed to this device.
     *
     * @param value Can be {@link #ACCESS_UNKNOWN}, {@link #ACCESS_ALLOWED} or
     *            {@link
     *            #ACCESS_REJECTED}.
     * @return Whether the value has been successfully set.
     * @hide
     */
    @SystemApi
    public boolean setPhonebookAccessPermission(int value) {
        return false;
    }

    /**
     * @return Whether the message access is allowed to this device. Can be
     *         {@link #ACCESS_UNKNOWN},
     *         {@link #ACCESS_ALLOWED} or {@link #ACCESS_REJECTED}.
     * @hide
     */
    public int getMessageAccessPermission() {
        return ACCESS_UNKNOWN;
    }

    /**
     * Sets whether the message access is allowed to this device.
     *
     * @param value Can be {@link #ACCESS_UNKNOWN}, {@link #ACCESS_ALLOWED} or
     *            {@link
     *            #ACCESS_REJECTED}.
     * @return Whether the value has been successfully set.
     * @hide
     */
    public boolean setMessageAccessPermission(int value) {
        return false;
    }

    /**
     * @return Whether the Sim access is allowed to this device. Can be
     *         {@link #ACCESS_UNKNOWN},
     *         {@link #ACCESS_ALLOWED} or {@link #ACCESS_REJECTED}.
     * @hide
     */
    public int getSimAccessPermission() {
        return ACCESS_UNKNOWN;
    }

    /**
     * Sets whether the Sim access is allowed to this device.
     *
     * @param value Can be {@link #ACCESS_UNKNOWN}, {@link #ACCESS_ALLOWED} or
     *            {@link
     *            #ACCESS_REJECTED}.
     * @return Whether the value has been successfully set.
     * @hide
     */
    public boolean setSimAccessPermission(int value) {
        return false;
    }

    /**
     * Get the most recent identified battery level of this Nearlink device
     *
     * @return Battery level in percents from 0 to 100, or
     *         {@link #BATTERY_LEVEL_UNKNOWN} if
     *         Nearlink is disabled, or device is disconnected, or does not have any
     *         battery reporting
     *         service, or return value is invalid
     * @hide
     */
    public int getBatteryLevel() {
        return BATTERY_LEVEL_UNKNOWN;
    }

    public boolean isNearlinkDock() {
        return false;
    }
}
