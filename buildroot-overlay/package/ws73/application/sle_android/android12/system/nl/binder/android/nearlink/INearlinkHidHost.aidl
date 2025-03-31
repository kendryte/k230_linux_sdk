/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.nearlink.NearlinkDevice;

/**
 * API for Nearlink HID service
 *
 * {@hide}
 */
interface INearlinkHidHost {
    // Public API
    boolean connect(in NearlinkDevice device);
    boolean disconnect(in NearlinkDevice device);
    List<NearlinkDevice> getConnectedDevices();
    List<NearlinkDevice> getDevicesMatchingConnectionStates(in int[] states);
    int getConnectionState(in NearlinkDevice device);
    boolean setPriority(in NearlinkDevice device, int priority);
    int getPriority(in NearlinkDevice device);
    /**
    * @hide
    */
    boolean setReport(in NearlinkDevice device, byte reportType, String report);
}
