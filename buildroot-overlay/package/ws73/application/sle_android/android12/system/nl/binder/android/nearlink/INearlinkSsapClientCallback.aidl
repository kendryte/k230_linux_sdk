package android.nearlink;

import android.os.ParcelUuid;
import android.nearlink.NearlinkSsapReadByUuid;
import android.nearlink.NearlinkSsapService;

interface INearlinkSsapClientCallback {
    void onRegistered(in int clientId, in int status);
    void onConnectionStateChanged(in int clientId, in String address, in int addressType, in int state);
    void onSearchComplete(in String address, in List<NearlinkSsapService> services, in int status);
    void onReadCfm(in String address, in int handle, in int type, in byte[] data, in int status);
    void onReadByUuidComplete(in String address, in ParcelUuid uuid, in List<NearlinkSsapReadByUuid> data, in int status);
    void onWriteCfm(in String address, in int handle, in int type, in int status);
    void onInfoExchanged(in String address, in int mtuSize, in int version, in int status);
    void onNotification(in String address, in int handle, in int type, in boolean isNotify, in byte[] data, in int status);
    void onReadRemoteRssi(in String address, in int rssi, in int status);
    void onConnectionParameterUpdated(in String address, in int interval, in int latency, in int timeout, in int status);
}
