// INearlinkDiscovery.aidl
package android.nearlink;

// Declare any non-default types here with import statements

import android.nearlink.NearlinkAnnounceParam;
import android.nearlink.NearlinkPublicData;
import android.nearlink.NearlinkSeekParams;
import android.nearlink.INearlinkAnnounceCallback;
import android.nearlink.INearlinkSeekCallback;
import android.nearlink.NearlinkSeekFilter;


interface INearlinkDiscovery {

    boolean startAnnounce(in int timeoutMillis, in NearlinkAnnounceParam nearlinkAnnounceParam, in NearlinkPublicData nearlinkAnnounceData, in NearlinkPublicData nearlinkSeekRespData,in INearlinkAnnounceCallback iNearlinkAnnounceCallback);

    boolean stopAnnounce(in INearlinkAnnounceCallback iNearlinkAnnounceCallback);

    boolean startSeek(in String seekId, in NearlinkSeekParams nearlinkSeekParams, in List<NearlinkSeekFilter> filters, in INearlinkSeekCallback iNearlinkSeekCallback);

    boolean stopSeek(in String seekId);

    boolean setTimeoutForDiscovery(in int timeout);

    long getDiscoveryEndMillis();

    int getAnnounceMode();

    boolean setAnnounceMode(in int mode,in int duration);

    boolean isDiscovering();

    boolean startDiscovery(in NearlinkSeekParams nearlinkSeekParams, in NearlinkAnnounceParam nearlinkAnnounceParam, in NearlinkPublicData nearlinkAnnounceData, in NearlinkPublicData nearlinkSeekRespData);

    boolean cancelDiscovery();

    int getTimeoutForDiscovery();

    void startSeekForIntent();

    void stopSeekForIntent();

}