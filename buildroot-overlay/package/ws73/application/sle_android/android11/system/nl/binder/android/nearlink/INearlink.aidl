// INearlink.aidl
package android.nearlink;

import android.nearlink.INearlinkCallback;
import android.nearlink.NearlinkAddress;
import android.nearlink.NearlinkDevice;
import android.os.ParcelUuid;

// Declare any non-default types here with import statements

interface INearlink {

     int getState();

     boolean enable();

     boolean disable();

     boolean isEnabled();

     void registerCallback(in INearlinkCallback cb);

     void unregisterCallback(in INearlinkCallback cb);

     String getName();

     boolean setName(in String name);

     String getAddress();

     boolean setAddress(in String address);

     void onServiceUp();

     void onServiceDown();

     ParcelUuid[] getUuids();

     String getRemoteName(in NearlinkDevice device);

     String getRemoteAlias(in NearlinkDevice device);

     boolean setRemoteAlias(in NearlinkDevice device, String name);

     int getRemoteAppearence(in NearlinkDevice device);

}