/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import android.content.Intent;
import android.nearlink.NearlinkAdapter;
import android.nearlink.NearlinkAppearance;
import android.nearlink.NearlinkDevice;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import com.android.nearlink.connection.PubTools;
import com.android.nearlink.discovery.publicdata.read.EasyPublicData;

import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 远端设备
 *
 * @since 2023-12-04
 */
public class RemoteDevices {

    private static final String TAG = "RemoteDevices";

    private static final boolean IS_REMOTE_DEVICE_DEBUG = false;

    private static final boolean IS_REMOTE_DEVICE_INFO = false;

    private ConcurrentHashMap<String, DeviceProperties> mDevices;

    NearlinkAdapter sAdapter;
    AdapterService sAdapterService;
    Handler mHandler;
    private final Object mObject = new Object();

    RemoteDevices(AdapterService service, Looper looper) {
        sAdapter = NearlinkAdapter.getDefaultAdapter();
        sAdapterService = service;
        mDevices = new ConcurrentHashMap<String, DeviceProperties>();
        mHandler = new RemoteDevicesHandler(looper);
        loadFromDisk(mDevices);
        showDevicesLoad();
    }

    void cleanup() {
        reset();
    }

    void reset() {

        if (mDevices != null) {
            Log.e(TAG, "mDevices clear");
            mDevices.clear();
        }
    }

    private class RemoteDevicesHandler extends Handler {

        /**
         * Handler must be created from an explicit looper to avoid threading ambiguity
         * @param looper The looper that this handler should be executed on
         */
        RemoteDevicesHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                default:
                    Log.e(TAG, "RemoteDevicesHandler can not handle what = " + msg.what);
            }
        }
    }

    DeviceProperties addDeviceProperties(String address) {
        DeviceProperties prop = new DeviceProperties();
        prop.mDevice = sAdapter.getRemoteDevice(address);
        prop.mAddress = address;
        DeviceProperties pv = mDevices.putIfAbsent(address, prop);
        return prop;
    }

    public void removeExpiredDevice(long currentTime, long expiredTime){
        if(mDevices == null){
            return;
        }

        debugLog("execute removeExpiredDevice");

        Iterator<Map.Entry<String, DeviceProperties>>  entryIterator  = mDevices.entrySet().iterator();
        while (entryIterator.hasNext()){

            Map.Entry<String, DeviceProperties> entry = entryIterator.next();

            DeviceProperties deviceProperties = entry.getValue();
            if(deviceProperties == null ){
                debugLog(" deviceProperties is null");
                continue;
            }

            long updateTimeMillis = deviceProperties.getUpdateTimeMillis();
            if(updateTimeMillis == 0){
                debugLog("key : " + entry.getKey() + " updateTimeMillis is 0");
                continue;
            }

            if(currentTime - updateTimeMillis < expiredTime){
                debugLog("key : " + entry.getKey() + " updateTimeMillis: " + updateTimeMillis + " not expired");
                continue;
            }

            NearlinkDevice nearlinkDevice = deviceProperties.getDevice();
            if(nearlinkDevice == null){
                debugLog( "key : " + entry.getKey() + " deviceProperties.nearlinkDevice is null");
                continue;
            }

            boolean isDisappeared = deviceProperties.isDisappeared();
            if(isDisappeared){
                debugLog("key : " + entry.getKey() + " isDisappeared is " + isDisappeared);
                continue;
            }
            deviceProperties.setDisappeared(true);

            Intent intent;
            intent = new Intent(NearlinkDevice.ACTION_DISAPPEARED);
            intent.putExtra(NearlinkDevice.EXTRA_DEVICE, nearlinkDevice);
            intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
            sAdapterService.sendBroadcast(intent, sAdapterService.NEARLINK_PERM);
            debugLog( "remove expired device address: " +  entry.getKey() + " updateTimeMillis: "  + updateTimeMillis);
        }
    }

    public synchronized void deviceRefreshUpdateTime(String address){
        NearlinkDevice bdDevice = getDevice(address);
        DeviceProperties device;
        if (bdDevice == null) {
            device = addDeviceProperties(address);
        } else {
            device = getDeviceProperties(bdDevice);
        }

        long updateTimeMillis = System.currentTimeMillis();
        // 设置seek到的更新时间
        device.setUpdateTimeMillis(updateTimeMillis);
        // 更新为未过期状态
        device.setDisappeared(false);
    }

    public synchronized void addOrSetAliasByAddress(NearlinkDevice device, String alias){
        NearlinkDevice bdDevice = getDevice(device.getAddress());
        DeviceProperties deviceProperties;
        if (bdDevice == null) {
            deviceProperties = addDeviceProperties(device.getAddress());
        } else {
            deviceProperties = getDeviceProperties(bdDevice);
        }
        deviceProperties.setAlias(NearlinkAdapter.getDefaultAdapter()
                        .getRemoteDevice(device.getAddress())
                , alias);
    }

    /**
     * seek result 的回调中尝试更新
     * @param
     */
    public synchronized void devicePropertyChangedCallback(String address, int newRssi, EasyPublicData easyPublicData) {

        Intent intent;
        NearlinkDevice bdDevice = getDevice(address);
        DeviceProperties device;
        if (bdDevice == null) {
            device = addDeviceProperties(address);
            bdDevice = getDevice(address);
        } else {
            device = getDeviceProperties(bdDevice);
        }

        String newName = easyPublicData.deviceLocalName;

        if (!DdUtils.isBlank(newName) &&
                !newName.equals(device.mName)) {
            debugLog("name changed adress : " + address
                    + " name :" + device.mName + " newName: " + newName);
            // 广播的名称变化，更新名称
            device.mName = newName;
            // 发送设备名称修改广播
            intent = new Intent(NearlinkDevice.ACTION_NAME_CHANGED);
            intent.putExtra(NearlinkDevice.EXTRA_DEVICE, bdDevice);
            intent.putExtra(NearlinkDevice.EXTRA_NAME, device.mName);
            intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
            sAdapterService.sendBroadcast(intent, sAdapterService.NEARLINK_PERM);


            if(address != null
                    && !address.isEmpty()
                    && !address.trim().isEmpty()){
                DiskUtils.writeNameOfDevicePropertiesToDisk(address, newName);

            }
        }

        if(newRssi != 0 &&
                newRssi != device.mRssi){
            // mRssi变化，则修改
            device.mRssi = newRssi;
        }

        int newAppearance = easyPublicData.appearance;
        if(newAppearance != 0 &&
                newAppearance != device.mNearlinkAppearance){
            device.mNearlinkAppearance = newAppearance;
            DiskUtils.writeAppearanceOfDevicePropertiesToDisk(address, newAppearance);
        }

    }

    public boolean isSeekResultReport(long currentMillis, long windowMillis, int eventType, NearlinkDevice nearlinkDevice){

        if(nearlinkDevice == null){
            debugLog("nearlinkDevice is null");
            return true;
        }

        if(DdUtils.isBlank(nearlinkDevice.getAddress())){
            debugLog("Address is blank");
            return true;
        }

        DeviceProperties deviceProperties = mDevices.get(nearlinkDevice.getAddress());
        if(deviceProperties == null){
            debugLog("deviceProperties is null");
            return true;
        }

        if(deviceProperties.lastAnnounceDataReportTime == 0){
            synchronized (mObject){
                deviceProperties.lastAnnounceDataReportTime = currentMillis;
            }
            debugLog("lastAnnounceDataReportTime is 0");
            return true;
        }

        if(deviceProperties.lastSeekRespDataReportTime == 0){
            synchronized (mObject){
                deviceProperties.lastSeekRespDataReportTime = currentMillis;
            }
            debugLog("lastSeekRespDataReportTime is 0");
            return true;
        }


        if(NearlinkAdapter.ANNOUNCE_DATA_EVENT_TYPE == eventType){
            if(currentMillis - deviceProperties.lastAnnounceDataReportTime
                    < windowMillis){
                debugLog(currentMillis + " - " + deviceProperties.lastAnnounceDataReportTime + " < " +  windowMillis);
                return false;
            }
            // 更新lastSeekResultReportTime
            debugLog("announce data result need upload");
            synchronized (mObject){
                deviceProperties.lastAnnounceDataReportTime = currentMillis;
            }
            return true;
        }

        if(NearlinkAdapter.SEEK_RSP_DATA_EVENT_TYPE == eventType){
            if(currentMillis - deviceProperties.lastSeekRespDataReportTime
                    < windowMillis){
                debugLog(currentMillis + " - " + deviceProperties.lastSeekRespDataReportTime + " < " +  windowMillis);
                return false;
            }
            // 更新lastSeekResultReportTime
            debugLog("seek resp data result need upload");
            synchronized (mObject){
                deviceProperties.lastSeekRespDataReportTime = currentMillis;
            }
            return true;
        }

        debugLog("unknown eventType = " + eventType);
        return true;
    }

    public synchronized boolean isNeedLog(String address, int logFrequency){
        NearlinkDevice bdDevice = getDevice(address);
        DeviceProperties device;
        if (bdDevice == null) {
            device = addDeviceProperties(address);
        } else {
            device = getDeviceProperties(bdDevice);
        }
        device.logCount++;
        if(device.logCount < logFrequency){
            return false;
        }
        device.logCount = 0;
        return true;
    }

    public DeviceProperties getDeviceProperties(NearlinkDevice device) {
        synchronized (mDevices) {
            return mDevices.get(device.getAddress());
        }
    }

    NearlinkDevice getDevice(String address) {
        DeviceProperties prop = mDevices.get(address);
        if (prop == null) {
            return null;
        }
        return prop.getDevice();
    }


    public class DeviceProperties {

        private String mName;
        private String mAddress;
        private int mRssi;
        private String mAlias;
        private NearlinkDevice mDevice;
        private volatile long updateTimeMillis;
        private volatile boolean isDisappeared = false;
        private volatile long lastAnnounceDataReportTime;
        private volatile long lastSeekRespDataReportTime;
        private int mNearlinkAppearance = NearlinkAppearance.UNKNOWN;
        private volatile int logCount = 0;

        public DeviceProperties() {

        }

        long getUpdateTimeMillis(){
            return updateTimeMillis;
        }

        boolean isDisappeared() {
            return isDisappeared;
        }

        void setDisappeared(boolean disappeared) {
            isDisappeared = disappeared;
        }

        long getLastAnnounceDataReportTime() {
            return lastAnnounceDataReportTime;
        }

        long getLastSeekRespDataReportTime() {
            return lastSeekRespDataReportTime;
        }

        void setUpdateTimeMillis(long updateTimeMillis){
            this.updateTimeMillis = updateTimeMillis;
        }

        void setAppearance(int appearance){
            synchronized (mObject){
                this.mNearlinkAppearance = appearance;
            }
        }

        String getAddress(){
            synchronized (mObject) {
                return mAddress;
            }
        }

        /**
         * @return the mName
         */
        public String getName() {
            synchronized (mObject) {
                return mName;
            }
        }

        public String getAlias() {
            synchronized (mObject) {
                return mAlias;
            }
        }

        public int getAppearance(){
            return mNearlinkAppearance;
        }

        NearlinkDevice getDevice() {
            synchronized (mObject) {
                return mDevice;
            }
        }

        void setAlias(NearlinkDevice device, String mAlias) {
            synchronized (mObject) {
                this.mAlias = mAlias;
                Intent intent = new Intent(NearlinkDevice.ACTION_ALIAS_CHANGED);
                intent.putExtra(NearlinkDevice.EXTRA_DEVICE, device);
                intent.putExtra(NearlinkDevice.EXTRA_NAME, mAlias);
                sAdapterService.sendBroadcast(intent, AdapterService.NEARLINK_PERM);

                String address = device.getAddress();
                if(address == null
                        || address.isEmpty()
                        || address.trim().isEmpty()){
                    return;
                }
                DiskUtils.writeAliasOfDevicePropertiesToDisk(address, mAlias);
            }
        }

    }

    private Integer convertToInteger(String obj){
        try{
            return Integer.valueOf(obj);
        }catch (Exception e){
            Log.e(TAG, "trans string to int error ", e);
            return NearlinkAppearance.UNKNOWN;
        }
    }

    private void loadFromDisk(ConcurrentHashMap<String, DeviceProperties> mDevices){

        loadNameFromDisk(mDevices);
        loadAliasFromDisk(mDevices);
        loadAppearanceFromDisk(mDevices);

        /**
         * 初始化DeviceProperties的其他值
         */
        Iterator<Map.Entry<String, DeviceProperties>> it = mDevices.entrySet().iterator();
        while (it.hasNext()){
            Map.Entry<String, DeviceProperties>  entry = it.next();
            String address = entry.getKey();
            if(DdUtils.isBlank(address)){
                continue;
            }
            DeviceProperties prop = entry.getValue();
            prop.mDevice = sAdapter.getRemoteDevice(address);
            prop.mAddress = address;
            prop.isDisappeared = false;
            prop.mRssi = 0;
            prop.updateTimeMillis = 0;
            prop.lastAnnounceDataReportTime = 0;
            prop.lastSeekRespDataReportTime = 0;
            prop.logCount = 0;
        }

    }

    // 初始化mName
    private void loadNameFromDisk(ConcurrentHashMap<String, DeviceProperties> mDevices){
        Map<String, String> allNameMap = DiskUtils.readNameOfDevicePropertiesFromDisk();
        if(allNameMap != null && allNameMap.size() > 0){
            Log.e(TAG, "load remote name properties size = " + allNameMap.size());
            for (Map.Entry<String, String> entry : allNameMap.entrySet()){
                DeviceProperties exist = mDevices.get(entry.getKey());
                if(exist == null){
                    DeviceProperties one = new DeviceProperties();
                    one.mAddress = entry.getKey();
                    one.mName = entry.getValue();
                    mDevices.putIfAbsent(entry.getKey(), one);
                }else {
                    exist.mName =  entry.getValue();
                }
                Log.e(TAG, "load remote addr = " + PubTools.macPrint(entry.getKey())
                        + " name = " + entry.getValue());
            }
        }
    }

    // 初始化 mAlias
    private void loadAliasFromDisk(ConcurrentHashMap<String, DeviceProperties> mDevices){
        Map<String, String> allAliasMap = DiskUtils.readAliasOfDevicePropertiesFromDisk();

        if(allAliasMap != null && allAliasMap.size() > 0){
            Log.e(TAG, "load remote alias properties size = " + allAliasMap.size());
            for (Map.Entry<String, String> entry : allAliasMap.entrySet()){

                DeviceProperties exist = mDevices.get(entry.getKey());
                if(exist == null){
                    DeviceProperties one = new DeviceProperties();
                    one.mAddress = entry.getKey();
                    one.mAlias = entry.getValue();
                    mDevices.putIfAbsent(entry.getKey(), one);
                }else {
                    exist.mAlias =  entry.getValue();
                }
                Log.e(TAG, "load remote addr = " + PubTools.macPrint(entry.getKey())
                        + " alias = " + entry.getValue());
            }
        }
    }

    // 初始化mAppearance
    private void loadAppearanceFromDisk(ConcurrentHashMap<String, DeviceProperties> mDevices){
        Map<String, String> allAppearanceMap = DiskUtils.readAppearanceOfDevicePropertiesFromDisk();

        if(allAppearanceMap != null && allAppearanceMap.size() > 0){
            Log.e(TAG, "load remote appearance properties size = " + allAppearanceMap.size());
            for (Map.Entry<String, String> entry : allAppearanceMap.entrySet()){

                DeviceProperties exist = mDevices.get(entry.getKey());
                if(exist == null){
                    DeviceProperties one = new DeviceProperties();
                    one.mAddress = entry.getKey();
                    one.mNearlinkAppearance = convertToInteger(entry.getValue());
                    mDevices.putIfAbsent(entry.getKey(), one);
                }else {
                    exist.mNearlinkAppearance =  convertToInteger(entry.getValue());
                }
                Log.e(TAG, "load remote addr = " + PubTools.macPrint(entry.getKey())
                        + " appearance = " + entry.getValue());
            }
        }
    }

    private void showDevicesLoad(){
        if(mDevices == null){
            Log.e(TAG, "mDevices is null");
            return;
        }
        Log.e(TAG, "mDevices load");
        for (Map.Entry<String, DeviceProperties> entry : mDevices.entrySet()){
            Log.e(TAG, "addr = " + PubTools.macPrint(entry.getKey())
                    + " name = " + entry.getValue().getName()
                    + " alias = " + entry.getValue().getAlias()
                    + " appearance = " + entry.getValue().getAppearance());
        }
    }

    private void debugLog(String msg){
        if(IS_REMOTE_DEVICE_DEBUG){
            Log.e(TAG, msg);
        }
    }

    private void infoLog(String msg){
        if(IS_REMOTE_DEVICE_DEBUG){
            Log.e(TAG, msg);
        }
    }


}
