/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.annotation.SdkConstant;
import android.annotation.SdkConstant.SdkConstantType;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * This class provides the public APIs to control the Nearlink Input
 * Device Profile.
 *
 * <p>NearlinkHidHost is a proxy object for controlling the Nearlink
 * Service via IPC. Use {@link NearlinkAdapter#getProfileProxy} to get
 * the NearlinkHidHost proxy object.
 *
 * <p>Each method is protected with its appropriate permission.
 *
 * @since 2023-11-14
 *
 * @hide
 */
public final class NearlinkHidHost implements NearlinkProfile {
    private static final String TAG = "NearlinkHidHost";
    private static final boolean DBG = true;
    private static final boolean VDBG = true;

    /**
     * Intent used to broadcast the change in connection state of the Input
     * Device profile.
     *
     * <p>This intent will have 3 extras:
     * <ul>
     * <li> {@link #EXTRA_STATE} - The current state of the profile. </li>
     * <li> {@link #EXTRA_PREVIOUS_STATE}- The previous state of the profile.</li>
     * <li> {@link NearlinkDevice#EXTRA_DEVICE} - The remote device. </li>
     * </ul>
     *
     * <p>{@link #EXTRA_STATE} or {@link #EXTRA_PREVIOUS_STATE} can be any of
     * {@link #STATE_DISCONNECTED}, {@link #STATE_CONNECTING},
     * {@link #STATE_CONNECTED}, {@link #STATE_DISCONNECTING}.
     *
     * <p>Requires {@link android.Manifest.permission#NEARLINK} permission to
     * receive.
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_CONNECTION_STATE_CHANGED =
            "android.nearlink.input.profile.action.CONNECTION_STATE_CHANGED";

    /**
     * @hide
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_HANDSHAKE =
            "android.nearlink.input.profile.action.HANDSHAKE";

    /**
     * @hide
     */
    @SdkConstant(SdkConstantType.BROADCAST_INTENT_ACTION)
    public static final String ACTION_REPORT =
            "android.nearlink.input.profile.action.REPORT";

    /**
     * Return codes for the connect and disconnect.
     *
     * @hide
     */
    public static final int INPUT_DISCONNECT_FAILED_NOT_CONNECTED = 5000;

    /**
     * @hide
     */
    public static final int INPUT_CONNECT_FAILED_ALREADY_CONNECTED = 5001;

    /**
     * @hide
     */
    public static final int INPUT_CONNECT_FAILED_ATTEMPT_FAILED = 5002;

    /**
     * @hide
     */
    public static final int INPUT_OPERATION_GENERIC_FAILURE = 5003;

    /**
     * @hide
     */
    public static final int INPUT_OPERATION_SUCCESS = 5004;

    /**
     * @hide
     */
    public static final String EXTRA_REPORT_TYPE =
            "android.nearlink.NearlinkHidHost.extra.REPORT_TYPE";

    /**
     * @hide
     */
    public static final String EXTRA_REPORT_ID =
            "android.nearlink.NearlinkHidHost.extra.REPORT_ID";

    /**
     * @hide
     */
    public static final String EXTRA_REPORT_BUFFER_SIZE =
            "android.nearlink.NearlinkHidHost.extra.REPORT_BUFFER_SIZE";

    /**
     * @hide
     */
    public static final String EXTRA_REPORT = "android.nearlink.NearlinkHidHost.extra.REPORT";

    /**
     * @hide
     */
    public static final String EXTRA_STATUS = "android.nearlink.NearlinkHidHost.extra.STATUS";

    private Context mContext;
    private ServiceListener mServiceListener;
    private NearlinkAdapter mAdapter;
    private volatile INearlinkHidHost mService;

    private final INearlinkStateChangeCallback mNearlinkStateChangeCallback =
            new INearlinkStateChangeCallback.Stub() {
                public void onNearlinkStateChange(boolean up) {
                    if (DBG) Log.e(TAG, "onNearlinkStateChange: up=" + up);
                    if (!up) {
                        if (VDBG) Log.e(TAG, "Unbinding service...");
                        synchronized (mConnection) {
                            try {
                                mService = null;
                                mContext.unbindService(mConnection);
                            } catch (Exception re) {
                                Log.e(TAG, "", re);
                            }
                        }
                    } else {
                        synchronized (mConnection) {
                            try {
                                if (mService == null) {
                                    if (VDBG) Log.e(TAG, "Binding service...");
                                    doBind();
                                }
                            } catch (Exception re) {
                                Log.e(TAG, "", re);
                            }
                        }
                    }
                }
            };

    private final ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            if (DBG) Log.d(TAG, "Proxy object connected");
            mService = INearlinkHidHost.Stub.asInterface(Binder.allowBlocking(service));
            if (mServiceListener != null) {
                mServiceListener.onServiceConnected(NearlinkProfile.HID_HOST,
                        NearlinkHidHost.this);
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            if (DBG) Log.d(TAG, "Proxy object disconnected");
            mService = null;
            if (mServiceListener != null) {
                mServiceListener.onServiceDisconnected(NearlinkProfile.HID_HOST);
            }
        }
    };

    /**
     * Create a NearlinkHidHost proxy object for interacting with the local
     * Nearlink Service which handles the InputDevice profile
     */
    NearlinkHidHost(Context context, ServiceListener l) {
        if (DBG) Log.d(TAG, "create NearlinkHidHost");
        mContext = context;
        mServiceListener = l;
        mAdapter = NearlinkAdapter.getDefaultAdapter();

        INearlinkManager mgr = mAdapter.getNearlinkManager();
        if (mgr != null) {
            try {
                mgr.registerStateChangeCallback(mNearlinkStateChangeCallback);
            } catch (RemoteException e) {
                Log.e(TAG, "", e);
            }
        }

        doBind();
    }

    boolean doBind() {
        Intent intent = new Intent(INearlinkHidHost.class.getName());
        ComponentName comp = intent.resolveSystemService(mContext.getPackageManager(), 0);
        intent.setComponent(comp);
        boolean bindServiceAsUser = mContext.bindServiceAsUser(intent, mConnection, 0, mContext.getUser());
        if (DBG) Log.d(TAG, "doBind: comp = " + comp + " , bindServiceAsUser = " + bindServiceAsUser);
        if (comp == null || !bindServiceAsUser) {
            Log.e(TAG, "Could not bind to Nearlink HID Service with " + intent);
            return false;
        }
        return true;
    }

    void close() {
        if (VDBG) log("close()");
        INearlinkManager mgr = mAdapter.getNearlinkManager();
        if (mgr != null) {
            try {
                mgr.unregisterStateChangeCallback(mNearlinkStateChangeCallback);
            } catch (Exception e) {
                Log.e(TAG, "", e);
            }
        }

        synchronized (mConnection) {
            if (mService != null) {
                try {
                    mService = null;
                    mContext.unbindService(mConnection);
                } catch (Exception re) {
                    Log.e(TAG, "", re);
                }
            }
        }
        mServiceListener = null;
    }

    /**
     * Initiate connection to a profile of the remote nearlink device.
     *
     * <p> The system supports connection to multiple input devices.
     *
     * <p> This API returns false in scenarios like the profile on the
     * device is already connected or Nearlink is not turned on.
     * When this API returns true, it is guaranteed that
     * connection state intent for the profile will be broadcasted with
     * the state. Users can get the connection state of the profile
     * from this intent.
     *
     * <p>Requires {@link android.Manifest.permission#NEARLINK_ADMIN}
     * permission.
     *
     * @param device Remote Nearlink Device
     * @return false on immediate error, true otherwise
     * @hide
     */
    public boolean connect(NearlinkDevice device) {
        if (DBG) log("connect(" + device + ")");
        final INearlinkHidHost service = mService;
        boolean enabled = isEnabled();
        boolean isValidDevice = isValidDevice(device);
        if (service != null && enabled && isValidDevice) {
            try {
                return service.connect(device);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
                return false;
            }
        }
        if (service == null) Log.e(TAG, "Proxy not attached to service");
        return false;
    }

    /**
     * Initiate disconnection from a profile
     *
     * <p> This API will return false in scenarios like the profile on the
     * Nearlink device is not in connected state etc. When this API returns,
     * true, it is guaranteed that the connection state change
     * intent will be broadcasted with the state. Users can get the
     * disconnection state of the profile from this intent.
     *
     * <p> If the disconnection is initiated by a remote device, the state
     * will transition from {@link #STATE_CONNECTED} to
     * {@link #STATE_DISCONNECTED}. If the disconnect is initiated by the
     * host (local) device the state will transition from
     * {@link #STATE_CONNECTED} to state {@link #STATE_DISCONNECTING} to
     * state {@link #STATE_DISCONNECTED}. The transition to
     * {@link #STATE_DISCONNECTING} can be used to distinguish between the
     * two scenarios.
     *
     * <p>Requires {@link android.Manifest.permission#NEARLINK_ADMIN}
     * permission.
     *
     * @param device Remote Nearlink Device
     * @return false on immediate error, true otherwise
     * @hide
     */
    public boolean disconnect(NearlinkDevice device) {
        if (DBG) log("disconnect(" + device + ")");
        final INearlinkHidHost service = mService;
        boolean enabled = isEnabled();
        boolean isValidDevice = isValidDevice(device);
        if (service != null && enabled && isValidDevice) {
            try {
                return service.disconnect(device);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
                return false;
            }
        }
        if (service == null) Log.w(TAG, "Proxy not attached to service");
        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public List<NearlinkDevice> getConnectedDevices() {
        if (VDBG) log("getConnectedDevices()");
        final INearlinkHidHost service = mService;
        if (service != null && isEnabled()) {
            try {
                return service.getConnectedDevices();
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
                return new ArrayList<NearlinkDevice>();
            }
        }
        if (service == null) Log.w(TAG, "Proxy not attached to service");
        return new ArrayList<NearlinkDevice>();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public List<NearlinkDevice> getDevicesMatchingConnectionStates(int[] states) {
        if (VDBG) log("getDevicesMatchingStates() states = " + states);
        final INearlinkHidHost service = mService;
        if (service != null && isEnabled()) {
            try {
                return service.getDevicesMatchingConnectionStates(states);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
                return new ArrayList<NearlinkDevice>();
            }
        }
        if (service == null) Log.w(TAG, "Proxy not attached to service");
        return new ArrayList<NearlinkDevice>();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getConnectionState(NearlinkDevice device) {
        if (VDBG) log("getState(" + device + ")");
        final INearlinkHidHost service = mService;
        if (service != null && isEnabled() && isValidDevice(device)) {
            try {
                return service.getConnectionState(device);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
                return NearlinkProfile.STATE_DISCONNECTED;
            }
        }
        if (service == null) Log.w(TAG, "Proxy not attached to service");
        return NearlinkProfile.STATE_DISCONNECTED;
    }

    /**
     * Set priority of the profile
     *
     * <p> The device should already be paired.
     * Priority can be one of {@link #PRIORITY_ON} or
     * {@link #PRIORITY_OFF},
     *
     * <p>Requires {@link android.Manifest.permission#NEARLINK_ADMIN}
     * permission.
     *
     * @param device   Paired nearlink device
     * @param priority
     * @return true if priority is set, false on error
     * @hide
     */
    public boolean setPriority(NearlinkDevice device, int priority) {
        if (DBG) log("setPriority(" + device + ", " + priority + ")");
        final INearlinkHidHost service = mService;
        if (service != null && isEnabled() && isValidDevice(device)) {
            if (priority != NearlinkProfile.PRIORITY_OFF
                    && priority != NearlinkProfile.PRIORITY_ON) {
                return false;
            }
            try {
                return service.setPriority(device, priority);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
                return false;
            }
        }
        if (service == null) Log.w(TAG, "Proxy not attached to service");
        return false;
    }

    /**
     * Get the priority of the profile.
     *
     * <p> The priority can be any of:
     * {@link #PRIORITY_AUTO_CONNECT}, {@link #PRIORITY_OFF},
     * {@link #PRIORITY_ON}, {@link #PRIORITY_UNDEFINED}
     *
     * <p>Requires {@link android.Manifest.permission#NEARLINK} permission.
     *
     * @param device Nearlink device
     * @return priority of the device
     * @hide
     */
    public int getPriority(NearlinkDevice device) {
        if (DBG) log("getPriority(" + device + ")");
        final INearlinkHidHost service = mService;
        if (service != null && isEnabled() && isValidDevice(device)) {
            try {
                return service.getPriority(device);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
                return NearlinkProfile.PRIORITY_OFF;
            }
        }
        if (service == null) Log.w(TAG, "Proxy not attached to service");
        return NearlinkProfile.PRIORITY_OFF;
    }

    private boolean isEnabled() {
        return mAdapter.getState() == NearlinkAdapter.STATE_ON;
    }

    private static boolean isValidDevice(NearlinkDevice device) {
        return device != null && NearlinkAdapter.checkNearlinkAddress(device.getAddress());
    }

    /**
     * Send Set_Report command to the connected HID input device.
     *
     * <p>Requires {@link android.Manifest.permission#NEARLINK_ADMIN} permission.
     *
     * @param device     Remote Nearlink Device
     * @param reportType Report type
     * @param report     Report receiving buffer size
     * @return false on immediate error, true otherwise
     * @hide
     */
    public boolean setReport(NearlinkDevice device, byte reportType, String report) {
        if (VDBG) log("setReport(" + device + "), reportType=" + reportType + " report=" + report);
        final INearlinkHidHost service = mService;
        boolean enabled = isEnabled();
        boolean isValidDevice = isValidDevice(device);
        if (service != null && enabled && isValidDevice) {
            try {
                return service.setReport(device, reportType, report);
            } catch (RemoteException e) {
                Log.e(TAG, "Stack:" + Log.getStackTraceString(new Throwable()));
                return false;
            }
        }
        if (service == null) Log.w(TAG, "Proxy not attached to service");
        return false;
    }

    private static void log(String msg) {
        Log.e(TAG, msg);
    }
}
