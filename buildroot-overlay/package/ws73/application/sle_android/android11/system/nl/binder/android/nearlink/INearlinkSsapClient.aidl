package android.nearlink;

import android.os.ParcelUuid;
import android.nearlink.INearlinkSsapClientCallback;

interface INearlinkSsapClient {
    boolean registerClient(in ParcelUuid appUuid, in String address, in INearlinkSsapClientCallback callback);
    boolean connect(in int clientId, in String address, in int addressType);
    void disconnect(in int clientId, in String address, in int addressType);
    boolean unregisterClient(in int clientId);
    boolean discoverServices(in int clientId, in String address);
    boolean readRequestByUuid(in int clientId, in String address, in int type, in int startHandle, in int endHandle, in ParcelUuid uuid);
    boolean readRequest(in int clientId, in String address, in int handle, in int type);
    boolean writeRequest(in int clientId, in String address, in int handle, in int type, in boolean needResponse, in byte[] data);
    boolean exchangeInfoRequest(in int clientId, in String address, in int mtuSize, in int version);
    boolean readRemoteRssi(in int clientId, in String address);
    boolean connectionParameterUpdate(int clientId, String address, int minInterval, int maxInterval,
                                          int maxLatency, int supervisionTimeout);
    boolean setTxDataLength(in int clientId, in int tx_octets);
}
