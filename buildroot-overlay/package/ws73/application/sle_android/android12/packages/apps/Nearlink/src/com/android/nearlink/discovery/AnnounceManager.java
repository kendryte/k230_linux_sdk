/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.discovery;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAnnounceCallbackConstants;
import android.nearlink.NearlinkPublicData;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.nearlink.INearlinkAnnounceCallback;
import android.nearlink.NearlinkAnnounceParam;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Optional;
import java.util.Set;

import com.android.nearlink.discovery.publicdata.writer.PublicData;
import com.android.nearlink.discovery.publicdata.writer.PublicDataBuilder;
import com.android.nearlink.discovery.publicdata.PublicDataEnums;
import com.android.nearlink.nlservice.AdapterService;
import com.android.nearlink.nlservice.DdUtils;
import com.android.nearlink.nlservice.ProtoDeviceAddr;

/**
 * 广播管理器
 *
 * @since 2023-12-04
 */
public class AnnounceManager {

    private static final boolean DBG = DiscoveryServiceConfig.DBG;
    private static final String TAG = DiscoveryServiceConfig.TAG_PREFIX + "AdvertiseManager";

    final String DEFAULT_NAME = "Nearlink";

    Map<IBinder, AnnouncerInfo> mAnnouncers;
    Set<String> jniCallbackTimeoutRecords;

    private static final int ANNOUNCE_ON = 0;
    private static final int ANNOUNCE_OFF = 1;
    private static final int ANNOUNCE_TURING_ON = 2;
    private static final int ANNOUNCE_TURING_OFF = 3;

    // discovery的announceId固定为1
    private final int DEFAULT_DISCOVERY_ANNOUNCE_ID = 1;

    // 星闪介入能力, 00000010支持SLE
    private final byte ACCESS_LAYER_CAPABILITY_SLE_SUPPORT = 2;

    // 扫描停止时间
    private final int ANNOUNCE_TIMEOUT_MS_TYPE = 0;
    // 超过最大路数限制
    private final int ANNOUNCE_EXCEED_MAX_USING_NUM_ERROR_TYPE = 1;
    // 启动广播jni超时回调类型
    private final int ANNOUNCE_ENABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE = 2;
    // 关闭广播jni超时回调类型
    private final int ANNOUNCE_DISABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE = 3;

    // 广播回调超时时间
    private static final long ANNOUNCE_JNI_CALLBACK_TIMEOUT_MILLIS = 1500;

    private AdapterService mAdapterService;
    private DiscoveryService mDiscoveryService;

    private AnnounceIdManager announceIdManager;

    private AnnounceHandler announceHandler;

    static {
        classInitNative();
    }

    public AnnounceManager(AdapterService adapterService, DiscoveryService mDiscoveryService) {
        this.mAdapterService = adapterService;
        this.mDiscoveryService = mDiscoveryService;
        mAnnouncers = Collections.synchronizedMap(new HashMap<>());
        jniCallbackTimeoutRecords = Collections.synchronizedSet(new HashSet<>());
        announceIdManager = new AnnounceIdManager();
        HandlerThread thread = new HandlerThread("AnnounceManager");
        thread.start();
        announceHandler = new AnnounceHandler(thread.getLooper());
        initNative();
    }

    public void cleanUp() {

        this.mAdapterService = null;
        this.mDiscoveryService = null;

        if (mAnnouncers != null) {
            mAnnouncers.clear();
            mAnnouncers = null;
        }

        if(jniCallbackTimeoutRecords != null){
            jniCallbackTimeoutRecords.clear();
            jniCallbackTimeoutRecords = null;
        }

        if (announceIdManager != null) {
            announceIdManager.cleanup();
            announceIdManager = null;
        }

        if (announceHandler != null) {
            // Shut down the thread
            announceHandler.removeCallbacksAndMessages(null);
            Looper looper = announceHandler.getLooper();
            if (looper != null) {
                looper.quitSafely();
            }
            announceHandler = null;
        }

        cleanupNative();
    }


    public boolean startAnnounceForDiscovery(NearlinkAnnounceParam nearlinkAnnounceParam,
                                             NearlinkPublicData nearlinkAnnounceData,
                                             NearlinkPublicData nearlinkSeekRespData) {
        Log.e(TAG, "AnnounceManager startAnnounceForDiscovery");
        PublicData announcePublicData = buildAnnounceData(nearlinkAnnounceData);
        PublicData respPublicData = buildSeekRespData(nearlinkSeekRespData);
        return startAnnounceToNative(DEFAULT_DISCOVERY_ANNOUNCE_ID, nearlinkAnnounceParam, announcePublicData, respPublicData);
    }

    public boolean startAnnounce(int timeoutMillis,
                                 NearlinkAnnounceParam nearlinkAnnounceParam,
                                 NearlinkPublicData nearlinkAnnounceData,
                                 NearlinkPublicData nearlinkSeekRespData,
                                 INearlinkAnnounceCallback iNearlinkAnnounceCallback) {
        Log.e(TAG, "AnnounceManager startAnnounce");

        int announceId = announceIdManager.borrowAnnounceId();
        if (NearlinkAdapter.EXCEED_MAX_ANNOUNCE_USING_NUM == announceId) {
            Log.e(TAG, "ANNOUNCE_ID is EXCEED_MAX_ANNOUNCE_NUM : " + announceId);
            sendAnnounceMessage(ANNOUNCE_EXCEED_MAX_USING_NUM_ERROR_TYPE,
                    iNearlinkAnnounceCallback);
            return false;
        }

        AnnounceDeathRecipient deathRecipient = new AnnounceDeathRecipient(iNearlinkAnnounceCallback);
        IBinder binder = toBinder(iNearlinkAnnounceCallback);
        try {
            binder.linkToDeath(deathRecipient, 0);
        } catch (RemoteException e) {
            throw new IllegalArgumentException("Can't link to announcer's death");
        }
        AnnouncerInfo announcerInfo = new AnnouncerInfo(announceId,
                ANNOUNCE_TURING_ON, timeoutMillis,
                deathRecipient, iNearlinkAnnounceCallback);
        mAnnouncers.put(binder, announcerInfo);
        Log.e(TAG, "announceId is " + announceId);
        PublicData announcePublicData = buildAnnounceData(nearlinkAnnounceData);
        PublicData respPublicData = buildSeekRespData(nearlinkSeekRespData);
        boolean res = startAnnounceToNative(announceId, nearlinkAnnounceParam,
                announcePublicData, respPublicData);
        if(!res){
            releaseAnnouncerInfo(announcerInfo);
        }else {
            addAnnounceStartJinCallbackTimeoutRecord(announcerInfo);
        }
        return res;
    }

    public boolean stopAnnounceForDiscovery() {
        int res = stopAnnounceNative(DEFAULT_DISCOVERY_ANNOUNCE_ID);
        if(res == 0){
            return true;
        }
        Log.e(TAG, "announceId = " + DEFAULT_DISCOVERY_ANNOUNCE_ID
                +" stopAnnounceNative error " + res);
        return false;
    }

    public boolean stopAnnounce(INearlinkAnnounceCallback iNearlinkAnnounceCallback) {

        Log.e(TAG, "service stopAnnounce");
        IBinder binder = toBinder(iNearlinkAnnounceCallback);
        AnnouncerInfo exist = mAnnouncers.get(binder);
        if (exist == null) {
            Log.e(TAG, "AnnouncerInfo is not exist");
            return false;
        }

        int res =  stopAnnounceNative(exist.announceId);
        if(res == 0){
            exist.announceStatus = ANNOUNCE_TURING_OFF;
            addAnnounceStopJinCallbackTimeoutRecord(exist);
            return true;
        }
        Log.e(TAG, "announceId = " + exist.announceId
                +" stopAnnounceNative error " + res);
        return false;
    }


    private void addAnnounceStartJinCallbackTimeoutRecord(AnnouncerInfo announcerInfo){
        String key = buildJinTimeoutKey(announcerInfo.announceId,
                ANNOUNCE_ENABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE);
        jniCallbackTimeoutRecords.add(key);
        sendAnnounceTimeoutDelayedMessage(ANNOUNCE_ENABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE,
                ANNOUNCE_JNI_CALLBACK_TIMEOUT_MILLIS, announcerInfo);
    }

    private void removeAnnounceStartJinCallbackTimeoutRecord(int announceId){
        String key = buildJinTimeoutKey(announceId,
                ANNOUNCE_ENABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE);
        jniCallbackTimeoutRecords.remove(key);
    }

    private void addAnnounceStopJinCallbackTimeoutRecord(AnnouncerInfo announcerInfo){
        String key = buildJinTimeoutKey(announcerInfo.announceId,
                ANNOUNCE_DISABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE);
        jniCallbackTimeoutRecords.add(key);
        sendAnnounceTimeoutDelayedMessage(ANNOUNCE_DISABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE,
                ANNOUNCE_JNI_CALLBACK_TIMEOUT_MILLIS, announcerInfo);
    }

    private void removeAnnounceStopJinCallbackTimeoutRecord(int announceId){
        String key = buildJinTimeoutKey(announceId,
                ANNOUNCE_DISABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE);
        jniCallbackTimeoutRecords.remove(key);
    }

    private boolean isContainJinCallbackKey(Integer announceId, int type){
        String key = buildJinTimeoutKey(announceId, type);
        return jniCallbackTimeoutRecords.contains(key);
    }

    private String buildJinTimeoutKey(Integer announceId, int type){
        if(announceId == null){
            Log.e(TAG, "announceId is null");
        }
        StringBuilder sb = new StringBuilder();
        sb.append(announceId).append("_").append(type);
        return sb.toString();
    }

    private class AnnounceHandler extends Handler {

        AnnounceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.e(TAG, "handleMessage,msg.what=" + msg.what);
            switch (msg.what) {
                case ANNOUNCE_TIMEOUT_MS_TYPE:
                    stopAnnounce((INearlinkAnnounceCallback) msg.obj);
                    break;
                case ANNOUNCE_EXCEED_MAX_USING_NUM_ERROR_TYPE:
                    handleMaxAnnounceNumError((INearlinkAnnounceCallback) msg.obj);
                    break;
                case ANNOUNCE_ENABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE:
                    handleJinEnableCallbackTimeout((AnnouncerInfo) msg.obj);
                    break;
                case ANNOUNCE_DISABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE:
                    handleJinDisableCallbackTimeout((AnnouncerInfo) msg.obj);
                    break;
                default:
                    // Shouldn't happen.
                    Log.e(TAG, "received an unkown message : " + msg.what);
            }
        }
    }

    private void sendAnnounceTimeoutDelayedMessage(int what, long delayMillis, Object obj){
        final AnnounceHandler handler = announceHandler;
        if (handler == null) {
            Log.e(TAG, "sendMessage: mHandler is null.");
            return;
        }
        Message message = new Message();
        message.what = what;
        message.obj = obj;
        handler.sendMessageDelayed(message, delayMillis);
    }


    private void sendAnnounceMessage(int what, Object obj){
        final AnnounceHandler handler = announceHandler;
        if (handler == null) {
            Log.e(TAG, "sendMessage: mHandler is null.");
            return;
        }
        Message message = new Message();
        message.what = what;
        message.obj = obj;
        handler.sendMessage(message);
    }

    class AnnouncerInfo {

        public int announceId;
        public volatile int announceStatus;
        public int timeoutMillis;
        public AnnounceDeathRecipient deathRecipient;
        public INearlinkAnnounceCallback callback;

        public AnnouncerInfo(int announceId, int announceStatus, int timeoutMillis,
                             AnnounceDeathRecipient deathRecipient,
                             INearlinkAnnounceCallback callback) {
            this.announceId = announceId;
            this.announceStatus = announceStatus;
            this.timeoutMillis = timeoutMillis;
            this.deathRecipient = deathRecipient;
            this.callback = callback;
        }
    }

    IBinder toBinder(INearlinkAnnounceCallback e) {
        return e.asBinder();
    }

    class AnnounceDeathRecipient implements IBinder.DeathRecipient {

        public INearlinkAnnounceCallback iNearlinkAnnounceCallback;

        AnnounceDeathRecipient(INearlinkAnnounceCallback iNearlinkAnnounceCallback) {
            this.iNearlinkAnnounceCallback = iNearlinkAnnounceCallback;
        }

        @Override
        public void binderDied() {
            Log.e(TAG, "Binder is dead - unregistering announce");
            stopAnnounce(iNearlinkAnnounceCallback);
        }
    }

    private void handleMaxAnnounceNumError(INearlinkAnnounceCallback iNearlinkAnnounceCallback){
        try {
            iNearlinkAnnounceCallback.onAnnounceEnabled(NearlinkAdapter.EXCEED_MAX_ANNOUNCE_USING_NUM,
                    NearlinkAnnounceCallbackConstants.ANNOUNCE_EXCEED_MAX_USING_NUM);
        }catch (RemoteException e){
            Log.e(TAG, "onAnnounceEnabled() - exceed max announce num Callback # failed (" + e + ")");
        }
    }

    private void handleJinEnableCallbackTimeout(AnnouncerInfo timeoutInfo){
        Log.e(TAG, "handleJinEnableCallbackTimeout");
        if(!isContainJinCallbackKey(timeoutInfo.announceId,
                ANNOUNCE_ENABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE)){
            return;
        }
        IBinder binder = toBinder(timeoutInfo.callback);
        AnnouncerInfo announcerInfo = mAnnouncers.get(binder);
        if(announcerInfo == null){
            Log.e(TAG, "cannot find binder" + binder);
            return;
        }
        Optional<AnnouncerInfo> optional = releaseAnnouncerInfo(announcerInfo);
        if(!optional.isPresent()){
            return;
        }
        AnnouncerInfo exist = optional.get();
        Log.e(TAG, "handleJinEnableCallbackTimeout,announceId= " + exist.announceId);
        try{
            exist.callback.onAnnounceEnabled(exist.announceId,
                    NearlinkAnnounceCallbackConstants.ANNOUNCE_HARDWARE_CALLBACK_ERROR);
        }catch (RemoteException e){
            Log.e(TAG, "handleJinEnableCallbackTimeout() - Callback # failed (" + e + ")");
        }
    }

    private void handleJinDisableCallbackTimeout(AnnouncerInfo timeoutInfo){
        Log.e(TAG, "handleJinDisableCallbackTimeout");
        if(!isContainJinCallbackKey(timeoutInfo.announceId,
                ANNOUNCE_DISABLE_JNI_CALLBACK_TIMEOUT_ERROR_TYPE)){
            return;
        }
        IBinder binder = toBinder(timeoutInfo.callback);
        AnnouncerInfo announcerInfo = mAnnouncers.get(binder);
        if(announcerInfo == null){
            Log.e(TAG, "can not find binder");
            return;
        }
        Optional<AnnouncerInfo> optional = releaseAnnouncerInfo(announcerInfo);
        if(!optional.isPresent()){
            return;
        }
        AnnouncerInfo exist = optional.get();
        Log.e(TAG, "handleJinDisableCallbackTimeout,announceId= " + exist.announceId);
        try{
            exist.callback.onAnnounceDisabled(exist.announceId,
                    NearlinkAnnounceCallbackConstants.ANNOUNCE_HARDWARE_CALLBACK_ERROR);
        }catch (RemoteException e){
            Log.e(TAG, "handleJinDisableCallbackTimeout() - Callback # failed (" + e + ")");
        }
    }

    public void onAnnounceEnabledCallback(int announceId, int status) throws RemoteException {

        Log.e(TAG, "onAnnounceEnabledCallback() - announceId=" + announceId + ", status=" + status);

        if (DEFAULT_DISCOVERY_ANNOUNCE_ID == announceId) {
            if (status != 0) {
                Log.e(TAG, "discovery start announce fail, announceId = " + announceId + " status = " + status);
                return;
            }
            return;
        }

        removeAnnounceStartJinCallbackTimeoutRecord(announceId);
        Map.Entry<IBinder, AnnouncerInfo> entry = findAnnouncer(announceId);
        if (entry == null) {
            Log.e(TAG, "onAnnounceEnabledCallback() - no callback found for announceId " + announceId);
            return;
        }

        AnnouncerInfo announcerInfo = entry.getValue();
        announcerInfo.announceStatus = ANNOUNCE_ON;
        INearlinkAnnounceCallback callback = announcerInfo.callback;

        if (status != 0) {
            Log.e(TAG, "onAnnounceEnabledCallback announceId = " + announceId + " status = " + status + " error !!!");
            releaseAnnouncerInfo(announcerInfo);
        }else {
            sendAnnounceTimeoutDelayedMessage(ANNOUNCE_TIMEOUT_MS_TYPE, announcerInfo.timeoutMillis, callback);
        }

        callback.onAnnounceEnabled(announceId, status);
    }


    public void onAnnounceDisabledCallback(int announceId, int status) throws RemoteException {
        Log.e(TAG, "onAnnounceDisabledCallback() - announceId=" + announceId + ", status=" + status);
        if (DEFAULT_DISCOVERY_ANNOUNCE_ID == announceId) {
            if (status != 0) {
                Log.e(TAG, "discovery stop announce fail, announceId = " + announceId + " status = " + status);
                return;
            }
            return;
        }
        removeAnnounceStopJinCallbackTimeoutRecord(announceId);
        Map.Entry<IBinder, AnnouncerInfo> entry = findAnnouncer(announceId);
        if (entry == null) {
            Log.e(TAG, "onAnnounceEnabledCallback() - no callback found for announceId " + announceId);
            return;
        }
        AnnouncerInfo announcerInfo = entry.getValue();
        Optional<AnnouncerInfo> announcerInfoOptional = releaseAnnouncerInfo(announcerInfo);
        if (announcerInfoOptional.isPresent()) {
            Log.e(TAG, "execute callback.onAnnounceDisabled - announceId = " + announceId);
            announcerInfoOptional.get().callback.onAnnounceDisabled(announceId, status);
        }
    }

    public void onAnnounceTerminaledCallback(int announceId) throws RemoteException {

        Log.e(TAG, "onAnnounceTerminaledCallback() - announceId=" + announceId);

        if (DEFAULT_DISCOVERY_ANNOUNCE_ID == announceId) {
            Log.e(TAG, "terminal discovery Announce, do nothing");
            return;
        }
        Map.Entry<IBinder, AnnouncerInfo> entry = findAnnouncer(announceId);
        if (entry == null) {
            Log.e(TAG, "onAnnounceEnabledCallback() - no callback found for announceId " + announceId);
            return;
        }
        AnnouncerInfo announcerInfo = entry.getValue();
        Optional<AnnouncerInfo> announcerInfoOptional = releaseAnnouncerInfo(announcerInfo);
        if (announcerInfoOptional.isPresent()) {
            Log.e(TAG, "execute callback.onAnnounceTerminaled - announceId = " + announceId);
            announcerInfoOptional.get().callback.onAnnounceTerminaled(announceId);
        }
    }

    private Optional<AnnouncerInfo> releaseAnnouncerInfo(AnnouncerInfo announcerInfo) {
        /**
         * 释放announceId
         */
        announceIdManager.releaseAnnounceId(announcerInfo.announceId);

        /**
         * 底层断开链接顶层不关注，删除callback即可
         */
        IBinder binder = toBinder(announcerInfo.callback);
        AnnouncerInfo exist = mAnnouncers.remove(binder);
        if (exist == null) {
            Log.e(TAG, "stopAnnouncingSet() - no client found for callback");
            return Optional.empty();
        }

        binder.unlinkToDeath(announcerInfo.deathRecipient, 0);

        return Optional.ofNullable(announcerInfo);
    }

    private Map.Entry<IBinder, AnnouncerInfo> findAnnouncer(int announceId) {
        Map.Entry<IBinder, AnnouncerInfo> entry = null;
        for (Map.Entry<IBinder, AnnouncerInfo> e : mAnnouncers.entrySet()) {
            if (e.getValue().announceId == announceId) {
                entry = e;
                break;
            }
        }
        return entry;
    }

    private PublicData buildAnnounceData(NearlinkPublicData nearlinkAnnounceData){
        PublicData announcePublicData = PublicDataBuilder.builder()
                .writeValue((byte)nearlinkAnnounceData.getAnnounceLevel(), PublicDataEnums.SEEK_LEVEL)
                .writeValue(ACCESS_LAYER_CAPABILITY_SLE_SUPPORT, PublicDataEnums.ACCESS_LAYER_CAPABILITY)
                .build();
        byte[] announceDataBytes = nearlinkAnnounceData.getData();
        if(announceDataBytes != null){
            byte[] newAnnounceDataBytes = DdUtils.mergeBytes(announcePublicData.getData(), announceDataBytes);
            announcePublicData.setData(newAnnounceDataBytes);
            announcePublicData.setLength(newAnnounceDataBytes.length);
        }
        return announcePublicData;
    }

    private PublicData buildSeekRespData(NearlinkPublicData nearlinkSeekRespData){
        String deviceShortName = mAdapterService.getName();
        if (DdUtils.isBlank(deviceShortName)) {
            deviceShortName = DEFAULT_NAME;
        }
        Log.e(TAG, "announce deviceShortName " + deviceShortName);
        PublicData respPublicData = PublicDataBuilder.builder()
                .writeValue(deviceShortName, PublicDataEnums.DEVICE_SHORT_LOCAL_NAME)
                .build();

        byte[] seekRespDataBytes = nearlinkSeekRespData.getData();
        if(seekRespDataBytes != null){
            byte[] newRespPublicDataBytes = DdUtils.mergeBytes(respPublicData.getData(), seekRespDataBytes);
            respPublicData.setData(newRespPublicDataBytes);
            respPublicData.setLength(newRespPublicDataBytes.length);
        }
        return respPublicData;
    }

    private boolean startAnnounceToNative(int announceId, NearlinkAnnounceParam nearlinkAnnounceParam, PublicData announcePublicData,
                                          PublicData respPublicData) {

        Log.e(TAG, "announceId is " + announceId + "announce public data length is " + announcePublicData.getLength()
                + "resp public data length is " + respPublicData.getLength());
        ProtoDeviceAddr protoDeviceAddr = mAdapterService.getProtoAddress();
        int res = setAnnounceParamNative(announceId,
                announceId,
                nearlinkAnnounceParam.getAnnounceMode(),
                nearlinkAnnounceParam.getAnnounceGtRole(),
                nearlinkAnnounceParam.getAnnounceLevel(),
                nearlinkAnnounceParam.getAnnounceIntervalMin(),
                nearlinkAnnounceParam.getAnnounceIntervalMax(),
                nearlinkAnnounceParam.getAnnounceChannelMap(),
                nearlinkAnnounceParam.getAnnounceTxPower(),
                protoDeviceAddr.addr,
                protoDeviceAddr.type,
                nearlinkAnnounceParam.getPeerAddress(),
                nearlinkAnnounceParam.getPeerAddrType(),
                nearlinkAnnounceParam.getConnIntervalMin(),
                nearlinkAnnounceParam.getConnIntervalMax(),
                nearlinkAnnounceParam.getConnMaxLatency(),
                nearlinkAnnounceParam.getConnSupervisionTimeout());
        if(res != 0){
            Log.e(TAG, "setAnnounceParamNative res: " + res);
            return false;
        }

        res = setAnnounceDataNative(announceId,
                announcePublicData.getLength(),
                respPublicData.getLength(),
                announcePublicData.getData(),
                respPublicData.getData());
        if(res != 0){
            Log.e(TAG, "setAnnounceDataNative res: " + res);
            return false;
        }

        res = startAnnounceNative(announceId);
        if(res != 0){
            Log.e(TAG, "startAnnounceNative res: " + res);
            return false;
        }

        return true;
    }

    static native void classInitNative();

    final native boolean initNative();

    native void cleanupNative();

    native int setAnnounceParamNative(int announceId,
                                      int announceHandle,
                                      int announceMode,
                                      int announceGtRole,
                                      int announceLevel,
                                      long announceIntervalMin,
                                      long announceIntervalMax,
                                      int announceChannelMap,
                                      byte announceTxPower,
                                      byte[] ownAddress,
                                      int ownAddrType,
                                      String peerAddress,
                                      int peerAddrType,
                                      int connIntervalMin,
                                      int connIntervalMax,
                                      int connMaxLatency,
                                      int connSupervisionTimeout);


    native int setAnnounceDataNative(int announceId,
                                     int announceDataLen, int seekRspDataLen,
                                     byte[] announceData, byte[] seekRspData);


    native int startAnnounceNative(int announceId);

    native int stopAnnounceNative(int announceId);

}

