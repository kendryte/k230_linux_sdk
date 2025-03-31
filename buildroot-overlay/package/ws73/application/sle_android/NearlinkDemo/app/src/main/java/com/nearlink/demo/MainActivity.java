package com.nearlink.demo;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAnnounceCallback;
import android.nearlink.NearlinkAnnounceSettings;
import android.nearlink.NearlinkAnnouncer;
import android.nearlink.NearlinkConstant;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkManager;
import android.nearlink.NearlinkPublicData;
import android.nearlink.NearlinkSeekCallback;
import android.nearlink.NearlinkSeekResultInfo;
import android.nearlink.NearlinkSeeker;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nearlink.demo.data.DeviceInfo;
import com.nearlink.demo.dd.DdTestEntry;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends BaseActivity {

    private static final String TAG = "MainActivity";

    DdTestEntry ddTestEntry;

    List<DeviceInfo> deviceInfoList = new ArrayList<>();
    DeviceListAdapter listAdapter = null;
    NearlinkAdapter nlAdapter;
    Switch swOpen;
    TextView mDeviceName;
    TextView mDeviceAddress;
    static MainActivity mainActivity;

    public static MainActivity getMainActivity() {
        return mainActivity;
    }

    private static final int REQ_CODE_ENABLE = 1;
    private static final int REQ_CODE_DISABLE = 2;

    void enableNearlink() {
        boolean res = nlAdapter.enable();
        UILog.log(getLogFlag(), "enable res: " + res);
    }

    void disableNearlink() {
        boolean res = nlAdapter.disable();
        UILog.log(getLogFlag(), "disable res: " + res);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mainActivity = this;
        mLogTxt = findViewById(R.id.etLog);
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

        NearlinkManager nlService = (NearlinkManager) MainActivity.this.getSystemService(Context.NEARLINK_SERVICE);
        if (nlService != null) {
            nlAdapter = nlService.getAdapter();
        } else {
            Log.e(TAG, "[onCreate] nlService null");
        }

        swOpen = findViewById(R.id.swOpen);
        swOpen.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (nlAdapter != null) {
                    boolean enabled = nlAdapter.isEnabled();
                    Log.e(TAG, "[onCheckedChanged] nlAdapter enabled:" + enabled);
                    if (isChecked) {
                        if (!enabled) {
                            boolean enable = nlAdapter.enable();
                            Log.e(TAG, "[onCheckedChanged] nlAdapter enable result:" + enable);
                            Toast.makeText(MainActivity.this, "打开星闪", Toast.LENGTH_SHORT).show();
                        }
                    } else {
                        if (enabled) {
                            boolean disable = nlAdapter.disable();
                            Log.e(TAG, "[onCheckedChanged] nlAdapter disable result:" + disable);
                            Toast.makeText(MainActivity.this, "关闭星闪", Toast.LENGTH_SHORT).show();
                        }
                    }
                } else {
                    Log.e(TAG, "[onCheckedChanged] nlService null");
                }
            }
        });

        Button btn_clear = findViewById(R.id.btn_clear);
        btn_clear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mLogTxt.setText("");
            }
        });

        mDeviceName = findViewById(R.id.sleName);
        mDeviceAddress = findViewById(R.id.sleAddress);

        LinearLayout renameLayout = findViewById(R.id.renameLayout);
        renameLayout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                createRenameDlg();
            }
        });

        LinearLayout readdressLayout = findViewById(R.id.readdressLayout);
        readdressLayout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                createReaddressDlg();
            }
        });

        Button btn_enablesle = findViewById(R.id.btn_enablesle);
        btn_enablesle.setOnClickListener((view) -> {
            enableNearlink();
        });

        Button btn_disablesle = findViewById(R.id.btn_disablesle);
        btn_disablesle.setOnClickListener((view) -> {
            disableNearlink();
        });

        Button startAnnounceBtn = findViewById(R.id.btn_startAnnounce);
        startAnnounceBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (nlAdapter != null) {
                    NearlinkAnnouncer nearlinkAnnouncer = nlAdapter.getNearlinkAnnouncer();
                    if (nearlinkAnnouncer != null) {
                        NearlinkAnnounceSettings settings = new NearlinkAnnounceSettings.Build().build();
                        NearlinkPublicData announceData = new NearlinkPublicData.Build().build();
                        NearlinkPublicData seekRespData = new NearlinkPublicData.Build().build();
                        NearlinkAnnounceCallback announceCallback = NearlinkAnnounceCallbackSingleInstance.getInstance();
                        nearlinkAnnouncer.startAnnounce(settings, announceData, seekRespData, announceCallback);
                    }

                }
            }

        });

        Button stopAnnounceBtn = findViewById(R.id.btn_stopAnnounce);
        stopAnnounceBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NearlinkAnnouncer nearlinkAnnouncer = nlAdapter.getNearlinkAnnouncer();
                NearlinkAnnounceCallback announceCallback = NearlinkAnnounceCallbackSingleInstance.getInstance();
                nearlinkAnnouncer.stopAnnounce(announceCallback);
            }
        });

        Button btn_startseek = findViewById(R.id.btn_startseek);
        btn_startseek.setOnClickListener((view) -> {
            Log.e(TAG, "[OnClick] btn_startseek");
            if (nlAdapter != null) {
                NearlinkSeeker nearlinkSeeker = nlAdapter.getNearlinkSeeker();
                nearlinkSeeker.startSeek(null, mSeekCallback);
            }
            Log.e(TAG, "[OnClick] btn_startseek end");
        });

        Button btn_stopseek = findViewById(R.id.btn_stopseek);
        btn_stopseek.setOnClickListener((view) -> {
            Log.e(TAG, "[OnClick] btn_stopseek");
            if (nlAdapter != null) {
                NearlinkSeeker nearlinkSeeker = nlAdapter.getNearlinkSeeker();
                nearlinkSeeker.stopSeek(mSeekCallback);
            }
            Log.e(TAG, "[OnClick] btn_stopseek end");
        });

        Button btn_getState = findViewById(R.id.btn_getState);
        btn_getState.setOnClickListener((view) -> {
            Log.e(TAG, "[OnClick] btn_stopseek");
            if (nlAdapter != null) {
                int state = nlAdapter.getState();
                UILog.log(getLogFlag(), NearlinkAdapter.nameForState(state));
            }
            Log.e(TAG, "[OnClick] btn_stopseek end");
        });

        Button btn_executeTestMethod = findViewById(R.id.btn_executeTestMethod);
        btn_executeTestMethod.setOnClickListener((view) -> {
            Log.e(TAG, "[OnClick] btn_executeTestMethod start");
            synchronized (this) {
                if (ddTestEntry == null) {
                    ddTestEntry = new DdTestEntry();
                    ddTestEntry.init();
                }
            }
            EditText moduleEditText = (EditText) findViewById(R.id.txt_moduleName);
            String moduleName = moduleEditText.getText().toString();
            EditText methodEditText = (EditText) findViewById(R.id.txt_testMethodName);
            String methodName = methodEditText.getText().toString();
            ddTestEntry.execute(moduleName, methodName);
            Log.e(TAG, "[OnClick] btn_executeTestMethod end");
        });

        Button btnRefresh = findViewById(R.id.btnRefresh);
        btnRefresh.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                deviceInfoList.clear();
                try {
                    List<NearlinkDevice> list = nlAdapter.getNearlinkConnection().getPairedDevices();
                    for (NearlinkDevice device : list) {
                        DeviceInfo dev = new DeviceInfo();
                        dev.name = device.getAliasName();
                        dev.address = device.getAddress();
                        dev.addrType = device.getAddressType();
                        dev.pairState = device.getPairState();
                        dev.connState = device.getConnectionState();
                        deviceInfoList.add(dev);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                listAdapter.notifyDataSetChanged();
            }
        });

        DeviceInfo ssapClient = new DeviceInfo();
        ssapClient.name = "Empty, please refresh and start seek";
        ssapClient.address = "09:00:00:00:00:00";
        ssapClient.addrType = 0;
        ssapClient.pairState = NearlinkConstant.SLE_PAIR_NONE;
        ssapClient.connState = NearlinkConstant.SLE_ACB_STATE_NONE;
        deviceInfoList.add(ssapClient);

        listAdapter = new DeviceListAdapter(this, R.layout.deviceitem_layout, deviceInfoList);
        ListView listView = findViewById(R.id.deviceList);
        listView.setAdapter(listAdapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Intent detail = new Intent(MainActivity.this, OperationsActivity.class);
                detail.putExtra("deviceInfo", deviceInfoList.get(position));
                MainActivity.this.startActivity(detail);
            }
        });
    }

    protected String getLogFlag() {
        return UILog.FLAG_MAIN;
    }

    EditText mLogTxt;

    @Override
    protected EditText getLogTxt() {
        return mLogTxt;
    }

    NearlinkSeekCallback mSeekCallback = new NearlinkSeekCallback() {

        @Override
        public void onSeekStartedSuccess() {
            UILog.log(getLogFlag(), "seek started [onSuccess]");
        }

        @Override
        public void onSeekStartedFailure(int i) {
            UILog.log(getLogFlag(), "seek started [onFailure] status = " + i);
        }

        @Override
        public void onSeekStoppedSuccess() {
            UILog.log(getLogFlag(), "seek stopped [onSuccess]");
        }

        @Override
        public void onSeekStoppedFailure(int i) {
            UILog.log(getLogFlag(), "seek stopped [onFailure] status = " + i);
        }

        @Override
        public void onResult(NearlinkSeekResultInfo nearlinkSeekResultInfo) {
            Log.e(TAG, "[onSeekResult] nearlinkSeekResultInfo:" + nearlinkSeekResultInfo);
            Log.e(TAG, "[onSeekResult] nearlinkSeekResultInfo getAddress:" + nearlinkSeekResultInfo.getAddress());
            Log.e(TAG, "[onSeekResult] nearlinkSeekResultInfo getRssi:" + nearlinkSeekResultInfo.getRssi());
            Log.e(TAG, "[onSeekResult] nearlinkSeekResultInfo getType:" + nearlinkSeekResultInfo.getEventType());

            int eventType = nearlinkSeekResultInfo.getEventType();
            if (NearlinkAdapter.SEEK_RSP_DATA_EVENT_TYPE != eventType) {
                return;
            }

            DeviceInfo deviceInfo = new DeviceInfo();
            deviceInfo.name = nearlinkSeekResultInfo.getDeviceLocalName();
            deviceInfo.address = nearlinkSeekResultInfo.getAddress();
            deviceInfo.rssi = nearlinkSeekResultInfo.getRssi();
            deviceInfo.pairState = nearlinkSeekResultInfo.getNearlinkDevice().getPairState();
            deviceInfo.connState = nearlinkSeekResultInfo.getNearlinkDevice().getConnectionState();
            Log.e(TAG, "[onScanResult] deviceInfo:" + deviceInfo);
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (deviceInfo != null) {
                        boolean find = false;
                        for (int i = 0; i < deviceInfoList.size(); i++) {
                            DeviceInfo device = deviceInfoList.get(i);
                            if (!TextUtils.isEmpty(device.address) && device.address.equals(deviceInfo.address)) {
                                deviceInfoList.set(i, deviceInfo);
                                find = true;
                                break;
                            }
                        }
                        if (!find) {
                            deviceInfoList.add(deviceInfo);
                        }
                        listAdapter.notifyDataSetChanged();
                        Log.e(TAG, "[onScanResult] end:");
                    }
                }
            });
        }
    };

    @Override
    protected void onResume() {
        super.onResume();
        Log.e(TAG, "[onResume] nlAdapter:" + nlAdapter);
        if (nlAdapter != null) {
            swOpen.setChecked(nlAdapter.isEnabled());
            String localName = nlAdapter.getName();
            Log.e(TAG, "[onResume] localName:" + localName);
            if (!TextUtils.isEmpty(localName)) {
                updateDeviceName(localName);
            }
            String address = nlAdapter.getAddress();
            Log.e(TAG, "[onResume] address:" + address);
            if (!TextUtils.isEmpty(address)) {
                updateDeviceAddress(address);
            }
        }
    }

    void updateDeviceName(String newName) {
        Log.e(TAG, "[updateDeviceName] newName:" + newName);
        mDeviceName.setText(newName);
    }

    void updateDeviceAddress(String address) {
        Log.e(TAG, "[updateDeviceAddress] address:" + address);
        mDeviceAddress.setText(address);
    }

    private AlertDialog mRenameAlertDialog;

    public void createRenameDlg() {
        if (nlAdapter != null) {
            String deviceName = nlAdapter.getName();
            AlertDialog.Builder builder = new AlertDialog.Builder(this)
                    .setTitle(R.string.rename_title)
                    .setView(createNameDialogView(deviceName))
                    .setPositiveButton(R.string.nearlink_rename_button, (dialog, which) -> {
                        String newName = mDeviceNameView.getText().toString();
                        updateDeviceName(newName);
                        nlAdapter.setName(newName);
                    })
                    .setNegativeButton(android.R.string.cancel, null);
            mRenameAlertDialog = builder.create();
            mRenameAlertDialog.setOnShowListener(d -> {
                if (mDeviceNameView != null && mDeviceNameView.requestFocus()) {
                    InputMethodManager imm = (InputMethodManager) MainActivity.this.getApplicationContext().getSystemService(
                            Context.INPUT_METHOD_SERVICE);
                    if (imm != null) {
                        imm.showSoftInput(mDeviceNameView, InputMethodManager.SHOW_IMPLICIT);
                    }
                }
            });
            mRenameAlertDialog.show();
            mOkButton = mRenameAlertDialog.getButton(DialogInterface.BUTTON_POSITIVE);
            mOkButton.setEnabled(mDeviceNameEdited);    // Ok button enabled after user edits
        }
    }

    EditText mDeviceNameView;

    private View createNameDialogView(String deviceName) {
        final LayoutInflater layoutInflater = (LayoutInflater) this
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view = layoutInflater.inflate(R.layout.dialog_edittext, null);
        mDeviceNameView = (EditText) view.findViewById(R.id.edittext);
        mDeviceNameView.setFilters(new InputFilter[]{
                new NearlinkLengthDeviceNameFilter()
        });
        mDeviceNameView.setText(deviceName);    // set initial value before adding listener
        if (!TextUtils.isEmpty(deviceName)) {
            mDeviceNameView.setSelection(deviceName.length());
        }
        mDeviceNameView.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                if (mDeviceNameUpdated) {
                    // Device name changed by code; disable Ok button until edited by user
                    mDeviceNameUpdated = false;
                    mOkButton.setEnabled(false);
                } else {
                    mDeviceNameEdited = true;
                    if (mOkButton != null) {
                        mOkButton.setEnabled(s.toString().trim().length() != 0);
                    }
                }
            }
        });
        setEditTextCursorPosition(mDeviceNameView);
        mDeviceNameView.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    updateDeviceName(v.getText().toString());
                    mRenameAlertDialog.dismiss();
                    mOkButton = null;
                    return true;    // action handled
                } else {
                    return false;   // not handled
                }
            }
        });
        return view;
    }

    // This flag is set when the name is updated by code, to distinguish from user changes
    private boolean mDeviceNameUpdated;

    // This flag is set when the user edits the name (preserved on rotation)
    private boolean mDeviceNameEdited;
    private Button mOkButton;

    //--------------------readdress--------------------------

    public static void setEditTextCursorPosition(EditText editText) {
        editText.setSelection(editText.getText().length());
    }


    private AlertDialog mReAddressAlertDialog;

    public void createReaddressDlg() {
        if (nlAdapter != null) {
            String deviceAddress = nlAdapter.getAddress();
            AlertDialog.Builder builder = new AlertDialog.Builder(this)
                    .setTitle(R.string.readdress_title)
                    .setView(createAddressDialogView(deviceAddress))
                    .setPositiveButton(R.string.nearlink_readdress_button, (dialog, which) -> {
                        String newAddress = mDeviceAddressView.getText().toString();
                        updateDeviceAddress(newAddress);
                    })
                    .setNegativeButton(android.R.string.cancel, null);
            mReAddressAlertDialog = builder.create();
            mReAddressAlertDialog.setOnShowListener(d -> {
                if (mDeviceAddressView != null && mDeviceAddressView.requestFocus()) {
                    InputMethodManager imm = (InputMethodManager) MainActivity.this.getApplicationContext().getSystemService(
                            Context.INPUT_METHOD_SERVICE);
                    if (imm != null) {
                        imm.showSoftInput(mDeviceAddressView, InputMethodManager.SHOW_IMPLICIT);
                    }
                }
            });
            mReAddressAlertDialog.show();
            mAddressOkButton = mReAddressAlertDialog.getButton(DialogInterface.BUTTON_POSITIVE);
            mAddressOkButton.setEnabled(mDeviceAddressEdited);    // Ok button enabled after user edits
        }
    }

    EditText mDeviceAddressView;
    // This flag is set when the name is updated by code, to distinguish from user changes
    private boolean mDeviceAddressUpdated;

    // This flag is set when the user edits the name (preserved on rotation)
    private boolean mDeviceAddressEdited;
    private Button mAddressOkButton;

    private View createAddressDialogView(String deviceAddress) {
        final LayoutInflater layoutInflater = (LayoutInflater) this
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view = layoutInflater.inflate(R.layout.dialog_edittext, null);
        mDeviceAddressView = (EditText) view.findViewById(R.id.edittext);
        mDeviceAddressView.setFilters(new InputFilter[]{
                new NearlinkLengthDeviceNameFilter()
        });
        mDeviceAddressView.setText(deviceAddress);    // set initial value before adding listener
        if (!TextUtils.isEmpty(deviceAddress)) {
            mDeviceAddressView.setSelection(deviceAddress.length());
        }
        mDeviceAddressView.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {

            }

            @Override
            public void afterTextChanged(Editable s) {
                if (mDeviceAddressUpdated) {
                    // Device name changed by code; disable Ok button until edited by user
                    mDeviceAddressUpdated = false;
                    mAddressOkButton.setEnabled(false);
                } else {
                    mDeviceAddressEdited = true;
                    if (mAddressOkButton != null) {
                        mAddressOkButton.setEnabled(s.toString().trim().length() != 0);
                    }
                }
            }
        });
        setEditTextCursorPosition(mDeviceAddressView);
        mDeviceAddressView.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    updateDeviceAddress(v.getText().toString());
                    mRenameAlertDialog.dismiss();
                    mAddressOkButton = null;
                    return true;    // action handled
                } else {
                    return false;   // not handled
                }
            }
        });
        return view;
    }

    private static class NearlinkAnnounceCallbackSingleInstance {

        private static volatile NearlinkAnnounceCallback instance;

        private NearlinkAnnounceCallbackSingleInstance() {
        }

        public static NearlinkAnnounceCallback getInstance() {
            if (instance == null) {
                synchronized (NearlinkAnnounceCallbackSingleInstance.class) {
                    if (instance == null) {
                        instance = new NearlinkAnnounceCallback() {

                            @Override
                            public void onStartedSuccess() {
                                super.onStartedSuccess();
                                UILog.log(UILog.FLAG_MAIN, "startAnnounce [onSuccess]");
                            }

                            @Override
                            public void onStartedFailure(int errorCode) {
                                super.onStartedFailure(errorCode);
                                UILog.log(UILog.FLAG_MAIN, "startAnnounce [onFailure]");
                            }

                            @Override
                            public void onStoppedSuccess() {
                                super.onStoppedSuccess();
                                UILog.log(UILog.FLAG_MAIN, "stopAnnounce [onSuccess]");
                            }

                            @Override
                            public void onStoppedFailure(int errorCode) {
                                super.onStoppedFailure(errorCode);
                                UILog.log(UILog.FLAG_MAIN, "stopAnnounce [onFailure]");
                            }

                            @Override
                            public void onTerminaled() {
                                super.onTerminaled();
                                Log.e(TAG, "onTerminaled");
                                UILog.log(UILog.FLAG_MAIN, "announce [onTerminaled]");
                            }
                        };
                    }
                }
            }
            return instance;
        }

    }

}