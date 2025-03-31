package com.nearlink.demo.ssap;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkManager;
import android.nearlink.NearlinkSsapClient;
import android.nearlink.NearlinkSsapClientCallback;
import android.nearlink.NearlinkSsapDescriptor;
import android.nearlink.NearlinkSsapProperty;
import android.nearlink.NearlinkSsapService;
import android.os.Handler;
import android.os.Message;

import com.nearlink.demo.IChatCallback;

import static com.nearlink.demo.ssap.UUIDs.log;


public class SsapClient {
    static final String TAG = "SSAP-Client-APP";

    private static final List<SsapClient> clients = new ArrayList<>();

    public static SsapClient getInstance(Activity activity, String address, int instanceId) {
        for (SsapClient client : clients) {
            if (client.address.equals(address) && client.instanceId == instanceId) {
                return client;
            }
        }
        SsapClient instance = new SsapClient(activity, address, instanceId);
        clients.add(instance);
        return instance;
    }

    public static void destroy() {
        for (SsapClient client : clients) {
            if (client.mClient != null) {
                client.closeClient();
            }
        }
        clients.clear();
    }

    private final Activity activity;
    private final String address;
    private final int instanceId;
    private NearlinkSsapClient mClient;
    IChatCallback mCallback;
    private boolean connected = false;
    boolean useBinary = true;

    private SsapClient(Activity activity, String address, int instanceId) {
        this.activity = activity;
        this.address = address;
        this.instanceId = instanceId;
    }

    public void disconnect() {
        mHandler.obtainMessage(DISCONNECT).sendToTarget();
    }

    void closeClient() {
        Message msg = mHandler.obtainMessage(CLOSE_CLIENT);
        mHandler.sendMessage(msg);
    }

    public void useSsapClient(IChatCallback callback) {
        if (mClient != null) {
            if (!connected) {
                mHandler.obtainMessage(RECONNECT).sendToTarget();
            }
            return;
        }
        mCallback = callback;

        final NearlinkManager manager = (NearlinkManager) activity.getSystemService(Context.NEARLINK_SERVICE);
        NearlinkAdapter adapter = manager.getAdapter();
        NearlinkDevice device = adapter.getRemoteDevice(address);

        log(TAG, "----------------- Client register on <" + NearlinkAdapter.getDefaultAdapter().getName()
                + "," + NearlinkAdapter.getDefaultAdapter().getAddress() + "> -----------------");
        mClient = device.connectSsap(new NearlinkSsapClientCallback() {
            @Override
            public void onConnectionStateChange(NearlinkSsapClient client, int newState) {
                if (newState == NearlinkAdapter.STATE_CONNECTED) {
                    log(TAG, device.getAddress() + " connected.");
                    connected = true;
                    Message msg = mHandler.obtainMessage(DISCOVER_SERVICE);
                    mHandler.sendMessage(msg);
                } else if (newState == NearlinkAdapter.STATE_DISCONNECTED) {
                    log(TAG, device.getAddress() + " disconnected.");
                    connected = false;
                } else {
                    log(TAG, "???");
                }
            }

            @Override
            public void onServicesDiscovered(NearlinkSsapClient client, int status) {
                List<NearlinkSsapService> services = client.getServices();
                if (services == null || services.size() == 0) {
                    log(TAG, ">>>>>>>>>>>>>>>>> No service discovered <<<<<<<<<<<<<<<<<<<");
                    return;
                }

                StringBuilder sb = new StringBuilder("\n");
                for (NearlinkSsapService service : services) {
                    sb.append("service [uuid=").append(service.getUuid()).append(" handle=")
                            .append(service.getHandle()).append("]\n");
                    for (NearlinkSsapProperty property : service.getProperties()) {
                        sb.append("    property handle=").append(property.getHandle()).append(" uuid=")
                                .append(property.getUuid()).append(opInd2String(property.getOperateIndication()));
                        if (property.getDescriptors().size() > 0) {
                            sb.append(" | descriptors:");
                        }
                        for (NearlinkSsapDescriptor descriptor : property.getDescriptors()) {
                            sb.append(" ").append(descriptor.getType());
                        }
                        sb.append("\n");
                    }
                }
                log(TAG, "Service discovered: " + sb);
            }

            @Override
            public void onPropertyRead(NearlinkSsapClient client, NearlinkSsapProperty property, int status) {
                String value;
                if (useBinary) {
                    value = byteArray2HexString(property.getValue());
                } else {
                    value = new String(property.getValue());
                }
                log(TAG, "Read property(uuid=" + property.getUuid() + " handle=" + property.getHandle() + ") return: "
                        + value);
            }

            @Override
            public void onDescriptorRead(NearlinkSsapClient client, NearlinkSsapDescriptor descriptor, int status) {
                NearlinkSsapProperty property = descriptor.getProperty();
                String value;
                if (useBinary) {
                    value = byteArray2HexString(property.getValue());
                } else {
                    value = new String(property.getValue());
                }
                log(TAG,
                        "Read descriptor(uuid=" + property.getUuid() + " handle=" + property.getHandle() + " type=" + descriptor.getType() + ") return: " + value);
            }

            @Override
            public void onPropertyReadByUuid(NearlinkSsapClient client, List<NearlinkSsapProperty> properties,
                                             int status) {
                log(TAG, "Read property by uuid return: ");
                int i = 1;
                for (NearlinkSsapProperty property : properties) {
                    log(TAG, "[" + i++ + "] " + new String(property.getValue()));
                }
            }

            @Override
            public void onDescriptorReadByUuid(NearlinkSsapClient client, List<NearlinkSsapDescriptor> descriptors,
                                               int status) {
                log(TAG, "Read descriptor by uuid return: ");
                int i = 1;
                for (NearlinkSsapDescriptor descriptor : descriptors) {
                    log(TAG, "[" + i++ + "] " + new String(descriptor.getValue()));
                }
            }

            @Override
            public void onPropertyWrite(NearlinkSsapClient client, NearlinkSsapProperty property, int status) {
                log(TAG,
                        "Write property(uuid=" + property.getUuid() + " handle=" + property.getHandle() + " value: " + new String(property.getValue()) + ") return: " + status);
            }

            @Override
            public void onDescriptorWrite(NearlinkSsapClient client, NearlinkSsapDescriptor descriptor, int status) {
                NearlinkSsapProperty property = descriptor.getProperty();
                log(TAG,
                        "Write descriptor(uuid=" + property.getUuid() + " handle=" + property.getHandle() + " type=" + descriptor.getType() + ") return: " + status);
            }

            @Override
            public void onExchangeInfo(NearlinkSsapClient client, int mtu, int version, int status) {
                log(TAG, "onExchangeInfo - mtu=" + mtu + " version=" + version + " status" + status);
            }

            @Override
            public void onPropertyChanged(NearlinkSsapClient client, NearlinkSsapProperty property) {
                super.onPropertyChanged(client, property);
                String value;
                if (useBinary) {
                    value = byteArray2HexString(property.getValue());
                } else {
                    value = new String(property.getValue());
                }
                log(TAG, "Received property(uuid=" + property.getUuid() + " handle=" + property.getHandle() + ") " +
                        "change: value=" + value);
                mCallback.onMessageReceived(value);
            }

            @Override
            public void onReadRemoteRssi(NearlinkSsapClient client, int rssi, int status) {
                super.onReadRemoteRssi(client, rssi, status);
            }

            @Override
            public void onConnectionParameterUpdated(NearlinkSsapClient client, int interval, int latency,
                                                     int timeout, int status) {
                super.onConnectionParameterUpdated(client, interval, latency, timeout, status);
            }
        }, new Handler());
    }

    private static String byteArray2HexString(byte[] data) {
        if (data == null || data.length == 0) {
            return "[]";
        }
        StringBuilder value = new StringBuilder("[");
        for (byte b : data) {
            value.append(String.format("0x%02X, ", b));
        }
        value = new StringBuilder(value.substring(0, value.length() - 2));
        value.append("]");
        return value.toString();
    }

    private static String checkBit(int i, int b) {
        return (i & b) == b ? "true " : "false";
    }

    private static String opInd2String(int opInd) {
        return String.format(" read:%s, write_no_rsp:%s, write:%s, notify:%s, indicate:%s", checkBit(opInd,
                        NearlinkSsapProperty.PROPERTY_READ)
                , checkBit(opInd, NearlinkSsapProperty.PROPERTY_WRITE_NO_RSP)
                , checkBit(opInd, NearlinkSsapProperty.PROPERTY_WRITE)
                , checkBit(opInd, NearlinkSsapProperty.PROPERTY_NOTIFY)
                , checkBit(opInd, NearlinkSsapProperty.PROPERTY_INDICATE));
    }

    private NearlinkSsapProperty getProperty(int handle) {
        for (NearlinkSsapService service : mClient.getServices()) {
            if (service.getHandle() > handle) {
                continue;
            }
            for (NearlinkSsapProperty property : service.getProperties()) {
                if (property.getHandle() == handle) {
                    return property;
                }
            }
        }
        return null;
    }

    private static final int DISCONNECT = 1;
    private static final int RECONNECT = 2;
    private static final int DISCOVER_SERVICE = 3;
    private static final int SET_MTU_C = 4;
    private static final int WRITE_REQUEST = 5;
    private static final int READ_REQUEST = 6;
    private static final int UPDATE_CONN_PARAM = 7;
    private static final int GET_RSSI = 8;
    private static final int CLOSE_CLIENT = 9;

    Handler mHandler = new ClientHandler();

    private class InnerObj {
        String message;
        NearlinkSsapProperty property = null;
        NearlinkSsapDescriptor descriptor = null;

        InnerObj(String msg) {
            this.message = msg;
            int handle = -1;
            int index = message.indexOf(' ');
            if (index != -1) {
                try {
                    handle = Integer.parseInt(message.substring(0, index));
                    message = message.substring(index).trim();
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    handle = Integer.parseInt(message);
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
                message = null;
            }
            if (handle == -1) {
                log(TAG, "Invalid property handle");
                return;
            }
            property = getProperty(handle);
            if (property == null) {
                log(TAG, "property handle " + handle + " do not exists in discovery list.");
                return;
            }
            if (message == null || message.length() == 0) {
                return;
            }
            index = message.indexOf(' ');
            int type = -1;
            if (index != -1) {
                try {
                    type = Integer.parseInt(message.substring(0, index));
                    message = message.substring(index).trim();
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            } else {
                try {
                    type = Integer.parseInt(message);
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
            if (type != -1) {
                for (NearlinkSsapDescriptor d : property.getDescriptors()) {
                    if (d.getType() == type) {
                        descriptor = d;
                        break;
                    }
                }
            }
        }
    }

    private class ClientHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            try {
                if (msg.what == DISCONNECT) {
                    log(TAG, "----------------- Client disconnect -----------------");
                    mClient.disconnect();
                } else if (msg.what == RECONNECT) {
                    log(TAG, "----------------- Client reconnect -----------------");
                    if (!mClient.connect()) {
                        log(TAG, "reconnect failed");
                    }
                } else if (msg.what == DISCOVER_SERVICE) {
                    log(TAG, "----------------- Client discover services -----------------");
                    if (!mClient.discoverServices()) {
                        log(TAG, "discoverServices failed");
                    }
                } else if (msg.what == SET_MTU_C) {
                    log(TAG, "----------------- Client set mtu -----------------");
                    String message = (String) msg.obj;
                    String[] kv = message.split(" ");
                    if (kv.length >= 2) {
                        int mtu;
                        try {
                            mtu = Integer.parseInt(kv[1]);
                            if (!mClient.exchangeInfo(mtu, 2)) {
                                log(TAG, "exchangeInfo - mtu=" + mtu + " version=2 failed.");
                            }
                        } catch (Exception e) {
                            log(TAG, e.getMessage());
                        }
                    }
                } else if (msg.what == WRITE_REQUEST) {
                    log(TAG, "----------------- Client write property -----------------");
                    String message = (String) msg.obj;
                    if (message.startsWith("WNR")) {
                        message = message.substring(3);
                    } else {
                        message = message.substring(1);
                    }
                    InnerObj innerObj = new InnerObj(message);
                    if (innerObj.property == null) {
                        return;
                    }
                    byte[] value;
                    if (useBinary) {
                        String[] binaryArray = innerObj.message.split(",");
                        value = new byte[binaryArray.length];
                        for (int i = 0; i < value.length; i++) {
                            try {
                                value[i] = Byte.parseByte(binaryArray[i], 16);
                            } catch (Exception e) {
                                log(TAG, "Invalid value：" + binaryArray[i]);
                                return;
                            }
                        }
                    } else {
                        value = innerObj.message.getBytes(StandardCharsets.UTF_8);
                    }
                    if (innerObj.descriptor == null) {
                        innerObj.property.setValue(value);
                        if (!mClient.writeProperty(innerObj.property)) {
                            log(TAG, "writeProperty uuid=" + innerObj.property.getUuid() + " handle="
                                    + innerObj.property.getHandle() + " failed.");
                        }
                    } else {
                        innerObj.descriptor.setValue(value);
                        if (!mClient.writeDescriptor(innerObj.descriptor)) {
                            log(TAG, "writeDescriptor uuid=" + innerObj.property.getUuid() + " handle="
                                    + innerObj.property.getHandle() + " type= "
                                    + innerObj.descriptor.getType() + " failed.");
                        }
                    }
                } else if (msg.what == READ_REQUEST) {
                    String message = (String) msg.obj;
                    message = message.substring(1);
                    log(TAG, "----------------- Client read property -----------------");
                    InnerObj innerObj = new InnerObj(message);
                    if (innerObj.property == null) {
                        return;
                    }
                    if (innerObj.descriptor == null) {
                        if (!mClient.readProperty(innerObj.property)) {
                            log(TAG, "readProperty uuid=" + innerObj.property.getUuid() + " handle="
                                    + innerObj.property.getHandle() + " failed.");
                        }
                    } else {
                        if (!mClient.readDescriptor(innerObj.descriptor)) {
                            log(TAG, "readDescriptor uuid=" + innerObj.property.getUuid() + " handle="
                                    + innerObj.property.getHandle() + " type= "
                                    + innerObj.descriptor.getType() + " failed.");
                        }
                    }
                } else if (msg.what == UPDATE_CONN_PARAM) {
                    log(TAG, "----------------- Client update connect parameter -----------------");
                    if (!mClient.updateConnectionParameter(50, 200, 10, 200)) {
                        log(TAG, "update connect parameter 50,200,10,200 failed.");
                    }
                } else if (msg.what == GET_RSSI) {
                    log(TAG, "----------------- Client read remote rssi -----------------");
                    if (!mClient.readRemoteRssi()) {
                        log(TAG, "read remote rssi failed.");
                    }
                } else if (msg.what == CLOSE_CLIENT) {
                    log(TAG, "----------------- Client unregister -----------------");
                    mClient.close();
                    mClient = null;
                    mCallback = null;
                    log(TAG, "SSAP client closed.");
                }
            } catch (Exception e) {
                e.printStackTrace();
                log(TAG, e.getMessage());
            }
        }
    }


    public boolean sendMessage(String message) {
        if (message == null) {
            return false;
        }
        if ("binary".equals(message)) {
            useBinary = !useBinary;
            log(TAG, "show binary = " + useBinary);
            return true;
        }
        int what;
        if (message.equals("RSSI")) {
            what = GET_RSSI;
        } else if (message.startsWith("W")) {
            what = WRITE_REQUEST;
        } else if (message.startsWith("R")) {
            what = READ_REQUEST;
        } else if (message.startsWith("MTU ")) {
            what = SET_MTU_C;
        } else if (message.equalsIgnoreCase("UCP")) {
            what = UPDATE_CONN_PARAM;
        } else {
            log(TAG, "----------------- unknown command -----------------");
            return false;
        }
        if (mClient == null) {
            log(TAG, "----------------- client is null, do not send message -----------------");
            return false;
        }
        Message msg = mHandler.obtainMessage(what, message);
        mHandler.sendMessage(msg);
        return true;
    }

}
