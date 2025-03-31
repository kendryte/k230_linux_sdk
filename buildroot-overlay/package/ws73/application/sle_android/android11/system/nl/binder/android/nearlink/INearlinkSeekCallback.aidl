// INearlinkSeekCallback.aidl
package android.nearlink;

// Declare any non-default types here with import statements
import android.nearlink.NearlinkSeekResultInfo;

interface INearlinkSeekCallback {

    void onSeekStarted(int status);

    void onSeekStopped(int status);

    void onSeekResult(in NearlinkSeekResultInfo nearlinkSeekResultInfo);

}