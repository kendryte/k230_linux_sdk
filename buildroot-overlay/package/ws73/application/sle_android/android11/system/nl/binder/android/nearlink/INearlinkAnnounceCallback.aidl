// INearlinkAnnounceCallback.aidl
package android.nearlink;

// Declare any non-default types here with import statements

interface INearlinkAnnounceCallback {

    void onAnnounceEnabled(int announceId, int status);

    void onAnnounceDisabled(int announceId, int status);

    void onAnnounceTerminaled(int announceId);

}