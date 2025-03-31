package com.nearlink.demo.ssap;

import static com.nearlink.demo.ssap.UUIDs.PROPERTY_INDICATION;
import static com.nearlink.demo.ssap.UUIDs.PROPERTY_NOTIFY;
import static com.nearlink.demo.ssap.UUIDs.PROPERTY_NO_NOTIFY_INDICATION;
import static com.nearlink.demo.ssap.UUIDs.PROPERTY_READ_ONLY;
import static com.nearlink.demo.ssap.UUIDs.PROPERTY_READ_WRITE;
import static com.nearlink.demo.ssap.UUIDs.PROPERTY_READ_WRITE2;
import static com.nearlink.demo.ssap.UUIDs.PROPERTY_WRITE_ONLY;
import static com.nearlink.demo.ssap.UUIDs.SERVICE_1;
import static com.nearlink.demo.ssap.UUIDs.SERVICE_2;
import static com.nearlink.demo.ssap.UUIDs.getProperty;
import static com.nearlink.demo.ssap.UUIDs.getService;
import static com.nearlink.demo.ssap.UUIDs.log;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkErrorCode;
import android.nearlink.NearlinkManager;
import android.nearlink.NearlinkSsapDescriptor;
import android.nearlink.NearlinkSsapProperty;
import android.nearlink.NearlinkSsapServer;
import android.nearlink.NearlinkSsapServerCallback;
import android.nearlink.NearlinkSsapService;
import android.os.Handler;
import android.os.Message;

import com.nearlink.demo.IChatCallback;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Objects;

public class SsapServer {
    static final String TAG = "SSAP-Server-APP";

    private static final List<SsapServer> servers = new ArrayList<>();

    public static SsapServer getInstance(Activity activity, String address) {
        for (SsapServer instance : servers) {
            if (Objects.equals(instance.address, address)) {
                return instance;
            }
        }
        SsapServer instance = new SsapServer(activity, address);
        servers.add(instance);
        return instance;
    }

    public static void destroy() {
        for (SsapServer server : servers) {
            if (server.mServer != null) {
                server.mServer.close();
            }
        }
    }

    private final Activity activity;

    private final String address;

    IChatCallback mCallback;

    NearlinkSsapServer mServer;

    List<NearlinkSsapService> mServices = new ArrayList<>();

    NearlinkDevice remoteDevice;

    private SsapServer(Activity activity, String address) {
        this.activity = activity;
        this.address = address;
        this.remoteDevice = NearlinkAdapter.getDefaultAdapter().getRemoteDevice(address);
    }

    public void openOrCloseNearlinkSsapServer(IChatCallback callback) {
        if (mServer != null) {
            log(TAG, "----------------- Server clear all services -----------------");
            mServer.clearServices();
            return;
        }
        mCallback = callback;

        log(TAG, "----------------- Server register -----------------");
        final NearlinkManager manager = (NearlinkManager) activity.getSystemService(Context.NEARLINK_SERVICE);
        mServer = manager.openSsapServer(activity, new NearlinkSsapServerCallback() {
            @Override
            public void onServiceAdded(NearlinkSsapService service, int status) {
                super.onServiceAdded(service, status);
                log(TAG, "Service " + service.getUuid() + " added.");
                if (mServices.isEmpty()) {
                    log(TAG, "All services added, wait for first remote device send request and catch it.");
                } else {
                    Message msg = mHandler.obtainMessage(ADD_SERVICE, mServices.remove(0));
                    mHandler.sendMessage(msg);
                }
            }

            @Override
            public void onServicesCleared(NearlinkSsapServer server, int status) {
                super.onServicesCleared(server, status);
                log(TAG, "All SSAP services deleted.");
                mServer.close();
                mServer = null;
                mCallback = null;
                log(TAG, "SSAP server closed.");
            }

            @SuppressLint("DefaultLocale")
            @Override
            public void onPropertyReadRequest(NearlinkDevice device, int requestId, NearlinkSsapProperty property,
                                              boolean needResponse, boolean needAuthorize) {
                super.onPropertyReadRequest(device, requestId, property, needResponse, needAuthorize);
                String str = String.format(Locale.getDefault(), "Received read property(handle=%d uuid=%s) request.",
                        property.getHandle(), property.getUuid());
                log(TAG, str);
                mCallback.onMessageReceived(str);
            }

            @SuppressLint("DefaultLocale")
            @Override
            public void onDescriptorReadRequest(NearlinkDevice device, int requestId, NearlinkSsapDescriptor descriptor,
                                                boolean needResponse, boolean needAuthorize) {
                super.onDescriptorReadRequest(device, requestId, descriptor, needResponse, needAuthorize);
                String str = String.format(Locale.getDefault(),
                        "Received read descriptor(handle=%d uuid=%s type=%d) request.",
                        descriptor.getProperty().getHandle(), descriptor.getProperty().getUuid(), descriptor.getType());
                log(TAG, str);
            }

            @SuppressLint("DefaultLocale")
            @Override
            public void onPropertyWriteRequest(NearlinkDevice device, int requestId, NearlinkSsapProperty property,
                                               boolean needResponse, boolean needAuthorize, byte[] value) {
                super.onPropertyWriteRequest(device, requestId, property, needResponse, needAuthorize, value);
                String str = String.format(Locale.getDefault(),
                        "Received write property(handle=%d uuid=%s) request, value is: %s", property.getHandle(),
                        property.getUuid(), new String(value, Charset.defaultCharset()));
                log(TAG, str);
                mCallback.onMessageReceived(new String(value, Charset.defaultCharset()));
                if (needResponse) {
                    mServer.sendResponse(device, requestId, NearlinkErrorCode.ERRCODE_SLE_SUCCESS, value);
                }
            }

            @SuppressLint("DefaultLocale")
            @Override
            public void onDescriptorWriteRequest(NearlinkDevice device, int requestId,
                                                 NearlinkSsapDescriptor descriptor, boolean needResponse,
                                                 boolean needAuthorize, byte[] value) {
                super.onDescriptorWriteRequest(device, requestId, descriptor, needResponse, needAuthorize, value);
                String str = String.format(Locale.getDefault(),
                        "I received your write descriptor(handle=%d uuid=%s type=%d) request, you " + "want write: %s",
                        descriptor.getProperty().getHandle(), descriptor.getProperty().getUuid(), descriptor.getType(),
                        new String(value, Charset.defaultCharset()));
                log(TAG, str);
                if (needResponse) {
                    mServer.sendResponse(device, requestId, NearlinkErrorCode.ERRCODE_SLE_SUCCESS, value);
                }
            }

            @Override
            public void onMtuChanged(NearlinkDevice device, int mtuSize, int version) {
                super.onMtuChanged(device, mtuSize, version);
                log(TAG,
                        "onMtuChanged - device=" + device.getAddress() + " mtuSize=" + mtuSize + " version=" + version);
            }
        });
        mHandler.obtainMessage(INIT).sendToTarget();
    }

    public boolean sendMessage(String message) {
        int what = NOTIFY;
        if (message.startsWith("MTU ")) {
            what = SET_MTU_S;
        } else if (message.startsWith("E")) {
            what = NOTIFY_ERR;
        }
        Message msg = mHandler.obtainMessage(what, message);
        mHandler.sendMessage(msg);
        return true;
    }

    private static final int SET_MTU_S = 11;

    private static final int INIT = 12;

    private static final int ADD_SERVICE = 13;

    private static final int NOTIFY = 14;

    private static final int NOTIFY_ERR = 15;

    Handler mHandler = new ServerHandler();

    @SuppressLint("HandlerLeak")
    private class ServerHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == SET_MTU_S) {
                log(TAG, "----------------- Server set mtu -----------------");
                String message = (String) msg.obj;
                String[] kv = message.split(" ");
                if (kv.length >= 2) {
                    int mtu;
                    try {
                        mtu = Integer.parseInt(kv[1]);
                        mServer.setInfo(mtu, 1);
                    } catch (Exception e) {
                        log(TAG, e.getMessage());
                    }
                }
            } else if (msg.what == INIT) {
                log(TAG, "----------------- Init services -----------------");
                NearlinkSsapService service1 = new NearlinkSsapService(SERVICE_1);
                NearlinkSsapProperty propertyReadOnly = new NearlinkSsapProperty(PROPERTY_READ_ONLY, 0,
                        NearlinkSsapProperty.PROPERTY_READ);
                NearlinkSsapProperty propertyWriteOnly = new NearlinkSsapProperty(PROPERTY_WRITE_ONLY, 0,
                        NearlinkSsapProperty.PROPERTY_WRITE);
                NearlinkSsapProperty propertyReadWrite = new NearlinkSsapProperty(PROPERTY_READ_WRITE, 0,
                        NearlinkSsapProperty.PROPERTY_READ | NearlinkSsapProperty.PROPERTY_WRITE_NO_RSP);
                NearlinkSsapProperty propertyReadWrite2 = new NearlinkSsapProperty(PROPERTY_READ_WRITE2, 0,
                        NearlinkSsapProperty.PROPERTY_READ | NearlinkSsapProperty.PROPERTY_WRITE);
                NearlinkSsapDescriptor descriptor = new NearlinkSsapDescriptor(
                        NearlinkSsapDescriptor.TYPE_CLIENT_CONFIGURATION);
                descriptor.setValue(new byte[]{1});
                propertyReadWrite2.addDescriptor(descriptor);
                service1.addProperty(propertyReadOnly);
                service1.addProperty(propertyWriteOnly);
                service1.addProperty(propertyReadWrite);
                service1.addProperty(propertyReadWrite2);

                NearlinkSsapService service2 = new NearlinkSsapService(SERVICE_2);
                NearlinkSsapProperty propertyNotify = new NearlinkSsapProperty(PROPERTY_NOTIFY, 0,
                        NearlinkSsapProperty.PROPERTY_NOTIFY);
                NearlinkSsapProperty propertyIndication = new NearlinkSsapProperty(PROPERTY_INDICATION, 0,
                        NearlinkSsapProperty.PROPERTY_INDICATE);
                NearlinkSsapProperty propertyNo = new NearlinkSsapProperty(PROPERTY_NO_NOTIFY_INDICATION, 0, 0);
                service2.addProperty(propertyNotify);
                service2.addProperty(propertyIndication);
                service2.addProperty(propertyNo);

                mServices.clear();
                mServices.add(service1);
                mServices.add(service2);
                Message msg2 = mHandler.obtainMessage(ADD_SERVICE, mServices.remove(0));
                mHandler.sendMessage(msg2);
            } else if (msg.what == ADD_SERVICE) {
                log(TAG, "----------------- Server add services -----------------");
                NearlinkSsapService service = (NearlinkSsapService) msg.obj;
                mServer.addService(service, true);
            } else if (msg.what == NOTIFY) {
                String message = (String) msg.obj;
                boolean notify = true;
                if (message.startsWith("I")) {
                    notify = false;
                    message = message.substring(1);
                    log(TAG, "----------------- Server indicate -----------------");
                } else {
                    log(TAG, "----------------- Server notify -----------------");
                }
                if (notify) {
                    NearlinkSsapProperty propertyNotify = null;
                    try {
                        NearlinkSsapService service = getService(mServer.getServices(), SERVICE_2);
                        propertyNotify = getProperty(service.getProperties(), PROPERTY_NOTIFY);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    if (propertyNotify == null) {
                        log(TAG, "Can not find propertyNotify: PROPERTY_NOTIFY");
                        return;
                    }
                    propertyNotify.setValue(message.getBytes(StandardCharsets.UTF_8));
                    log(TAG,
                            "notifyPropertyChanged result: " + mServer.notifyPropertyChanged(remoteDevice, propertyNotify));
                } else {
                    NearlinkSsapProperty propertyIndication = null;
                    try {
                        NearlinkSsapService service = getService(mServer.getServices(), SERVICE_2);
                        propertyIndication = getProperty(service.getProperties(), PROPERTY_INDICATION);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    if (propertyIndication == null) {
                        log(TAG, "Can not find propertyIndication: PROPERTY_INDICATION");
                        return;
                    }
                    propertyIndication.setValue(message.getBytes(StandardCharsets.UTF_8));
                    log(TAG, "indicatePropertyChanged result: " + mServer.notifyPropertyChanged(remoteDevice,
                            propertyIndication));
                }
            } else if (msg.what == NOTIFY_ERR) {
                String message = (String) msg.obj;
                boolean notify = true;
                if (message.startsWith("EI")) {
                    notify = false;
                    message = message.substring(2);
                    log(TAG, "----------------- Server indicate no operate property -----------------");
                } else {
                    log(TAG, "----------------- Server notify no operate property -----------------");
                }
                if (notify) {
                    NearlinkSsapProperty propertyNotify = null;
                    try {
                        NearlinkSsapService service = getService(mServer.getServices(), SERVICE_2);
                        propertyNotify = getProperty(service.getProperties(), PROPERTY_NO_NOTIFY_INDICATION);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    if (propertyNotify == null) {
                        log(TAG, "Can not find propertyNotify: PROPERTY_NOTIFY");
                        return;
                    }
                    propertyNotify.setValue(message.getBytes(StandardCharsets.UTF_8));
                    log(TAG,
                            "notifyPropertyChanged result: " + mServer.notifyPropertyChanged(remoteDevice, propertyNotify));
                } else {
                    NearlinkSsapProperty propertyIndication = null;
                    try {
                        NearlinkSsapService service = getService(mServer.getServices(), SERVICE_2);
                        propertyIndication = getProperty(service.getProperties(), PROPERTY_NO_NOTIFY_INDICATION);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    if (propertyIndication == null) {
                        log(TAG, "Can not find propertyIndication: PROPERTY_INDICATION");
                        return;
                    }
                    propertyIndication.setValue(message.getBytes(StandardCharsets.UTF_8));
                    log(TAG, "indicatePropertyChanged result: " + mServer.notifyPropertyChanged(remoteDevice,
                            propertyIndication));
                }
            }
        }
    }
}
