/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * 扫描管理器
 *
 * @since 2023-12-04
 */
public class NearlinkSeeker {

    private static final boolean DBG = true;
    private static final String TAG = "NearlinkSeeker";

    private final INearlinkManager mNearlinkServiceManage;
    private final Map<NearlinkSeekCallback, NearlinkSeekCallbackWrapper> mScanClients;
    private final Handler mHandler;
    private NearlinkAdapter mNearlinkAdapter;

    /**
     * @hide
     */
    public NearlinkSeeker(INearlinkManager iNearlinkManager) {
        this.mNearlinkServiceManage = iNearlinkManager;
        mHandler = new Handler(Looper.getMainLooper());
        this.mNearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        mScanClients = new HashMap<NearlinkSeekCallback, NearlinkSeekCallbackWrapper>();
    }

    /**
     * 启动扫描，星闪扫描到结果使用NearlinkSeekCallback回调调用方
     *
     * @param seekCallback 回调
     */
    public void startSeek(NearlinkSeekCallback seekCallback) {
        startSeek(new ArrayList<>(), seekCallback);
    }

    /**
     * 启动扫描，星闪扫描到结果使用NearlinkSeekCallback回调调用方
     *
     * @param filters      过滤器
     * @param seekCallback 回调
     */
    public void startSeek(List<NearlinkSeekFilter> filters, NearlinkSeekCallback seekCallback) {

        NearlinkUtils.checkAdapterStateOn(mNearlinkAdapter);

        NearlinkSeekParams nearlinkSeekParams = NearlinkSeekParams.builder().build();
        synchronized (mScanClients) {
            if (seekCallback != null && mScanClients.containsKey(seekCallback)) {
                postStartCallbackErrorOrReturn(seekCallback, NearlinkSeekCallbackConstants.SEEK_FAILED_ALREADY_STARTED);
                return;
            }
            INearlinkDiscovery iNearlinkDiscovery;
            try {
                iNearlinkDiscovery = mNearlinkServiceManage.getNearlinkDiscovery();
            } catch (RemoteException e) {
                iNearlinkDiscovery = null;
            }
            if (iNearlinkDiscovery == null) {
                postStartCallbackErrorOrReturn(seekCallback, NearlinkSeekCallbackConstants.SEEK_FAILED_INTERNAL_ERROR);
                return;
            }

            NearlinkSeekCallbackWrapper nearlinkSeekCallbackWrapper = new NearlinkSeekCallbackWrapper(nearlinkSeekParams,
                    iNearlinkDiscovery, seekCallback, filters);

            nearlinkSeekCallbackWrapper.start();

        }
    }

    /**
     * 停止扫描
     *
     * @param seekCallback 与启动seek同一个callback
     */
    public void stopSeek(NearlinkSeekCallback seekCallback) {
        NearlinkUtils.checkAdapterStateOn(mNearlinkAdapter);
        synchronized (mScanClients) {
            NearlinkSeekCallbackWrapper wrapper = mScanClients.remove(seekCallback);
            if (wrapper == null) {
                if (DBG) {
                    Log.d(TAG, "could not find callback wrapper");
                }
                return;
            }
            wrapper.stop();
        }
    }

    private void postStartCallbackErrorOrReturn(final NearlinkSeekCallback callback, final int errorCode) {
        if (callback == null) {
            Log.e(TAG, "postStartCallbackErrorOrReturn callback is null");
        } else {
            postStartCallbackError(callback, errorCode);
        }
    }

    private void postStartCallbackError(final NearlinkSeekCallback callback, final int errorCode) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callback.onSeekStartedFailure(errorCode);
            }
        });
    }


    private void postStopCallbackErrorOrReturn(final NearlinkSeekCallback callback, final int errorCode) {
        if (callback == null) {
            Log.e(TAG, "postStopCallbackErrorOrReturn callback is null");
        } else {
            postStopCallbackError(callback, errorCode);
        }
    }

    private void postStopCallbackError(final NearlinkSeekCallback callback, final int errorCode) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callback.onSeekStoppedFailure(errorCode);
            }
        });
    }

    /**
     * aidl 包装类
     */
    private class NearlinkSeekCallbackWrapper extends INearlinkSeekCallback.Stub {

        String seekId;
        NearlinkSeekParams nearlinkSeekParams;
        INearlinkDiscovery iNearlinkDiscovery;
        NearlinkSeekCallback nearlinkSeekCallback;
        List<NearlinkSeekFilter> filters;

        public NearlinkSeekCallbackWrapper(NearlinkSeekParams nearlinkSeekParams, INearlinkDiscovery iNearlinkDiscovery, NearlinkSeekCallback nearlinkSeekCallback, List<NearlinkSeekFilter> filters) {
            this.seekId = UUID.randomUUID().toString();
            this.nearlinkSeekParams = nearlinkSeekParams;
            this.iNearlinkDiscovery = iNearlinkDiscovery;
            this.nearlinkSeekCallback = nearlinkSeekCallback;
            this.filters = filters;
        }

        /**
         * 启动扫描
         */
        public void start() {
            synchronized (this) {
                try {
                    iNearlinkDiscovery.startSeek(seekId, nearlinkSeekParams, filters, this);
                    mScanClients.put(nearlinkSeekCallback, this);
                } catch (RemoteException e) {
                    Log.e(TAG, "application start exception", e);
                    postStartCallbackErrorOrReturn(nearlinkSeekCallback, NearlinkSeekCallbackConstants.SEEK_FAILED_INTERNAL_ERROR);
                }
            }
        }

        /**
         * 停止扫描
         */
        public void stop() {
            try {
                iNearlinkDiscovery.stopSeek(seekId);
            } catch (RemoteException e) {
                Log.e(TAG, "application stop exception", e);
                postStopCallbackErrorOrReturn(nearlinkSeekCallback, NearlinkSeekCallbackConstants.SEEK_FAILED_INTERNAL_ERROR);
            }
        }

        @Override
        public void onSeekStarted(int status) {

            if (status == NearlinkSeekCallbackConstants.SEEK_SUCCESS) {
                nearlinkSeekCallback.onSeekStartedSuccess();
                return;
            }

            nearlinkSeekCallback.onSeekStartedFailure(status);
        }

        @Override
        public void onSeekStopped(int status) {
            mScanClients.remove(nearlinkSeekCallback);
            if (status == NearlinkSeekCallbackConstants.SEEK_SUCCESS) {
                nearlinkSeekCallback.onSeekStoppedSuccess();
                return;
            }
            nearlinkSeekCallback.onSeekStoppedFailure(status);
        }

        @Override
        public void onSeekResult(NearlinkSeekResultInfo nearlinkSeekResultInfo) {
            nearlinkSeekCallback.onResult(nearlinkSeekResultInfo);
        }
    }


}
