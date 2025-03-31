/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settingslib.nearlink;

import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.os.ParcelUuid;
import android.util.Log;

/**
 * NearlinkDeviceFilter contains a static method that returns a
 * Filter object that returns whether or not the NearlinkDevice
 * passed to it matches the specified filter type constant from
 * {@link android.nearlink.NearlinkDevicePicker}.
 *
 * @since 2023-12-04
 */
public final class NearlinkDeviceFilter {
    private static final String TAG = "NearlinkDeviceFilter";

    /**
     * The filter interface to external classes.
     */
    public interface Filter {
        boolean matches(NearlinkDevice device);
    }

    /**
     * All filter singleton (referenced directly).
     */
    public static final Filter ALL_FILTER = new AllFilter();

    /**
     * Bonded devices only filter (referenced directly).
     */
    public static final Filter BONDED_DEVICE_FILTER = new BondedDeviceFilter();

    /**
     * Unbonded devices only filter (referenced directly).
     */
    public static final Filter UNBONDED_DEVICE_FILTER = new UnbondedDeviceFilter();

    /**
     * Table of singleton filter objects.
     */
    private static final Filter[] FILTERS = {
            ALL_FILTER,             // FILTER_TYPE_ALL
    };

    /**
     * Private constructor.
     */
    private NearlinkDeviceFilter() {
    }

    /**
     * Returns the singleton {@link Filter} object for the specified type,
     * or {@link #ALL_FILTER} if the type value is out of range.
     *
     * @param filterType a constant from NearlinkDevicePicker
     * @return a singleton object implementing the {@link Filter} interface.
     */
    public static Filter getFilter(int filterType) {
        if (filterType >= 0 && filterType < FILTERS.length) {
            return FILTERS[filterType];
        } else {
            Log.w(TAG, "Invalid filter type " + filterType + " for device picker");
            return ALL_FILTER;
        }
    }

    /**
     * Filter that matches all devices.
     */
    private static final class AllFilter implements Filter {
        public boolean matches(NearlinkDevice device) {
            return true;
        }
    }

    /**
     * Filter that matches only bonded devices.
     */
    private static final class BondedDeviceFilter implements Filter {
        public boolean matches(NearlinkDevice device) {
            return device.getPairState() == NearlinkConstant.SLE_PAIR_PAIRED;
        }
    }

    /**
     * Filter that matches only unbonded devices.
     */
    private static final class UnbondedDeviceFilter implements Filter {
        public boolean matches(NearlinkDevice device) {
            return device.getPairState() != NearlinkConstant.SLE_PAIR_PAIRED;
        }
    }

    /**
     * Parent class of filters based on UUID and/or Nearlink class.
     */
    private abstract static class ClassUuidFilter implements Filter {
        abstract boolean matches(ParcelUuid[] uuids, int nlClass);

        public boolean matches(NearlinkDevice device) {
            return matches(device.getUuids(), device.getNearlinkAppearance());
        }
    }
}
