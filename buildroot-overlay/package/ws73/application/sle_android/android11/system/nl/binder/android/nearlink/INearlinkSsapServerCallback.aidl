package android.nearlink;

import android.nearlink.NearlinkSsapService;

interface INearlinkSsapServerCallback {
    void onServerRegistered(in int serverId, in int status);
    void onServiceAdded(in NearlinkSsapService service, in int status);
    void onAllServiceDeleted(in int status);
    void onServerReadRequest(in String address, in int requestId, in int handle, in int type, in boolean needResponse, in boolean needAuthorize, in int status);
    void onServerWriteRequest(in String address, in int requestId, in int handle, in int type, in boolean needResponse, in boolean needAuthorize, in byte[] data, in int status);
    void onMtuChanged(in String address, in int mtuSize, in int version, in int status);
}
