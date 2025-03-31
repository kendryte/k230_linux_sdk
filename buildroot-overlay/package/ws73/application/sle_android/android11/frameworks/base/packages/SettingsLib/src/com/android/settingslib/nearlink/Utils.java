/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.nearlink.NearlinkAppearance;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkProfile;
import android.support.annotation.DrawableRes;
import android.util.Pair;

import com.android.settingslib.R;
import com.android.settingslib.graph.BluetoothDeviceLayerDrawable;

/**
 * @since 2023-12-04
 */
public class Utils {
    public static final boolean V = true; // verbose logging
    public static final boolean D = true;  // regular logging

    private static ErrorListener sErrorListener;

    public static int getConnectionStateSummary(int connectionState) {
        switch (connectionState) {
            case NearlinkProfile.STATE_CONNECTED:
                return R.string.nearlink_connected;
            case NearlinkProfile.STATE_CONNECTING:
                return R.string.nearlink_connecting;
            case NearlinkProfile.STATE_DISCONNECTED:
                return R.string.nearlink_disconnected;
            case NearlinkProfile.STATE_DISCONNECTING:
                return R.string.nearlink_disconnecting;
            default:
                return 0;
        }
    }

    static void showError(Context context, String name, int messageResId) {
        if (sErrorListener != null) {
            sErrorListener.onShowError(context, name, messageResId);
        }
    }

    public static void setErrorListener(ErrorListener listener) {
        sErrorListener = listener;
    }

    public interface ErrorListener {
        void onShowError(Context context, String name, int messageResId);
    }

    public static Pair<Drawable, String> getNlClassDrawableWithDescription(Context context, CachedNearlinkDevice cachedDevice) {
        return getNlClassDrawableWithDescription(context, cachedDevice, 1 /* iconScale */);
    }

    public static Pair<Drawable, String> getNlClassDrawableWithDescription(Context context, CachedNearlinkDevice cachedDevice, float iconScale) {
        int nlClass = cachedDevice.getNlClass();
        final int level = cachedDevice.getBatteryLevel();
        switch (nlClass) {
            case NearlinkAppearance.GENERAL_PHONE:
                return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_bt_cellphone, level, iconScale), context.getString(R.string.nearlink_talkback_phone));

            case NearlinkAppearance.HEAD_HEADSET:
            case NearlinkAppearance.NECK_HEADSET:
                return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_bt_headphones_a2dp, level, iconScale),
                        context.getString(R.string.nearlink_talkback_headphone));

            case NearlinkAppearance.HEAD_EARPHONE:
                return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_bt_headset_hfp, level, iconScale),
                        context.getString(R.string.nearlink_talkback_headset));

            case NearlinkAppearance.IN_EAR_HEADSET:
                return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_bt_hearing_aid, level, iconScale),
                        context.getString(R.string.nearlink_talkback_headphone));

            case NearlinkAppearance.GENERAL_COMPUTER:
            case NearlinkAppearance.TOUCH_PAD:
            case NearlinkAppearance.TABLET_PC:
                return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_bt_laptop, level, iconScale),
                        context.getString(R.string.nearlink_talkback_computer));

            case NearlinkAppearance.MOUSE:
                return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_bt_pointing_hid, level, iconScale),
                        context.getString(R.string.nearlink_talkback_input_peripheral));

            case NearlinkAppearance.GENERAL_HID:
                return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_bt_misc_hid, level, iconScale),
                        context.getString(R.string.nearlink_talkback_input_peripheral));

            case NearlinkAppearance.KEYBOARD:
                return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_lockscreen_ime, level, iconScale),
                        context.getString(R.string.nearlink_talkback_input_peripheral));

            default:
                // unrecognized device class; continue
        }

        return new Pair<>(getNearlinkDrawable(context, R.drawable.ic_settings_nearlink, level, iconScale), context.getString(R.string.nearlink_talkback_nearlink));
    }

    public static Drawable getNearlinkDrawable(Context context, @DrawableRes int resId, int batteryLevel, float iconScale) {
        if (batteryLevel != NearlinkDevice.BATTERY_LEVEL_UNKNOWN) {
            return BluetoothDeviceLayerDrawable.createLayerDrawable(context, resId, batteryLevel,
                    iconScale);
        } else {
            return context.getDrawable(resId);
        }
    }
}
