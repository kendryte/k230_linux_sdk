// INearlinkStateChangeCallback.aidl
package android.nearlink;

/**
 * System private API for Nearlink state change callback.
 *
 * {@hide}
 */
oneway interface INearlinkStateChangeCallback {

    void onNearlinkStateChange(boolean on);
}