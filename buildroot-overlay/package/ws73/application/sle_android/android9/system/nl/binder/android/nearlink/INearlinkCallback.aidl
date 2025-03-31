// INearlinkCallback.aidl
package android.nearlink;

// Declare any non-default types here with import statements

interface INearlinkCallback {

     void onNearlinkStateChange(int prevState, int newState);

}