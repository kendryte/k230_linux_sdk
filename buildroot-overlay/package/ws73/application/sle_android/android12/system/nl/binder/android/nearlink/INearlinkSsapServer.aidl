package android.nearlink;

import android.os.ParcelUuid;
import android.nearlink.INearlinkSsapServerCallback;
import android.nearlink.NearlinkSsapService;

interface INearlinkSsapServer {
    boolean registerServer(in ParcelUuid appId, in INearlinkSsapServerCallback callback);
    boolean unregisterServer(in int serverId);
    boolean addService(in int serverId, in boolean isPrimary, in NearlinkSsapService service);
    boolean deleteAllServices(in int serverId);
    boolean sendResponse(in int serverId, in String address, in int requestId, in int status, in byte[] value);
    boolean notifyOrIndicate(in int serverId, in String address, in int handle, in int type, in byte[] value);
    boolean setInfo(in int serverId, in int mtuSize, in int version);
}
