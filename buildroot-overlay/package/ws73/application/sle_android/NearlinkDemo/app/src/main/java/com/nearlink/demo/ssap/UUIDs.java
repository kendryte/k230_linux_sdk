package com.nearlink.demo.ssap;

import android.nearlink.NearlinkSsapProperty;
import android.nearlink.NearlinkSsapService;
import android.util.Log;

import com.nearlink.demo.UILog;

import java.util.List;
import java.util.UUID;

public class UUIDs {

    static final String TAGS = "SSAP-Server-APP";
    public static final UUID SERVICE_1 = UUID.fromString("00001111-2222-3333-4444-000000000001");
    public static final UUID SERVICE_2 = UUID.fromString("00001111-2222-3333-4444-000000000002");
    public static final UUID PROPERTY_READ_ONLY = UUID.fromString("00001111-2222-3333-4444-100000000001");
    public static final UUID PROPERTY_WRITE_ONLY = UUID.fromString("00001111-2222-3333-4444-100000000002");
    public static final UUID PROPERTY_READ_WRITE = UUID.fromString("00001111-2222-3333-4444-100000000003");
    public static final UUID PROPERTY_READ_WRITE2 = UUID.fromString("00001111-2222-3333-4444-100000000004");
    public static final UUID PROPERTY_NOTIFY = UUID.fromString("00001111-2222-3333-4444-200000000001");
    public static final UUID PROPERTY_INDICATION = UUID.fromString("00001111-2222-3333-4444-200000000002");
    public static final UUID PROPERTY_NO_NOTIFY_INDICATION = UUID.fromString("00001111-2222-3333-4444-200000000003");

    public static final UUID HID_SERVICE = UUID.fromString("37BEA880-FC70-11EA-B720-00000000060B");
    public static final UUID HID_PROP_REPORT_MAP = UUID.fromString("37BEA880-FC70-11EA-B720-000000001039");
    public static final UUID HID_PROP_CONTROL_POINT = UUID.fromString("37BEA880-FC70-11EA-B720-00000000103A");
    public static final UUID HID_PROP_REPORT_REF = UUID.fromString("37BEA880-FC70-11EA-B720-00000000103B");
    public static final UUID HID_PROP_SEND_REPORT = UUID.fromString("37BEA880-FC70-11EA-B720-00000000103C");

    public static NearlinkSsapService getService(List<NearlinkSsapService> services, UUID uuid) {
        for (NearlinkSsapService service : services) {
            if (service.getUuid().equals(uuid)) {
                return service;
            }
        }
        return null;
    }

    public static NearlinkSsapProperty getProperty(List<NearlinkSsapProperty> properties, UUID uuid)
            throws IllegalArgumentException {
        for (NearlinkSsapProperty property : properties) {
            if (property.getUuid().equals(uuid)) {
                return property;
            }
        }
        throw new IllegalArgumentException("Property(" + uuid + ") do not exists.");
    }

    public static NearlinkSsapProperty getProperty2(List<NearlinkSsapService> services, int handle)
            throws IllegalArgumentException {
        for (NearlinkSsapService service : services) {
            List<NearlinkSsapProperty> properties = service.getProperties();
            for (NearlinkSsapProperty property : properties) {
                if (property.getHandle() == handle) {
                    return property;
                }
            }
        }
        throw new IllegalArgumentException("Property(" + handle + ") do not exists.");
    }

    public static NearlinkSsapProperty getProperty(List<NearlinkSsapProperty> properties, int handle)
            throws IllegalArgumentException {
        for (NearlinkSsapProperty property : properties) {
            if (property.getHandle() == handle) {
                return property;
            }
        }
        throw new IllegalArgumentException("Property(" + handle + ") do not exists.");
    }

    public static void log(String tag, String msg) {
        Log.d(tag, msg);
        UILog.log(UILog.FLAG_OP, tag, msg);
    }

}
