/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import java.util.List;

/**
 * 星闪SSAP客户端回调抽象类
 *
 * @since 2023-12-01
 */
public abstract class NearlinkSsapClientCallback {

    public void onConnectionStateChange(NearlinkSsapClient client, int state) {
    }

    public void onServicesDiscovered(NearlinkSsapClient client, int status) {
    }

    public void onPropertyRead(NearlinkSsapClient client, NearlinkSsapProperty property, int status) {
    }

    public void onDescriptorRead(NearlinkSsapClient client, NearlinkSsapDescriptor descriptor, int status) {
    }

    public void onPropertyReadByUuid(NearlinkSsapClient client, List<NearlinkSsapProperty> properties, int status) {
    }

    public void onDescriptorReadByUuid(NearlinkSsapClient client, List<NearlinkSsapDescriptor> descriptors,
                                       int status) {
    }

    public void onPropertyWrite(NearlinkSsapClient client, NearlinkSsapProperty property, int status) {
    }

    public void onDescriptorWrite(NearlinkSsapClient client, NearlinkSsapDescriptor descriptor, int status) {
    }

    public void onExchangeInfo(NearlinkSsapClient client, int mtu, int version, int status) {
    }

    public void onPropertyChanged(NearlinkSsapClient client, NearlinkSsapProperty property) {
    }

    public void onReadRemoteRssi(NearlinkSsapClient client, int rssi, int status) {
    }

    public void onConnectionParameterUpdated(NearlinkSsapClient client, int interval, int latency, int timeout,
                                             int status) {
    }
}
