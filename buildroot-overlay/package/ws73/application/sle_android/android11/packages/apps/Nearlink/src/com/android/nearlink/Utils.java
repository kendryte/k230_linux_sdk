/*
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 */

package com.android.nearlink;

import android.app.ActivityManager;
import android.app.AppOpsManager;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.pm.PackageManager;
import android.content.pm.UserInfo;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.os.Binder;
import android.os.Build;
import android.os.ParcelUuid;
import android.os.Process;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

/**
 * @hide
 */

public final class Utils {
    private static final String TAG = "NearlinkUtils";
    private static final int MICROS_PER_UNIT = 625;
    private static final String PTS_TEST_MODE_PROPERTY = "persist.nearlink.pts";

    static final int BD_ADDR_LEN = 6; // bytes
    static final int BD_UUID_LEN = 16; // bytes

    public static String getAddressStringFromByte(byte[] address) {
        if (address == null || address.length != BD_ADDR_LEN) {
            return null;
        }

        return String.format("%02X:%02X:%02X:%02X:%02X:%02X", address[0], address[1], address[2],
                address[3], address[4], address[5]);
    }

    public static byte[] getByteAddress(NearlinkDevice device) {
        return getBytesFromAddress(device.getAddress());
    }

    public static byte[] getBytesFromAddress(String address) {
        int i = 0;
        int j = 0;
        byte[] output = new byte[BD_ADDR_LEN];

        for (i = 0; i < address.length(); i++) {
            if (address.charAt(i) != ':') {
                output[j] = (byte) Integer.parseInt(address.substring(i, i + 2), BD_UUID_LEN);
                j++;
                i++;
            }
        }

        return output;
    }

    public static int byteArrayToInt(byte[] valueBuf) {
        return byteArrayToInt(valueBuf, 0);
    }

    public static short byteArrayToShort(byte[] valueBuf) {
        ByteBuffer converter = ByteBuffer.wrap(valueBuf);
        converter.order(ByteOrder.nativeOrder());
        return converter.getShort();
    }

    public static int byteArrayToInt(byte[] valueBuf, int offset) {
        ByteBuffer converter = ByteBuffer.wrap(valueBuf);
        converter.order(ByteOrder.nativeOrder());
        return converter.getInt(offset);
    }

    public static String byteArrayToString(byte[] valueBuf) {
        StringBuilder sb = new StringBuilder();
        for (int idx = 0; idx < valueBuf.length; idx++) {
            if (idx != 0) {
                sb.append(" ");
            }
            sb.append(String.format("%02x", valueBuf[idx]));
        }
        return sb.toString();
    }

    public static byte[] intToByteArray(int value) {
        ByteBuffer converter = ByteBuffer.allocate(4);
        converter.order(ByteOrder.nativeOrder());
        converter.putInt(value);
        return converter.array();
    }

    public static byte[] uuidToByteArray(ParcelUuid pUuid) {
        int length = BD_UUID_LEN;
        ByteBuffer converter = ByteBuffer.allocate(length);
        converter.order(ByteOrder.BIG_ENDIAN);
        long msb;
        long lsb;
        UUID uuid = pUuid.getUuid();
        msb = uuid.getMostSignificantBits();
        lsb = uuid.getLeastSignificantBits();
        converter.putLong(msb);
        converter.putLong(8, lsb);
        return converter.array();
    }

    public static byte[] uuidsToByteArray(ParcelUuid[] uuids) {
        int length = uuids.length * BD_UUID_LEN;
        ByteBuffer converter = ByteBuffer.allocate(length);
        converter.order(ByteOrder.BIG_ENDIAN);
        UUID uuid;
        long msb;
        long lsb;
        for (int i = 0; i < uuids.length; i++) {
            uuid = uuids[i].getUuid();
            msb = uuid.getMostSignificantBits();
            lsb = uuid.getLeastSignificantBits();
            converter.putLong(i * BD_UUID_LEN, msb);
            converter.putLong(i * BD_UUID_LEN + 8, lsb);
        }
        return converter.array();
    }

    public static ParcelUuid[] byteArrayToUuid(byte[] val) {
        int numUuids = val.length / BD_UUID_LEN;
        ParcelUuid[] puuids = new ParcelUuid[numUuids];
        UUID uuid;
        int offset = 0;

        ByteBuffer converter = ByteBuffer.wrap(val);
        converter.order(ByteOrder.BIG_ENDIAN);

        for (int i = 0; i < numUuids; i++) {
            puuids[i] = new ParcelUuid(
                    new UUID(converter.getLong(offset), converter.getLong(offset + 8)));
            offset += BD_UUID_LEN;
        }
        return puuids;
    }

    public static String debugGetAdapterStateString(int state) {
        switch (state) {
            case NearlinkAdapter.STATE_OFF:
                return "STATE_OFF";
            case NearlinkAdapter.STATE_ON:
                return "STATE_ON";
            case NearlinkAdapter.STATE_TURNING_ON:
                return "STATE_TURNING_ON";
            case NearlinkAdapter.STATE_TURNING_OFF:
                return "STATE_TURNING_OFF";
            default:
                return "UNKNOWN";
        }
    }

    public static String ellipsize(String s) {
        // Only ellipsize release builds
        if (!Build.TYPE.equals("user")) {
            return s;
        }
        if (s == null) {
            return null;
        }
        if (s.length() < 3) {
            return s;
        }
        return s.charAt(0) + "⋯" + s.charAt(s.length() - 1);
    }

    public static void copyStream(InputStream is, OutputStream os, int bufferSize)
            throws IOException {
        if (is != null && os != null) {
            byte[] buffer = new byte[bufferSize];
            int bytesRead = 0;
            while ((bytesRead = is.read(buffer)) >= 0) {
                os.write(buffer, 0, bytesRead);
            }
        }
    }

    public static void safeCloseStream(InputStream is) {
        if (is != null) {
            try {
                is.close();
            } catch (Throwable t) {
                Log.d(TAG, "Error closing stream", t);
            }
        }
    }

    public static void safeCloseStream(OutputStream os) {
        if (os != null) {
            try {
                os.close();
            } catch (Throwable t) {
                Log.d(TAG, "Error closing stream", t);
            }
        }
    }

    static int sSystemUiUid = UserHandle.USER_NULL;
    public static void setSystemUiUid(int uid) {
        Utils.sSystemUiUid = uid;
    }

    static int sForegroundUserId = UserHandle.USER_NULL;
    public static void setForegroundUserId(int uid) {
        Utils.sForegroundUserId = uid;
    }

    public static boolean checkCaller() {
        int callingUser = UserHandle.getCallingUserId();
        int callingUid = Binder.getCallingUid();
        return (sForegroundUserId == callingUser) || (sSystemUiUid == callingUid)
                || (Process.SYSTEM_UID == callingUid);
    }

    public static boolean checkCallerAllowManagedProfiles(Context mContext) {
        if (mContext == null) {
            return checkCaller();
        }
        int callingUser = UserHandle.getCallingUserId();
        int callingUid = Binder.getCallingUid();

        // Use the Nearlink process identity when making call to get parent user
        long ident = Binder.clearCallingIdentity();
        try {
            UserManager um = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
            UserInfo ui = um.getProfileParent(callingUser);
            int parentUser = (ui != null) ? ui.id : UserHandle.USER_NULL;

            // Always allow SystemUI/System access.
            return (sForegroundUserId == callingUser) || (sForegroundUserId == parentUser)
                    || (sSystemUiUid == callingUid) || (Process.SYSTEM_UID == callingUid);
        } catch (Exception ex) {
            Log.e(TAG, "checkCallerAllowManagedProfiles: Exception ex=" + ex);
            return false;
        } finally {
            Binder.restoreCallingIdentity(ident);
        }
    }

    /**
     * Enforce the context has android.Manifest.permission.NEARLINK_ADMIN permission. A
     * {@link SecurityException} would be thrown if neither the calling process or the application
     * does not have NEARLINK_ADMIN permission.
     *
     * @param context Context for the permission check.
     */
    public static void enforceAdminPermission(ContextWrapper context) {
        context.enforceCallingOrSelfPermission(android.Manifest.permission.NEARLINK_ADMIN,
                "Need NEARLINK_ADMIN permission");
    }

    /**
     * Checks that calling process has android.Manifest.permission.ACCESS_COARSE_LOCATION or
     * android.Manifest.permission.ACCESS_FINE_LOCATION and a corresponding app op is allowed
     */
    public static boolean checkCallerHasLocationPermission(Context context, AppOpsManager appOps,
            String callingPackage) {
        if (context.checkCallingOrSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION)
                == PackageManager.PERMISSION_GRANTED && isAppOppAllowed(
                        appOps, AppOpsManager.OP_FINE_LOCATION, callingPackage)) {
            return true;
        }

        if (context.checkCallingOrSelfPermission(android.Manifest.permission.ACCESS_COARSE_LOCATION)
                == PackageManager.PERMISSION_GRANTED && isAppOppAllowed(
                        appOps, AppOpsManager.OP_COARSE_LOCATION, callingPackage)) {
            return true;
        }
        // Enforce location permission for apps targeting M and later versions
        if (isMApp(context, callingPackage)) {
            // PEERS_MAC_ADDRESS is another way to get scan results without
            // requiring location permissions, so only throw an exception here
            // if PEERS_MAC_ADDRESS permission is missing as well
            if (!checkCallerHasPeersMacAddressPermission(context)) {
                throw new SecurityException("Need ACCESS_COARSE_LOCATION or "
                        + "ACCESS_FINE_LOCATION permission to get scan results");
            }
        } else {
            // Pre-M apps running in the foreground should continue getting scan results
            if (isForegroundApp(context, callingPackage)) {
                return true;
            }
            Log.e(TAG, "Permission denial: Need ACCESS_COARSE_LOCATION or ACCESS_FINE_LOCATION "
                    + "permission to get scan results");
        }
        return false;
    }

    /**
     * Returns true if the caller holds PEERS_MAC_ADDRESS.
     */
    public static boolean checkCallerHasPeersMacAddressPermission(Context context) {
        return context.checkCallingOrSelfPermission(android.Manifest.permission.PEERS_MAC_ADDRESS)
                == PackageManager.PERMISSION_GRANTED;
    }

    public static boolean isLegacyForegroundApp(Context context, String pkgName) {
        return !isMApp(context, pkgName) && isForegroundApp(context, pkgName);
    }

    private static boolean isMApp(Context context, String pkgName) {
        try {
            return context.getPackageManager().getApplicationInfo(pkgName, 0).targetSdkVersion
                    >= Build.VERSION_CODES.M;
        } catch (PackageManager.NameNotFoundException e) {
            // In case of exception, assume M app
            Log.e(TAG, "getApplicationInfo targetSdkVersion: error ex=" + e);
        }
        return true;
    }

    /**
     * Return true if the specified package name is a foreground app.
     *
     * @param pkgName application package name.
     */
    private static boolean isForegroundApp(Context context, String pkgName) {
        ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> tasks = am.getRunningTasks(1);
        return !tasks.isEmpty() && pkgName.equals(tasks.get(0).topActivity.getPackageName());
    }

    private static boolean isAppOppAllowed(AppOpsManager appOps, int op, String callingPackage) {
        return appOps.noteOp(op, Binder.getCallingUid(), callingPackage)
                == AppOpsManager.MODE_ALLOWED;
    }

    /**
     * Converts {@code millisecond} to unit. Each unit is 0.625 millisecond.
     */
    public static int millsToUnit(int milliseconds) {
        return (int) (TimeUnit.MILLISECONDS.toMicros(milliseconds) / MICROS_PER_UNIT);
    }

    /**
     * Check if we are running in NearlinkInstrumentationTest context by trying to load
     * com.android.nearlink.FileSystemWriteTest. If we are not in Instrumentation test mode, this
     * class should not be found. Thus, the assumption is that FileSystemWriteTest must exist.
     * If FileSystemWriteTest is removed in the future, another test class in
     * NearlinkInstrumentationTest should be used instead
     *
     * @return true if in NearlinkInstrumentationTest, false otherwise
     */
    public static boolean isInstrumentationTestMode() {
        try {
            return Class.forName("com.android.nearlink.FileSystemWriteTest") != null;
        } catch (ClassNotFoundException exception) {
            return false;
        }
    }

    /**
     * Throws {@link IllegalStateException} if we are not in NearlinkInstrumentationTest. Useful
     * for ensuring certain methods only get called in NearlinkInstrumentationTest
     */
    public static void enforceInstrumentationTestMode() {
        if (!isInstrumentationTestMode()) {
            throw new IllegalStateException("Not in NearlinkInstrumentationTest");
        }
    }

    /**
     * Check if we are running in PTS test mode. To enable/disable PTS test mode, invoke
     * {@code adb shell setprop persist.nearlink.pts true/false}
     *
     * @return true if in PTS Test mode, false otherwise
     */
    public static boolean isPtsTestMode() {
        return SystemProperties.getBoolean(PTS_TEST_MODE_PROPERTY, false);
    }

    /**
     * Get uid/pid string in a binder call
     *
     * @return "uid/pid=xxxx/yyyy"
     */
    public static String getUidPidString() {
        return "uid/pid=" + Binder.getCallingUid() + "/" + Binder.getCallingPid();
    }
}
