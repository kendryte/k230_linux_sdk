/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package com.android.nearlink.discovery;

import android.nearlink.INearlinkSeekCallback;
import android.nearlink.NearlinkSeekFilter;
import android.nearlink.NearlinkSeekParams;
import android.os.IBinder;

import java.util.List;

/**
 * SeekClient客户端
 *
 * @since 2023-12-04
 */
public class SeekClient {

    static final int START_SEEK_TYPE = 0;
    static final int START_DISCOVERY_TYPE = 1;
    static final int START_SEEK_INTENT = 2;
    static final int UN_CALLED = 0;
    static final int CALLED = 1;

    String seekId;
    int type;
    volatile int seekStartCallbackStatus;
    volatile int seekStopCallbackStatus;
    NearlinkSeekParams nearlinkSeekParams;
    INearlinkSeekCallback iNearlinkSeekCallback;
    IBinder.DeathRecipient deathRecipient;
    List<NearlinkSeekFilter> filters;

    public SeekClient(String seekId, int type, NearlinkSeekParams nearlinkSeekParams, INearlinkSeekCallback iNearlinkSeekCallback, List<NearlinkSeekFilter> filters) {
        this.seekId = seekId;
        this.type = type;
        this.nearlinkSeekParams = nearlinkSeekParams;
        this.seekStartCallbackStatus = UN_CALLED;
        this.seekStopCallbackStatus = UN_CALLED;
        this.iNearlinkSeekCallback = iNearlinkSeekCallback;
        this.filters = filters;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    public String getSeekId() {
        return seekId;
    }

    public void setSeekId(String seekId) {
        this.seekId = seekId;
    }

    public NearlinkSeekParams getNearlinkSeekParams() {
        return nearlinkSeekParams;
    }

    public void setNearlinkSeekParams(NearlinkSeekParams nearlinkSeekParams) {
        this.nearlinkSeekParams = nearlinkSeekParams;
    }

    public INearlinkSeekCallback getiNearlinkSeekCallback() {
        return iNearlinkSeekCallback;
    }

    public void setiNearlinkSeekCallback(INearlinkSeekCallback iNearlinkSeekCallback) {
        this.iNearlinkSeekCallback = iNearlinkSeekCallback;
    }

    public IBinder.DeathRecipient getDeathRecipient() {
        return deathRecipient;
    }

    public void setDeathRecipient(IBinder.DeathRecipient deathRecipient) {
        this.deathRecipient = deathRecipient;
    }

    public List<NearlinkSeekFilter> getFilters() {
        return filters;
    }

    public void setFilters(List<NearlinkSeekFilter> filters) {
        this.filters = filters;
    }

    public int getSeekStartCallbackStatus() {
        return seekStartCallbackStatus;
    }

    public void setSeekStartCallbackStatus(int seekStartCallbackStatus) {
        this.seekStartCallbackStatus = seekStartCallbackStatus;
    }

    public int getSeekStopCallbackStatus() {
        return seekStopCallbackStatus;
    }

    public void setSeekStopCallbackStatus(int seekStopCallbackStatus) {
        this.seekStopCallbackStatus = seekStopCallbackStatus;
    }
}
