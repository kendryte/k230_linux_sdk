/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import android.util.Log;

import com.android.nearlink.connection.PubTools;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.Properties;

/**
 * SharedPreferences 持久化保存工具
 *
 * @since 2023-12-04
 */
public class DiskUtils {

    private static final String TAG = "DiskUtils";

    private static final String BASE_DIR = "/data/misc/nearlink/";

    private static final String PROP_FILE_LOCATION = BASE_DIR + "prop";

    private static final String LOCAL_PREFIX = "0_";
    private static final String REMOTE_NAME_PREFIX = "1_";
    private static final String REMOTE_ALIAS_PREFIX = "2_";
    private static final String REMOTE_APPEARANCE_PREFIX = "3_";

    /**
     * 本地设备名称命名key
     */
    private static final String DEVICE_LOCAL_NAME_KEY = "DEVICE_LOCAL_NAME_KEY";

    private static class Holder {

        private volatile static Properties properties;

        private void Holder(){}

        public static Properties getSingleInstance(){
            if(properties == null){
                synchronized (Holder.class){
                    if(properties == null){
                        Optional<Properties> optional
                                = loadProperties(PROP_FILE_LOCATION);
                        properties = optional.get();
                        Log.e(TAG, "start load properties");
                        for (String key : properties.stringPropertyNames()){
                            String value = properties.getProperty(key);
                            Log.e(TAG, "key = " + PubTools.macPrint(key)
                                    + "value = " + value);
                        }
                    }
                }
            }
            return properties;
        }

        private static Optional<Properties> loadProperties(String fileLocation) {
            Optional<File> optional = createIfNotExistFile(fileLocation);
            if(!optional.isPresent()){
                Log.e(TAG, "set prop file is null");
                return Optional.empty();
            }
            FileInputStream fis = null;
            Properties props = new Properties();
            try {
                fis = new FileInputStream(optional.get());
                props.load(fis);
            } catch (IOException e) {
                Log.e(TAG, "load file error ", e);
                e.printStackTrace();
            } finally {
                if (fis != null) {
                    try{
                        fis.close();
                    }catch (IOException e){
                        Log.e(TAG, "close error", e);
                    }

                }
            }
            return Optional.ofNullable(props);
        }
    }


    private static void setProps(Properties p, String fileLocation) {
        File propFile = new File(fileLocation);
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(propFile);
            p.store(fos, null);
            fos.flush();
        } catch (IOException e) {
            Log.e(TAG, "load file error ", e);
            e.printStackTrace();
        } finally {
            if (fos != null) {
                try{
                    fos.close();
                }catch (IOException e){
                    Log.e(TAG, "close error", e);
                }
            }
        }
    }

    private static Optional<File> createIfNotExistFile(String fileLocation){
        File propertiesFile = new File(fileLocation);
        if (!propertiesFile.exists()) {
            boolean flag;
            try{
                flag = propertiesFile.createNewFile();
            }catch (IOException e){
                e.printStackTrace();
                Log.e(TAG, "create file error", e);
                return Optional.empty();
            }

            if (!flag) {
                Log.e(TAG, "createNewFile failed flag is " + flag);
                return Optional.empty();
            }
        }
        return Optional.ofNullable(propertiesFile);
    }


    public synchronized static void set(String key, String value) {
        Properties properties = Holder.getSingleInstance();
        properties.setProperty(key, value);
        setProps(properties, PROP_FILE_LOCATION);
    }

    public static String get(String key) {
        return Holder.getSingleInstance().getProperty(key);
    }


    public static Map<String, String> getAddressToValueByPrefix(String prefix) {
        Map<String, String> resMap = new HashMap<>();
        Properties properties = Holder.getSingleInstance();
        for (String key : properties.stringPropertyNames()){
            if(!key.startsWith(prefix)){
                continue;
            }
            String value = properties.getProperty(key);
            String address = key.substring(prefix.length());
            resMap.put(address, value);
        }
        return resMap;
    }

    /**
     * 设备本端名称读取
     *
     * @return
     */
    public static String readNameOfLocalDeviceFromDisk() {
        String key = buildKey(LOCAL_PREFIX, DEVICE_LOCAL_NAME_KEY);
        return get(key);
    }

    /**
     * 设备本端名称持久化
     *
     * @return
     */
    public static void writeNameOfLocalDeviceToDisk(String localName) {
        String key = buildKey(LOCAL_PREFIX, DEVICE_LOCAL_NAME_KEY);
        set(key, localName);
    }

    /**
     * 远端设备名称读取
     *
     * @return
     */
    public static Map<String, String> readNameOfDevicePropertiesFromDisk() {
        return getAddressToValueByPrefix(REMOTE_NAME_PREFIX);
    }

    /**
     * 远端别名读取
     *
     * @return
     */
    public static Map<String, String> readAliasOfDevicePropertiesFromDisk() {
        return getAddressToValueByPrefix(REMOTE_ALIAS_PREFIX);
    }

    /**
     * 远端外观数据读取
     * @return
     */
    public static Map<String, String> readAppearanceOfDevicePropertiesFromDisk() {
        return getAddressToValueByPrefix(REMOTE_APPEARANCE_PREFIX);
    }


    /**
     * 远端设备名称持久化
     *
     * @param address
     * @param nameValue
     */
    public static void writeNameOfDevicePropertiesToDisk(String address, String nameValue) {
        String key = buildKey(REMOTE_NAME_PREFIX, address);
        set(key, nameValue);
    }

    /**
     * 远端别名持久化
     *
     * @param address
     * @param aliasValue
     */
    public static void writeAliasOfDevicePropertiesToDisk(String address, String aliasValue) {
        String key = buildKey(REMOTE_ALIAS_PREFIX, address);
        set(key, aliasValue);
    }


    /**
     * 远端外观数据持久化
     *
     * @param address
     * @param appearance
     */
    public static void writeAppearanceOfDevicePropertiesToDisk(String address, int appearance) {
        String value = null;
        try{
            value = String.valueOf(appearance);
        }catch (Exception e){
            Log.e(TAG, "trans int to String err", e);
        }
        if(value == null){
            return;
        }
        String key = buildKey(REMOTE_APPEARANCE_PREFIX, address);
        set(key, value);
    }


    private static String buildKey(String prefix, String key){
        StringBuilder sb = new StringBuilder();
        sb.append(prefix).append(key);
        return sb.toString();
    }


}
