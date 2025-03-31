/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery;

import android.nearlink.NearlinkAdapter;
import android.util.Log;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * 广播ID管理器
 *
 * @since 2023-12-04
 */
public class AnnounceIdManager {

    private static final String TAG = "AnnounceIdManager";

    private Set<Integer> mAnnounceIdsPools;
    private AtomicInteger atomicInteger;
    private int announceUsingNum;

    // 最大可使用announce路数
    private final int MAX_ANNOUNCE_USING_NUM = 1;
    // 闭区间
    private final int MIN_INIT_ANNOUNCE_ID = 2;
    // 闭区间
    private final int MAX_INIT_ANNOUNCE_ID = 2;
    // so库接收的最大广播参数
    private final int MAX_ANNOUNCE_ID_LEN = 255;

    public AnnounceIdManager() {
        announceUsingNum = 0;
        atomicInteger = new AtomicInteger(MAX_INIT_ANNOUNCE_ID);
        mAnnounceIdsPools = new HashSet<>(MAX_ANNOUNCE_USING_NUM);
        initAnnounceIdsPools(mAnnounceIdsPools);
    }

    public void cleanup(){
        if(mAnnounceIdsPools != null){
            mAnnounceIdsPools.clear();
            mAnnounceIdsPools = null;
        }

        atomicInteger = null;
    }

    private void initAnnounceIdsPools(Set<Integer> announceIdsPools){
        for (int announceId = MIN_INIT_ANNOUNCE_ID ; announceId <= MAX_INIT_ANNOUNCE_ID  ; announceId ++){
            announceIdsPools.add(announceId);
        }
    }

    public Integer borrowAnnounceId(){
        synchronized (mAnnounceIdsPools){

            if(announceUsingNum >= MAX_ANNOUNCE_USING_NUM){
                Log.e(TAG, "announceUsingNum is " + announceUsingNum + " > MAX_ANNOUNCE_USING_NUM " + MAX_ANNOUNCE_USING_NUM);
                return NearlinkAdapter.EXCEED_MAX_ANNOUNCE_USING_NUM;
            }

            announceUsingNum++;

            if(!mAnnounceIdsPools.isEmpty()){
                Iterator<Integer> it = mAnnounceIdsPools.iterator();
                Integer announceId = it.next();
                it.remove();
                return announceId;
            }

            Log.e(TAG, "mAnnounceIdsPools is empty, try get from atomicInteger");
            int newAnnounceId = atomicInteger.incrementAndGet();
            if(newAnnounceId >= MAX_ANNOUNCE_ID_LEN){
                Log.e(TAG, "newAnnounceId exceed MAX_ANNOUNCE_ID_LEN : " + newAnnounceId);
                return NearlinkAdapter.EXCEED_MAX_ANNOUNCE_USING_NUM;
            }

            return newAnnounceId;
        }
    }

    public void releaseAnnounceId(Integer announceId){
        synchronized (mAnnounceIdsPools){
            Log.e(TAG, "releaseAnnounceId : " + announceId);
            announceUsingNum--;
            mAnnounceIdsPools.add(announceId);
        }
    }

}
