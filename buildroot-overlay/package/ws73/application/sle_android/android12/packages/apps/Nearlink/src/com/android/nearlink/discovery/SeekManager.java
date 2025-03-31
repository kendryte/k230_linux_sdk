/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery;

import android.content.Intent;
import android.nearlink.INearlinkSeekCallback;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAppearance;
import android.nearlink.NearlinkDevice;
import android.nearlink.NearlinkSeekCallbackConstants;
import android.nearlink.NearlinkSeekFilter;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;

import android.nearlink.NearlinkSeekParams;
import android.nearlink.NearlinkSeekResultInfo;
import android.os.RemoteException;
import android.util.Log;

import com.android.nearlink.connection.PubTools;
import com.android.nearlink.discovery.publicdata.read.EasyPublicData;
import com.android.nearlink.discovery.publicdata.read.PublicDataReadProcessor;
import com.android.nearlink.discovery.publicdata.read.impl.callback.EasyPublicDataReadCallback;
import com.android.nearlink.discovery.publicdata.read.impl.parser.StandardSubPublicDataParser;
import com.android.nearlink.nlservice.AdapterService;
import com.android.nearlink.nlservice.DdUtils;
import com.android.nearlink.nlservice.RemoteDevices;

import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.Optional;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.UUID;

/**
 * Seek管理器
 *
 * @since 2023-12-04
 */
public class SeekManager {

    private static final String TAG = DiscoveryServiceConfig.TAG_PREFIX + "SeekManager";

    private static final boolean LOG_RESULT_INFO_LOG = false;
    private static final String LOG_RESULT_TAG = TAG + "Seek-Result:";

    private static final int LOG_FREQ = 100;

    private static final int SEEK_START_NEARLINK_SCAN_MSG_TYPE = 0;
    private static final int SEEK_STOP_NEARLINK_SCAN_MSG_TYPE = 1;
    /**
     * seek 自动停止时间处理类型
     */
    private static final int SEEK_TIMEOUT_MS_MSG_TYPE = 3;
    private static final int START_REMOVE_EXPIRED_DEVICE_MSG_TYPE = 4;
    private static final int STOP_REMOVE_EXPIRED_DEVICE_MSG_TYPE = 5;
    /**
     * start jni启动回调超时失败处理类型
     */
    private static final int SEEK_START_JNI_CALLBACK_TIMEOUT_TYPE = 6;
    /**
     * stop jni启动回调超时失败处理类型
     */
    private static final int SEEK_STOP_JNI_CALLBACK_TIMEOUT_TYPE = 7;

    private static final int SCAN_ON = 0;
    private static final int SCAN_OFF = 1;
    private static final int SCAN_TURING_ON = 2;
    private static final int SCAN_TURING_OFF = 3;
    private static final int SCAN_START_ERROR = 4;
    private static final int SCAN_STOP_ERROR = 5;

    private static final boolean IS_OPEN_IMMEDIATE_SEEK_RESULT_REPORT = false;
    private static final long DEFAULT_SEEK_RESULT_REPORT_WINDOW_MILLIS = 500;
    /**
     * 默认设备过期认定时间
     */
    private static final long DEVICE_DEFAULT_EXPIRED_MS = 5*1000;

    /**
     * 设备过期检查周期
     */
    private static final long CHECK_EXPIRED_DEVICE_PERIOD_MS = 5*1000;
    /**
     * SEEK自动停止时间
     */
    private static final long SEEK_TIME_OUT_MILLIS = 60*60*1000;

    private static final long TURNING_WAIT_MILLIS = 2000;

    private static final long SEEK_JNI_CALLBACK_TIMEOUT_MILLIS = 1500;

    private static final int STOP_SEEK_NATIVE_SEEK_CLIENT_NUM = 1;

    private volatile int seekManagerStatus = SCAN_OFF;

    private Object seekStatusObject = new Object();

    private Timer mTimer;

    private TimerTask mTimerTask;

    private volatile SeekHandler mHandler;

    private AdapterService mAdapterService;

    private DiscoveryService mDiscoveryService;

    private PublicDataReadProcessor mPublicDataProcessor;

    private Map<String, SeekClient> seekClientMap;

    private volatile String seekStopNativeId;

    static {
        classInitNative();
    }

    public SeekManager(AdapterService adapterService, DiscoveryService discoveryService) {
        this.mAdapterService = adapterService;
        this.mDiscoveryService = discoveryService;
        this.seekClientMap = Collections.synchronizedMap(new HashMap<>());
        // 标准协议
        this.mPublicDataProcessor = PublicDataReadProcessor.getPublicDataProcessor(new StandardSubPublicDataParser());
        HandlerThread thread = new HandlerThread("SeekManager");
        thread.start();
        this.mHandler = new SeekHandler(thread.getLooper());
        initNative();
    }

    public void cleanUp(){

        this.mAdapterService = null;
        this.mDiscoveryService = null;

        if(this.seekClientMap != null){
            this.seekClientMap.clear();
            this.seekClientMap = null;
        }

        if(this.mPublicDataProcessor != null){
            this.mPublicDataProcessor.cleanUp();
            this.mPublicDataProcessor = null;
        }

        if (mHandler != null) {
            // Shut down the thread
            mHandler.removeCallbacksAndMessages(null);
            Looper looper = mHandler.getLooper();
            if (looper != null) {
                looper.quitSafely();
            }
            mHandler = null;
        }

        if(mTimer != null){
            mTimer = null;
        }

        if(mTimerTask != null){
            mTimerTask = null;
        }

        cleanupNative();

    }


    void startSeekForDiscovery(NearlinkSeekParams nearlinkSeekParams){

        Log.e(TAG, "start discovery seek");
        SeekClient seekClient = new SeekClient(null, SeekClient.START_DISCOVERY_TYPE, nearlinkSeekParams, null, null);
        sendSeekClientMessage(SEEK_START_NEARLINK_SCAN_MSG_TYPE, null, seekClient);

    }

    void startSeekForCallback(SeekClient seekClient){

        INearlinkSeekCallback iNearlinkSeekCallback = seekClient.getiNearlinkSeekCallback();
        if(iNearlinkSeekCallback == null){
            Log.e(TAG, "ERROR!!! start seek, but seekCallback is null");
            return;
        }

        NearlinkSeekDeathRecipient deathRecipient = new NearlinkSeekDeathRecipient(seekClient);
        IBinder binder = toBinder(seekClient.getiNearlinkSeekCallback());
        try {
            binder.linkToDeath(deathRecipient, 0);
        } catch (RemoteException e) {
            throw new IllegalArgumentException("Can't link to seek's death");
        }

        seekClient.setDeathRecipient(deathRecipient);

        Log.e(TAG, "start seek");

        synchronized (seekClientMap){
            seekClientMap.put(seekClient.getSeekId(), seekClient);
        }

        Log.e(TAG, "send msg SEEK_START_SEEK_SCAN,type= " + seekClient.getType());
        sendSeekClientMessage(SEEK_START_NEARLINK_SCAN_MSG_TYPE, null, seekClient);

    }

    public void startSeekForIntent(){
        SeekClient seekClientForIntent = SingleSeekClient.getInstance();
        synchronized (seekClientMap){
            if(seekClientMap.get(seekClientForIntent.getSeekId()) != null){
                Log.e(TAG, "already start seek for intent");
                return;
            }
            seekClientMap.put(seekClientForIntent.getSeekId(), seekClientForIntent);
        }

        Log.e(TAG, "send msg SEEK_START_SEEK_SCAN,type= " + seekClientForIntent.getType());
        sendSeekClientMessage(SEEK_START_NEARLINK_SCAN_MSG_TYPE, null, seekClientForIntent);
    }

    public void stopSeekForDiscovery(){
        SeekClient seekClient = new SeekClient(null, SeekClient.START_DISCOVERY_TYPE,
                null, null, null);
        sendSeekClientMessage(SEEK_STOP_NEARLINK_SCAN_MSG_TYPE,
                null, seekClient);
    }

    void stopSeek(String seekId){
        Log.e(TAG, "stop seek");

        Optional<SeekClient> optional = preStopSeek(seekId);
        if(!optional.isPresent()){
            return;
        }

        SeekClient seekClient = optional.get();
        Log.e(TAG, "send msg SEEK_STOP_SEEK_SCAN, type= " + seekClient.getType());
        sendSeekClientMessage(SEEK_STOP_NEARLINK_SCAN_MSG_TYPE,
                null, seekClient);
    }

    public void stopSeekForIntent(){
        Log.e(TAG, "service stopSeekForIntent");
        SeekClient seekClientForIntent = SingleSeekClient.getInstance();
        sendSeekClientMessage(SEEK_STOP_NEARLINK_SCAN_MSG_TYPE,
                null, seekClientForIntent);
    }

    private Optional<SeekClient> preStopSeek(String seekId){

        SeekClient seekClient = null;
        synchronized (seekClientMap){
            seekClient = seekClientMap.get(seekId);
        }

        if(seekClient == null){
            Log.e(TAG, "seekId = " + seekId + "but not in seekClientMap");
            return Optional.empty();
        }

        try {
            IBinder binder = toBinder(seekClient.getiNearlinkSeekCallback());
            binder.unlinkToDeath(seekClient.getDeathRecipient(), 0);
        } catch (NoSuchElementException e) {
            Log.e(TAG, "Unable to unlink deathRecipient for seek id " + seekId);
        }

        return Optional.ofNullable(seekClient);
    }

    private void sendSeekClientMessage(int what, Long delayMillis, SeekClient seekClient) {

        final SeekHandler handler = mHandler;

        if (handler == null) {
            Log.e(TAG, "sendMessage: mHandler is null.");
            return;
        }
        Message message = new Message();
        message.what = what;
        message.obj = seekClient;

        if(delayMillis == null || delayMillis <= 0){
            handler.sendMessage(message);
            return;
        }

        handler.sendMessageDelayed(message, delayMillis);
    }

    private void sendJniCallbackDelayedMessage(int what, Long delayMillis){
        final SeekHandler handler = mHandler;
        if (handler == null) {
            Log.e(TAG, "sendMessage: mHandler is null.");
            return;
        }
        handler.sendEmptyMessageDelayed(what, delayMillis);
    }



    private void sendStartRemoveExpiredDeviceMessage(){
        final SeekHandler handler = mHandler;
        if (handler == null) {
            Log.e(TAG, "sendMessage: mHandler is null.");
            return;
        }
        handler.sendEmptyMessage(START_REMOVE_EXPIRED_DEVICE_MSG_TYPE);
    }


    private void sendStopRemoveExpiredDeviceMessage(){
        final SeekHandler handler = mHandler;
        if (handler == null) {
            Log.e(TAG, "sendMessage: mHandler is null.");
            return;
        }
        handler.sendEmptyMessage(STOP_REMOVE_EXPIRED_DEVICE_MSG_TYPE);
    }


    private class SeekHandler extends Handler {

        SeekHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.e(TAG, "handleMessage,msg.what=" + msg.what);
            switch (msg.what) {
                case SEEK_START_NEARLINK_SCAN_MSG_TYPE:
                    handleStartScan((SeekClient) msg.obj);
                    break;

                case SEEK_STOP_NEARLINK_SCAN_MSG_TYPE:

                    handleStopScan((SeekClient) msg.obj);
                    break;

                case SEEK_TIMEOUT_MS_MSG_TYPE:
                    handleStopScanTimeout((SeekClient) msg.obj);
                    break;

                case START_REMOVE_EXPIRED_DEVICE_MSG_TYPE:

                    handleStartRemoveExpiredDevice();
                    break;

                case STOP_REMOVE_EXPIRED_DEVICE_MSG_TYPE:

                    handleStopRemoveExpiredDevice();
                    break;

                case SEEK_START_JNI_CALLBACK_TIMEOUT_TYPE:
                    handleJinStartCallbackTimeout();
                    break;

                case SEEK_STOP_JNI_CALLBACK_TIMEOUT_TYPE:
                    handleJinStopCallbackTimeout();
                    break;

                default:
                    // Shouldn't happen.
                    Log.e(TAG, "received an unkown message : " + msg.what);
            }
        }
    }

    void handleStartScan(SeekClient seekClient){
        Log.e(TAG, "do handleStartScan, type= " +seekClient.getType());

        if(isStartSeekCallbackType(seekClient)){
            Log.e(TAG, "sendSeekTimeoutClientMessage");
            sendSeekClientMessage(SEEK_TIMEOUT_MS_MSG_TYPE,
                    SEEK_TIME_OUT_MILLIS,
                    seekClient);
        }

        if(seekManagerStatus == SCAN_ON){
            Log.e(TAG, "SEEK IS ALREADY ON !!!");

            /**
             * 如果是startSeek类型
             */
            if(isStartSeekCallbackType(seekClient)){
                try{
                    seekClient.setSeekStartCallbackStatus(SeekClient.CALLED);
                    seekClient.getiNearlinkSeekCallback()
                            .onSeekStarted(NearlinkSeekCallbackConstants.SEEK_SUCCESS);
                }catch (RemoteException e){
                    Log.e(TAG, "handleStartScan onSeekStarted() - Callback # failed (" + e + ")");
                }
            }

            /**
             * 如果是startDiscovery类型
             */
            if(seekClient.getType() == SeekClient.START_DISCOVERY_TYPE){
                mDiscoveryService.synMarkSeekOnDiscovery();
            }

            return;
        }

        if(seekManagerStatus == SCAN_TURING_ON){
            Log.e(TAG, "SEEK IS TURNING ON BUSY !!!");
            if(isStartSeekCallbackType(seekClient)){
                try{
                    seekClient.getiNearlinkSeekCallback()
                            .onSeekStarted(NearlinkSeekCallbackConstants.SEEK_STARTING_BUSY);
                }catch (RemoteException e){
                    Log.e(TAG, "handleStartScan onSeekStarted() - Callback # failed (" + e + ")");
                }
            }
            return;
        }

        if(seekManagerStatus == SCAN_TURING_OFF){
            Log.e(TAG, "try to start seek, but at seek turing off");
            // 如果是SCAN_TURING_OFF的状态，说明协议栈正在关闭中，等待1s后再处理
            sendSeekClientMessage(SEEK_START_NEARLINK_SCAN_MSG_TYPE, TURNING_WAIT_MILLIS, seekClient);
            return;
        }



        NearlinkSeekParams params = seekClient.nearlinkSeekParams;
        setSeekParamNative(params.getOwnAddrType(), params.getFilterDuplicates(), params.getSeekFilterPolicy(),
                params.getSeekPhys(), params.getSeekType(), params.getSeekInterval(), params.getSeekWindow());
        int res = startSeekNative();

        if(res != 0){
            Log.e(TAG, "startSeekNative error res = " + res);
            updateSeekManagerStatus(SCAN_START_ERROR);
            mDiscoveryService.synMarkSeekOnDiscoveryError();
            return;
        }
        updateSeekManagerStatus(SCAN_TURING_ON);
        sendJniCallbackDelayedMessage(SEEK_START_JNI_CALLBACK_TIMEOUT_TYPE,
                SEEK_JNI_CALLBACK_TIMEOUT_MILLIS);
    }

    void handleStopScan(SeekClient seekClient) {
        Log.e(TAG, "do handleStopScan, type= " + seekClient.getType());
        if(seekManagerStatus == SCAN_OFF){
            Log.e(TAG, "SEEK IS ALREADY OFF !!!");
            removeSeekClientAndNotifySeekStop(seekClient);
            return;
        }

        if(seekManagerStatus == SCAN_TURING_OFF){
            Log.e(TAG, "SEEK IS TURING OFF BUSY !!!");
            if(seekClient.getType() == SeekClient.START_SEEK_TYPE &&
                    seekClient.getiNearlinkSeekCallback() != null){
                try{
                    seekClient.getiNearlinkSeekCallback()
                            .onSeekStopped(NearlinkSeekCallbackConstants.SEEK_STOPPING_BUSY);
                }catch (RemoteException e){
                    Log.e(TAG, "handleStopScan onSeekStopped() - Callback # failed (" + e + ")");
                }
            }
            return;
        }

        if(seekManagerStatus == SCAN_TURING_ON){
            Log.e(TAG, "try to stop seek, but at seek turing on");
            // 如果是SCAN_TURING_ON的状态，说明协议栈正在打开中，等待1s后再处理
            sendSeekClientMessage(SEEK_STOP_NEARLINK_SCAN_MSG_TYPE, TURNING_WAIT_MILLIS, seekClient);
            return;
        }

        /**
         * 检查最后指定native Stop
         */
        boolean isNeedStopSeekNative = false;
        synchronized (seekClientMap){
            if(seekClient.getType() == SeekClient.START_DISCOVERY_TYPE
                    && seekClientMap.size() == 0){
                isNeedStopSeekNative =true;
            }else if(seekClient.getType() != SeekClient.START_DISCOVERY_TYPE
                    && seekClientMap.size() <= STOP_SEEK_NATIVE_SEEK_CLIENT_NUM
                    && mDiscoveryService.isDiscoverySeekOffOrTuringOff()){
                isNeedStopSeekNative =true;
            }
        }

        if(!isNeedStopSeekNative){
            removeSeekClientAndNotifySeekStop(seekClient);
        }else {
            if (seekClient.getType() != SeekClient.START_DISCOVERY_TYPE) {
                seekStopNativeId = seekClient.getSeekId();
            }
            
            int res = stopSeekNative();
            if(res != 0){
                Log.e(TAG, "stopSeekNative error res: " + res);
                updateSeekManagerStatus(SCAN_STOP_ERROR);
                mDiscoveryService.synMarkSeekOffDiscoveryError();
                return;
            }


            updateSeekManagerStatus(SCAN_TURING_OFF);

            sendJniCallbackDelayedMessage(SEEK_STOP_JNI_CALLBACK_TIMEOUT_TYPE,
                    SEEK_JNI_CALLBACK_TIMEOUT_MILLIS);
        }
    }

    void handleStopScanTimeout(SeekClient seekClient){
        Log.e(TAG, "handleStopScanTimeout seekId = " + seekClient.seekId);
        Optional<SeekClient> optional = preStopSeek(seekClient.seekId);
        if(!optional.isPresent()){
            Log.e(TAG, "handleStopScanTimeout, seekId = " + seekClient.seekId + " not exist");
            return;
        }
        handleStopScan(optional.get());
    }

    private void handleStartRemoveExpiredDevice(){
        Log.e(TAG, "handleStartRemoveExpiredDevice");
        startRemoveExpiredDeviceTimerTask();
    }

    private void handleStopRemoveExpiredDevice(){
        Log.e(TAG, "handleStopRemoveExpiredDevice");
        stopRemoveExpiredDeviceTimerTask();
    }

    private void removeSeekClientAndNotifySeekStop(SeekClient seekClient){

        synchronized (seekClientMap){
            seekClientMap.remove(seekClient.getSeekId());
        }

        if(seekClient.getType() == SeekClient.START_SEEK_TYPE &&
                seekClient.getiNearlinkSeekCallback() != null){
            try{
                seekClient.getiNearlinkSeekCallback()
                        .onSeekStopped(NearlinkSeekCallbackConstants.SEEK_SUCCESS);
            }catch (RemoteException e){
                Log.e(TAG, "handleStopScan onSeekStopped() - Callback # failed (" + e + ")");
            }
        }

        if(seekClient.getType() == SeekClient.START_DISCOVERY_TYPE){
            mDiscoveryService.synMarkSeekOffDiscovery();
        }
    }

    private void handleJinStartCallbackTimeout(){
        Log.e(TAG, "jni start callback timeout handle");
        updateSeekManagerStatus(SCAN_START_ERROR);
        mDiscoveryService.synMarkSeekOnDiscoveryError();
    }

    private void handleJinStopCallbackTimeout(){
        Log.e(TAG, "jni stop callback timeout handle");
        updateSeekManagerStatus(SCAN_STOP_ERROR);
        mDiscoveryService.synMarkSeekOffDiscoveryError();
    }

    /**
     * 停止和清理定时器
     */
    private void stopRemoveExpiredDeviceTimerTask(){

        if(mTimer != null){
            mTimer.cancel();
            mTimer = null;
        }

        if(mTimerTask != null){
            mTimerTask.cancel();
            mTimerTask = null;
        }
    }

    /**
     * 启动剔除过去设备任务
     */
    private void startRemoveExpiredDeviceTimerTask(){

        if(mTimer == null){
            mTimer = new Timer();
        }

        if(mTimerTask == null){
            mTimerTask = new TimerTask() {
                @Override
                public void run() {
                    try{
                        if(mAdapterService == null){
                            Log.e(TAG, "mAdapterService is null, not to execute removeExpiredDevice");
                            return;
                        }
                        mAdapterService.getRemoteDevices()
                                .removeExpiredDevice(System.currentTimeMillis(), DEVICE_DEFAULT_EXPIRED_MS);
                    }catch (Exception e){
                        Log.e(TAG, "mTimerTask failed (" + e + ")");
                    }
                }
            };
        }

        if(mTimer != null && mTimerTask != null){
            Log.e(TAG, "mTimer schedule");
            // execute
            mTimer.schedule(mTimerTask, 0,  CHECK_EXPIRED_DEVICE_PERIOD_MS);
        }
    }

    private boolean isStartSeekCallbackType(SeekClient seekClient){
        return ((seekClient.getType() == SeekClient.START_SEEK_TYPE) &&
                (seekClient.iNearlinkSeekCallback != null));
    }


    public void onStartSeekCallback(int status){

        Log.e(TAG, "onStartSeekCallback, status is" + status);
        mHandler.removeMessages(SEEK_START_JNI_CALLBACK_TIMEOUT_TYPE);

        if(status == 0){
            // 发送启动剔除过期设备任务的消息
            sendStartRemoveExpiredDeviceMessage();
            updateSeekManagerStatus(SCAN_ON);
            // 通知startDiscovery的Seek状态
            mDiscoveryService.synMarkSeekOnDiscovery();
        }else {
            updateSeekManagerStatus(SCAN_START_ERROR);
            mDiscoveryService.synMarkSeekOnDiscoveryError();
        }

        synchronized (seekClientMap){
            if(isNeedNotify()){
                Set<Map.Entry<String, SeekClient>> sets = seekClientMap.entrySet();
                Iterator<Map.Entry<String, SeekClient>> it = sets.iterator();
                while (it.hasNext()){
                    Map.Entry<String, SeekClient>  entry =  it.next();
                    SeekClient seekClient = entry.getValue();
                    if(seekClient.getType() != SeekClient.START_SEEK_TYPE
                            || seekClient.getiNearlinkSeekCallback() == null){
                        continue;
                    }
                    try{
                        Log.e(TAG, "onSeekStarted execute seekId = " + seekClient.seekId);
                        if(seekClient.seekStartCallbackStatus == SeekClient.CALLED){
                            continue;
                        }
                        seekClient.getiNearlinkSeekCallback().onSeekStarted(status);
                    }catch (RemoteException e){
                        Log.e(TAG, "onSeekStarted() - Callback # failed (" + e + ")");
                    }
                }
            }
        }

    }

    public void onSeekDisableCallback(int status){

        Log.e(TAG, "onSeekDisableCallback, status is" + status);

        mHandler.removeMessages(SEEK_STOP_JNI_CALLBACK_TIMEOUT_TYPE);

        if(status == 0){
            // 发送停止剔除过期设备任务的消息
            sendStopRemoveExpiredDeviceMessage();
            updateSeekManagerStatus(SCAN_OFF);
            mDiscoveryService.synMarkSeekOffDiscovery();
        }else {
            updateSeekManagerStatus(SCAN_STOP_ERROR);
            mDiscoveryService.synMarkSeekOffDiscoveryError();
        }


        synchronized (seekClientMap){
            if(isNeedNotify()){
                SeekClient seekClient = seekClientMap.remove(seekStopNativeId);
                seekStopNativeId = null;
                if (seekClient == null) {
                    return;
                }
                if (seekClient.getType() == SeekClient.START_SEEK_TYPE
                            && seekClient.getiNearlinkSeekCallback() != null) {
                    try{
                        Log.e(TAG, "onSeekStopped execute");
                        seekClient.getiNearlinkSeekCallback().onSeekStopped(status);
                    }catch (RemoteException e){
                        Log.e(TAG, "onSeekStopped() - Callback # failed (" + e + ")");
                    }
                }
            }
        }
    }

    private boolean isNeedNotify(){
        return !seekClientMap.isEmpty();
    }

    private String bytesToAddressHex(byte[] bytes){
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < bytes.length ; i++){
            sb.append(":");
            String hex = Integer.toHexString(bytes[i] & 0xFF).toUpperCase();
            if(hex.length() < 2){
                sb.append(0);
            }
            sb.append(hex);
        }
        return sb.substring(1);
    }

    public void onSeekResultCallback(int eventType, int addrType, byte[] addr,
                                     int directAddrType, byte[] directAddr,
                                     int rssi, int dataStatus, int dataLength, byte[] data){
        boolean isNeedNotifyFlag = isNeedNotify();
        if( !isNeedNotifyFlag && !mDiscoveryService.isDiscoverySeekOn()){
            Log.e(TAG, "no one at seek");
            return;
        }

        String javaAddr = bytesToAddressHex(addr);
        String javaDirectAddr = bytesToAddressHex(directAddr);

        logResultInfo("eventType "+eventType +"; addrType: "+addrType
                +" ; addr:"+ PubTools.macPrint(javaAddr)+" ;dataLength:" + dataLength);

        RemoteDevices remoteDevices = mAdapterService.getRemoteDevices();
        remoteDevices.deviceRefreshUpdateTime(javaAddr);
        if(!LOG_RESULT_INFO_LOG && remoteDevices.isNeedLog(javaAddr, LOG_FREQ)){
            StringBuilder sb = new StringBuilder();
            sb.append("eventType = ").append(eventType).append(" addr = ")
                    .append(PubTools.macPrint(javaAddr));
            Log.e(TAG, sb.toString());
        }

        NearlinkDevice nearlinkDevice = NearlinkAdapter.getDefaultAdapter().getRemoteDevice(javaAddr);
        final NearlinkSeekResultInfo nearlinkSeekResultInfo = new NearlinkSeekResultInfo(eventType, javaAddr, addrType, javaDirectAddr, directAddrType,
                rssi, dataStatus, dataLength, data, "",  nearlinkDevice);


        if(!IS_OPEN_IMMEDIATE_SEEK_RESULT_REPORT){
            // 窗口时间内只能上报一次
            boolean isSeekResultReportFlag = mAdapterService.getRemoteDevices()
                    .isSeekResultReport(System.currentTimeMillis(),
                            DEFAULT_SEEK_RESULT_REPORT_WINDOW_MILLIS,
                            eventType,
                            nearlinkDevice);
            if(!isSeekResultReportFlag){
                return;
            }
        }

        EasyPublicData easyPublicData = new EasyPublicData();
        mPublicDataProcessor.doProcess(javaAddr, dataLength, data, new EasyPublicDataReadCallback(easyPublicData));

        // 更新RemoteDevices中的DeviceProperties
        mAdapterService.getRemoteDevices().devicePropertyChangedCallback(javaAddr, rssi, easyPublicData);

        updateSeekResult(nearlinkSeekResultInfo, easyPublicData);

        // 如果存在callback
        if(isNeedNotifyFlag){
            notifyNearlinkSeekResultInfo(nearlinkSeekResultInfo);
        }

        // 处于discovery状态且SEEK_RSP_DATA_EVENT_TYPE类型，则把数据发送广播
        if(mDiscoveryService.isDiscoverySeekOn()){

            int appearance = getAppearanceFromDeviceProperties(nearlinkDevice);
            String remoteAlias = getAliasFromDeviceProperties(nearlinkDevice);
            String remoteDeviceName = tryGetNameOrAddress(javaAddr, nearlinkDevice, easyPublicData);
            Intent intent = getIntent(NearlinkDevice.ACTION_FOUND,
                    remoteDeviceName, remoteAlias,
                    rssi, appearance, nearlinkDevice);
            logResultInfo("send broadcast to settings addr = " +  PubTools.macPrint(javaAddr)
                    + " name = " + nearlinkSeekResultInfo.getDeviceLocalName()
                    + " appearance = " +Integer.toHexString(appearance));
            mDiscoveryService.sendBroadcastMultiplePermissions(intent, new String[]{
                    AdapterService.NEARLINK_PERM, android.Manifest.permission.ACCESS_COARSE_LOCATION});

        }
    }

    /**
     * 最大努力获取名称，没有用address代替
     * @param javaAddr
     * @param nearlinkDevice
     * @param easyPublicData
     * @return
     */
    private String tryGetNameOrAddress(String javaAddr, NearlinkDevice nearlinkDevice, EasyPublicData easyPublicData){
        String deviceLocalName = easyPublicData.deviceLocalName;
        if(!DdUtils.isBlank(deviceLocalName)){
            return deviceLocalName;
        }
        Optional<String> optional = getDeviceNameFromDeviceProperties(nearlinkDevice);
        if(!optional.isPresent()){
            return javaAddr;
        }
        String cacheName = optional.get();
        if(DdUtils.isBlank(cacheName)){
            return javaAddr;
        }
        return cacheName;
    }

    private void notifyNearlinkSeekResultInfo(NearlinkSeekResultInfo nearlinkSeekResultInfo){
        Set<Map.Entry<String, SeekClient>> sets = seekClientMap.entrySet();
        Iterator<Map.Entry<String, SeekClient>> it = sets.iterator();
        while (it.hasNext()){
            Map.Entry<String, SeekClient>  entry =  it.next();
            SeekClient seekClient = entry.getValue();
            if(seekClient.getType() == SeekClient.START_SEEK_INTENT){

                int appearance = getAppearanceFromDeviceProperties(nearlinkSeekResultInfo.getNearlinkDevice());
                String alias = getAliasFromDeviceProperties(nearlinkSeekResultInfo.getNearlinkDevice());

                Intent intent = getIntent(NearlinkDevice.ACTION_DETECTED,
                        nearlinkSeekResultInfo.getDeviceLocalName(),
                        alias, nearlinkSeekResultInfo.getRssi(), appearance,
                        nearlinkSeekResultInfo.getNearlinkDevice());

                logResultInfo("send broadcast intent  addr = " + PubTools.macPrint(nearlinkSeekResultInfo.getAddress())
                        + " name = " + nearlinkSeekResultInfo.getDeviceLocalName()
                        + " appearance = " + Integer.toHexString(appearance));
                mDiscoveryService.sendBroadcastMultiplePermissions(intent, new String[]{
                        AdapterService.NEARLINK_PERM, android.Manifest.permission.ACCESS_COARSE_LOCATION});

            }else if (seekClient.getType() == SeekClient.START_SEEK_TYPE
                    && seekClient.getiNearlinkSeekCallback() != null){
                if(!matchesFilters(seekClient, nearlinkSeekResultInfo)){
                    continue;
                }
                INearlinkSeekCallback iNearlinkSeekCallback = seekClient.getiNearlinkSeekCallback();
                try{
                    iNearlinkSeekCallback.onSeekResult(nearlinkSeekResultInfo);
                }catch (RemoteException e){
                    Log.e(TAG, "onSeekResult() - Callback # failed (" + e + ")");
                }
            }
        }
    }

    private Intent getIntent(String action, String remoteDeviceName, String remoteDeviceAlias,
                             int rssi, int appearance, NearlinkDevice nearlinkDevice){
        Intent intent = new Intent(action);
        intent.putExtra(NearlinkDevice.EXTRA_NAME, remoteDeviceName);
        intent.putExtra(NearlinkDevice.EXTRA_ALIAS, remoteDeviceAlias);
        intent.putExtra(NearlinkDevice.EXTRA_RSSI, rssi);
        intent.putExtra(NearlinkDevice.EXTRA_APPEARANCE, appearance);
        intent.putExtra(NearlinkDevice.EXTRA_DEVICE, nearlinkDevice);
        return intent;
    }

    private void updateSeekResult(NearlinkSeekResultInfo seekResultInfo, EasyPublicData easyPublicData){
        if(!DdUtils.isBlank(easyPublicData.deviceLocalName)){
            seekResultInfo.setDeviceLocalName(easyPublicData.deviceLocalName);
        }
    }

    private int getAppearanceFromDeviceProperties(NearlinkDevice nearlinkDevice){
        RemoteDevices.DeviceProperties deviceProp = mAdapterService.getRemoteDevices().getDeviceProperties(nearlinkDevice);

        if(deviceProp == null){
            return NearlinkAppearance.UNKNOWN;
        }

        return deviceProp.getAppearance();
    }

    private String getAliasFromDeviceProperties(NearlinkDevice nearlinkDevice){
        RemoteDevices.DeviceProperties deviceProp = mAdapterService.getRemoteDevices().getDeviceProperties(nearlinkDevice);
        if(deviceProp == null){
            return null;
        }
        return deviceProp.getAlias();
    }

    private Optional<String> getDeviceNameFromDeviceProperties(NearlinkDevice nearlinkDevice){
        RemoteDevices.DeviceProperties deviceProp = mAdapterService.getRemoteDevices().getDeviceProperties(nearlinkDevice);

        if(deviceProp == null){
            return Optional.empty();
        }

        return Optional.ofNullable(deviceProp.getName());
    }

    private IBinder toBinder(INearlinkSeekCallback e) {
        return e.asBinder();
    }

    private void updateSeekManagerStatus(int status){
        Log.e(TAG, "seekManagerStatus pre: " + seekManagerStatus + " new: " + status);
        synchronized (seekStatusObject){
            seekManagerStatus = status;
        }
    }

    class NearlinkSeekDeathRecipient implements IBinder.DeathRecipient {

        public SeekClient seekClient;

        public NearlinkSeekDeathRecipient(SeekClient seekClient) {
            this.seekClient = seekClient;
        }

        @Override
        public void binderDied() {
            synchronized (seekClientMap){
                seekClientMap.remove(seekClient.getSeekId());
            }
            handleStopScan(seekClient);
        }
    }

    private static class SingleSeekClient {

        private static volatile SeekClient instance;

        private SingleSeekClient(){}

        public static synchronized SeekClient getInstance(){
            if(instance == null){
                synchronized (SingleSeekClient.class){
                    if(instance == null){
                        String uuid = UUID.randomUUID().toString();
                        instance = new SeekClient(uuid, SeekClient.START_SEEK_INTENT,
                                NearlinkSeekParams.builder().build(), null, null);
                    }
                }
            }
            return instance;
        }
    }


    private boolean matchesFilters(SeekClient seekClient, NearlinkSeekResultInfo nearlinkSeekResultInfo){
        if (seekClient.filters == null || seekClient.filters.isEmpty()) {
            return true;
        }
        for (NearlinkSeekFilter filter : seekClient.filters) {
            if (filter.matches(nearlinkSeekResultInfo)) {
                return true;
            }
        }
        return false;
    }

    private void logResultInfo(String msg){
        if(LOG_RESULT_INFO_LOG){
            Log.e(LOG_RESULT_TAG, msg);
        }
    }


    static native void classInitNative();

    final native boolean initNative();

    native void cleanupNative();

    native int setSeekParamNative(int ownAddrType,
                                  int filterDuplicates,
                                  int seekFilterPolicy,
                                  int seekPhys,
                                  int seekType,
                                  int seekInterval,
                                  int seekWindow);

    native int startSeekNative();

    native int stopSeekNative();

}
