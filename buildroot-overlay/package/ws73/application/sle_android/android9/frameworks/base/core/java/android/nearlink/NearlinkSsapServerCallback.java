/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

/**
 * 星闪SSAP服务端回调抽象类
 *
 * @since 2023-12-01
 */
public abstract class NearlinkSsapServerCallback {
    public void onServiceAdded(NearlinkSsapService service, int status) {
    }

    public void onServicesCleared(NearlinkSsapServer server, int status) {
    }

    public void onPropertyReadRequest(NearlinkDevice device, int requestId, NearlinkSsapProperty property, boolean needResponse, boolean needAuthorize) {
    }

    public void onDescriptorReadRequest(NearlinkDevice device, int requestId, NearlinkSsapDescriptor descriptor, boolean needResponse, boolean needAuthorize) {
    }

    public void onPropertyWriteRequest(NearlinkDevice device, int requestId, NearlinkSsapProperty property, boolean needResponse, boolean needAuthorize, byte[] value) {
    }

    public void onDescriptorWriteRequest(NearlinkDevice device, int requestId, NearlinkSsapDescriptor descriptor, boolean needResponse, boolean needAuthorize, byte[] value) {
    }

    public void onMtuChanged(NearlinkDevice device, int mtuSize, int version) {
    }
}
