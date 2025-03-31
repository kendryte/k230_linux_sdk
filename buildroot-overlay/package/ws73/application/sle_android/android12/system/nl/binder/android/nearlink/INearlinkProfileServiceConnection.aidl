// INearlinkProfileServiceConnection.aidl
package android.nearlink;

import android.content.ComponentName;
import android.os.IBinder;

/**
 * Callback for nearlink profile connections.
 *
 * {@hide}
 */
oneway interface INearlinkProfileServiceConnection {
    void onServiceConnected(in ComponentName comp, in IBinder service);
    void onServiceDisconnected(in ComponentName comp);
}