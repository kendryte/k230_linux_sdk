// INearlinkEnableCallback.aidl
package android.nearlink;

// Declare any non-default types here with import statements

interface INearlinkEnableCallback {

    void onEnabled(int status);

    void onDisabled(int status);

}