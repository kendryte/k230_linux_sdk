// INearlinkManager.aidl
package android.nearlink;

import android.nearlink.INearlink;
import android.nearlink.INearlinkManagerCallback;
import android.nearlink.INearlinkProfileServiceConnection;
import android.nearlink.INearlinkStateChangeCallback;
import android.nearlink.INearlinkDiscovery;
import android.nearlink.INearlinkSsapClient;
import android.nearlink.INearlinkSsapServer;
import android.nearlink.INearlinkConnection;

/**
 * System private API for talking with the Nearlink service.
 *
 * {@hide}
 */
interface INearlinkManager {
    INearlink registerAdapter(in INearlinkManagerCallback callback);
    void unregisterAdapter(in INearlinkManagerCallback callback);
    void registerStateChangeCallback(in INearlinkStateChangeCallback callback);
    void unregisterStateChangeCallback(in INearlinkStateChangeCallback callback);
    boolean isEnabled();
    boolean enable(String packageName);
    boolean disable(String packageName, boolean persist);
    int getState();
    INearlinkDiscovery getNearlinkDiscovery();
    INearlinkConnection getNearlinkConnection();
    INearlinkSsapClient getNearlinkSsapClient();
    INearlinkSsapServer getNearlinkSsapServer();

    String getAddress();
    String getName();

    int updateNearlinkAppCount(IBinder b, boolean enable, String packageName);
    boolean isNearlinkAppPresent();
}