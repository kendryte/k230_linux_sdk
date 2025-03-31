package com.nearlink.demo;

import java.util.Set;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAddress;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkHidHost;
import android.nearlink.NearlinkManager;
import android.nearlink.NearlinkProfile;
import android.os.Bundle;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.nearlink.demo.data.DeviceInfo;
import com.nearlink.demo.ssap.SsapClient;
import com.nearlink.demo.ssap.SsapServer;
import com.nearlink.demo.ssap.UUIDs;

public class OperationsActivity extends BaseActivity {
    public static final String TAG = "OperationsActivity";

    public static final int OP_CODE_LETF = 0x1;
    public static final int OP_CODE_RIGHT = 0x2;
    public static final int OP_CODE_UP = 0x3;
    public static final int OP_CODE_DOWN = 0x4;
    public static final int OP_CODE_BTN1 = 0x5;
    public static final int OP_CODE_BTN2 = 0x6;
    public static final int OP_CODE_BTN3 = 0x7;
    public static final int OP_CODE_BTN4 = 0x8;
    public static final int OP_CODE_BTN5 = 0x9;
    public static final int OP_CODE_BTN6 = 0xA;
    public static final int OP_CODE_BTN7 = 0xB;
    public static final int OP_CODE_BTN8 = 0xC;
    public static final int OP_CODE_BTN9 = 0xD;
    public static final int OP_CODE_BTN0 = 0xE;
    TextView deviceName;
    TextView address;
    NearlinkAdapter nlAdapter;
    NearlinkAddress nearlinkAddress;
    NearlinkDevice nearlinkDevice;
    int connectId;

    boolean asSsapClient = false;
    private int currentInstanceId;
    boolean asSsapServer = false;
    private DeviceInfo deviceInfo;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                Log.e(TAG, "Received intent with null action");
                return;
            }
            if (NearlinkAdapter.ACTION_CONNECTION_STATE_CHANGED.equals(action)) {
                NearlinkDevice device = intent.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
                int prevState = intent.getIntExtra(NearlinkAdapter.EXTRA_PREVIOUS_CONNECTION_STATE, -1);
                int state = intent.getIntExtra(NearlinkAdapter.EXTRA_CONNECTION_STATE, -1);
                int discReason = intent.getIntExtra(NearlinkAdapter.EXTRA_DISC_REASON, -1);
                UILog.log(TAG,
                        "ACTION_CONNECTION_STATE_CHANGED： address:" + device.getAddress()
                                + ", addressType:" + device.getAddressType()
                                + ", connState:" + String.format("0x%02x", state)
                                + ", discReason:" + String.format("0x%02x", discReason));
            } else if (NearlinkAdapter.ACTION_PAIR_STATE_CHANGED.equals(action)) {
                NearlinkDevice device = intent.getParcelableExtra(NearlinkDevice.EXTRA_DEVICE);
                int prevState = intent.getIntExtra(NearlinkDevice.EXTRA_PREVIOUS_PAIR_STATE, -1);
                int state = intent.getIntExtra(NearlinkDevice.EXTRA_PAIR_STATE, -1);
                UILog.log(TAG,
                        "ACTION_PAIR_STATE_CHANGED： address:" + device.getAddress() + ", addressType:" + device.getAddressType() + ", pairState:" + String.format("0x%02x", state));

            }
        }
    };

    @SuppressLint("MissingInflatedId")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.operation_activity);
        Intent intent = this.getIntent();
        deviceInfo = (DeviceInfo) intent.getSerializableExtra("deviceInfo");

        NearlinkManager nlService = (NearlinkManager) getSystemService(Context.NEARLINK_SERVICE);
        if (nlService != null) {
            nlAdapter = nlService.getAdapter();
        } else {
            Log.e(TAG, "[onCreate] nlService null");
        }

        mLogTxt = findViewById(R.id.etLog);

        initHid();

        IntentFilter filter = new IntentFilter();
        filter.addAction(NearlinkAdapter.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(NearlinkAdapter.ACTION_PAIR_STATE_CHANGED);
        registerReceiver(mReceiver, filter);

        Button btn_clear = findViewById(R.id.btn_clear);
        btn_clear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mLogTxt.setText("");
            }
        });

        LinearLayout content = findViewById(R.id.contentLayout);
        Button btnFold = findViewById(R.id.btnFold);
        btnFold.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int visibility = content.getVisibility();
                if (visibility == View.VISIBLE) {
                    content.setVisibility(View.GONE);
                    btnFold.setBackgroundResource(R.drawable.ic_arrow_fold);
                } else {
                    content.setVisibility(View.VISIBLE);
                    btnFold.setBackgroundResource(R.drawable.ic_arrow_expend);
                }
            }
        });

        content.setVisibility(View.GONE);
        btnFold.setBackgroundResource(R.drawable.ic_arrow_fold);

        deviceName = findViewById(R.id.deviceName);
        address = findViewById(R.id.sleAddress);
        if (deviceInfo != null) {
            nearlinkAddress = new NearlinkAddress(deviceInfo.address, deviceInfo.addrType);
            nearlinkDevice = nlAdapter.getRemoteDevice(nearlinkAddress);
            String aliasName = nearlinkDevice.getAliasName();
            if (aliasName != null && !TextUtils.isEmpty(aliasName))
                deviceName.setText(aliasName);
            else
                deviceName.setText(deviceInfo.name);
            address.setText(deviceInfo.address);
        }

        Button stopPairBtn = findViewById(R.id.btnStopPair);
        stopPairBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (nlAdapter != null) {
                    try {
                        boolean result = nearlinkDevice.removePair();
                        UILog.log(TAG, "[removePair] result:" + result);
                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "[removePair] e:" + e.toString());
                    }
                }
            }
        });

        findViewById(R.id.btnClearPair).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (nlAdapter != null) {
                    try {
                        boolean result = nlAdapter.getNearlinkConnection().removeAllPairs();
                        UILog.log(TAG, "[removeAllPairs] btnClearPair:" + result);
                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "[removeAllPairs] e:" + e.toString());
                    }
                }
            }
        });
        Button connectBtn = findViewById(R.id.btnConnect);
        connectBtn.setOnClickListener(v -> {
            if (nlAdapter != null) {
                try {
                    boolean result = nearlinkDevice.createConnect();
                    UILog.log(TAG, "[connect] result:" + result);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    Log.e(TAG, "[connect] e:" + e.toString());
                }
            }
        });

        findViewById(R.id.btnConnState).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (nlAdapter != null && nearlinkAddress != null && nearlinkDevice != null) {
                    try {
                        int connState = nearlinkDevice.getConnectionState();
                        UILog.log(TAG, "[getConnectionState] connState:" + String.format("0x%02x", connState));
                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "[getConnectionState] e:" + e.toString());
                    }
                }
            }
        });
        Button disconnectBtn = findViewById(R.id.btnDisconnect);
        disconnectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    boolean result = nearlinkDevice.disConnect();
                    UILog.log(TAG, "[disConnect] result:" + result);
                } catch (RemoteException e) {
                    e.printStackTrace();
                    Log.e(TAG, "[disConnect] e:" + e.toString());
                }
            }
        });

        Button stateBtn = findViewById(R.id.btnGetPairState);
        stateBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (nlAdapter != null && nearlinkAddress != null && nearlinkDevice != null) {
                    try {
                        int pairState = nearlinkDevice.getPairState();
                        UILog.log(TAG, "[getPairState] pairState:" + String.format("0x%02x", pairState));
                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "[getPairState] e:" + e.toString());
                    }
                }
            }
        });
        findViewById(R.id.btnIsPaired).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (nlAdapter != null && nearlinkAddress != null && nearlinkDevice != null) {
                    try {
                        boolean result = nearlinkDevice.isPaired();
                        UILog.log(TAG, "[isPaired] result:" + result);
                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "[isPaired] e:" + e.toString());
                    }
                }
            }
        });


        findViewById(R.id.btnUpdateParam).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
            }
        });

        findViewById(R.id.btnPairNum).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (nlAdapter != null && nearlinkAddress != null && nearlinkDevice != null) {
                    try {
                        int result = nlAdapter.getNearlinkConnection().getPairedDevicesNum();
                        UILog.log(TAG, "[getPairedDevicesNum] result:" + result);

                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "[getPairedDevicesNum] e:" + e.toString());
                    }
                }
            }
        });

        findViewById(R.id.btnPairDevices).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (nlAdapter != null && nearlinkAddress != null && nearlinkDevice != null) {
                    try {
                        Set<NearlinkDevice> list = nlAdapter.getPairedDevices();
                        UILog.log(TAG, "[getPairedDevices] list:" + list);
                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "[getPairedDevices] e:" + e.toString());
                    }
                }
            }
        });


        findViewById(R.id.btnIsConnected).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (nlAdapter != null && nearlinkAddress != null && nearlinkDevice != null) {
                    try {
                        boolean result = nearlinkDevice.isConnected();

                        UILog.log(TAG, "[isConnected] result:" + result);

                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e(TAG, "[isConnected] e:" + e.toString());
                    }
                }
            }
        });

        findViewById(R.id.btnSsapClient1).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                currentInstanceId = 1;
                useSsapClient();
            }
        });

        findViewById(R.id.btnSsapClient2).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                currentInstanceId = 2;
                useSsapClient();
            }
        });

        findViewById(R.id.btnSsapClient3).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                currentInstanceId = 3;
                useSsapClient();
            }
        });

        findViewById(R.id.btnDisconnectClient).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SsapClient.getInstance(OperationsActivity.this, address.getText().toString(), currentInstanceId).disconnect();
            }
        });

        findViewById(R.id.btnCloseAllSsapClient).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SsapClient.destroy();
            }
        });

        findViewById(R.id.btnSsapServer).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                asSsapServer = !asSsapServer;
                SsapServer.getInstance(OperationsActivity.this, address.getText().toString()).openOrCloseNearlinkSsapServer(new IChatCallback() {
                    @Override
                    public void onMessageReceived(String message) {
                        EditText input = findViewById(R.id.receiveTxt);
                        input.append(message + "\n");
                    }
                });
            }
        });

        // 定义下拉列表需要显示的文本数组
        String[] starArray = {getString(R.string.left_arrow), getString(R.string.right_arrow),
                getString(R.string.up_arrow), getString(R.string.down_arrow), "1", "2", "3", "4", "5", "6", "7", "8",
                "9"};

        // 声明一个下拉列表的数组适配器
        ArrayAdapter<String> starAdapter = new ArrayAdapter<String>(this, R.layout.item_select, starArray);
        // 从布局文件中获取名叫sp_dropdown的下拉框
        Spinner sp_dropdown = findViewById(R.id.spinner);
        // 设置下拉框的标题。对话框模式才显示标题，下拉模式不显示标题
        sp_dropdown.setPrompt("请选择");
        sp_dropdown.setAdapter(starAdapter); // 设置下拉框的数组适配器
        sp_dropdown.setSelection(0); // 设置下拉框默认显示第一项
        // 给下拉框设置选择监听器，一旦用户选中某一项，就触发监听器的onItemSelected方法
        sp_dropdown.setOnItemSelectedListener(new MySelectedListener());

        EditText input = findViewById(R.id.inputTxt);

        Button sendBtn = findViewById(R.id.btn_send);
        sendBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String strInput = input.getText().toString();
                if (asSsapClient) {
                    if (!SsapClient.getInstance(OperationsActivity.this, address.getText().toString(),
                            currentInstanceId).sendMessage(strInput)) {
                        showToast("发送失败，请查看日志");
                    }
                }
                if (asSsapServer) {
                    if (!SsapServer.getInstance(OperationsActivity.this, address.getText().toString()).sendMessage(strInput)) {
                        showToast("发送失败，请查看日志");
                    }
                }
            }
        });
    }

    private void useSsapClient() {
        asSsapClient = true;
        UUIDs.log("UserApk", "use client instance: " + currentInstanceId);
        SsapClient.getInstance(OperationsActivity.this, address.getText().toString(), currentInstanceId).useSsapClient(new IChatCallback() {
            @Override
            public void onMessageReceived(String message) {
                EditText input = findViewById(R.id.receiveTxt);
                input.append(message + "\n");
            }
        });
    }

    EditText mLogTxt;

    @Override
    protected EditText getLogTxt() {
        return mLogTxt;
    }

    @Override
    protected String getLogFlag() {
        return UILog.FLAG_OP;
    }

    private NearlinkHidHost hidHost;

    private void initHid() {
        NearlinkDevice device = nlAdapter.getRemoteDevice(deviceInfo.address);
        findViewById(R.id.btn_hid_connect).setOnClickListener(v -> {
            Log.e(TAG, "onClick: btn_hid_connect");
            nlAdapter.getProfileProxy(OperationsActivity.this, new NearlinkProfile.ServiceListener() {
                @Override
                public void onServiceConnected(int profile, NearlinkProfile nearlinkProfile) {
                    Log.e(TAG, "onServiceConnected: profile = NearlinkProfile.HID_HOST == " + profile);
                    hidHost = (NearlinkHidHost) nearlinkProfile;
                    boolean connect = hidHost.connect(device);
                    String connectResult = connect ? "Hid连接成功" : "Hid连接失败";
                    showToast(connectResult);
                    UILog.log(TAG, "Hid connect: [connect ret] " + connect);
                }

                @Override
                public void onServiceDisconnected(int profile) {
                    Log.e(TAG, "onServiceDisconnected: profile = " + profile);
                }
            }, NearlinkProfile.HID_HOST);
        });
        findViewById(R.id.btn_hid_disconnect).setOnClickListener(v -> {
            Log.e(TAG, "onClick: btn_hid_disconnect");
            if (hidHost != null) {
                boolean disconnect = hidHost.disconnect(device);
                String disconnectResult = disconnect ? "Hid断联成功" : "Hid断联失败";
                showToast(disconnectResult);
                UILog.log(TAG, "Hid disconnect: [disconnect ret] " + disconnect);
            } else {
                showToast("请先连接HID");
            }
        });
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        Log.e(TAG, "onKeyDown: event = " + event.getKeyCode() + " , keyCode = " + keyCode);
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        Log.e(TAG, "onKeyUp: event = " + event.getKeyCode() + " , keyCode = " + keyCode);
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        Log.e(TAG, "dispatchTouchEvent: event = " + ev);
        return super.dispatchTouchEvent(ev);
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent ev) {
        Log.e(TAG, "dispatchGenericMotionEvent: event = " + ev);
        return super.dispatchGenericMotionEvent(ev);
    }

    void onBtnClick(String tag) {
        Toast.makeText(this, tag, Toast.LENGTH_SHORT).show();
    }


    // 定义一个选择监听器，它实现了接口OnItemSelectedListener
    class MySelectedListener implements AdapterView.OnItemSelectedListener {

        @Override
        public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {

        }

        // 未选择时的处理方法，通常无需关注
        public void onNothingSelected(AdapterView<?> arg0) {
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        SsapClient.destroy();
        SsapServer.destroy();
        unregisterReceiver(mReceiver);
    }

    private void showToast(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }
}
