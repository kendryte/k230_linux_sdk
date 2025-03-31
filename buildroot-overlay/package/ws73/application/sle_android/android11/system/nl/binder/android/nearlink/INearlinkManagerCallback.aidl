// INearlinkManagerCallback.aidl
package android.nearlink;

import android.nearlink.INearlink;
/**
 * API for Communication between NearlinkAdapter and NearlinkManager
 *
 * {@hide}
 */
oneway interface INearlinkManagerCallback {
    void onNearlinkServiceUp(in INearlink nearlinkService);
    void onNearlinkServiceDown();
}