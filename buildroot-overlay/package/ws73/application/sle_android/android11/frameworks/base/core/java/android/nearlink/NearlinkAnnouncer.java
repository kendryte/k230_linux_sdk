/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */
package android.nearlink;

import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;

import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * 广播管理器
 *
 * @since 2023-12-04
 */

public class NearlinkAnnouncer {

    private static final String TAG = "NearlinkAnnouncer";

    /**
     * ANNOUNCE_LEVEL 数据长度
     */
    private static final int ANNOUNCE_LEVEL_LEN = 3;

    private final Handler mHandler;

    private final INearlinkManager mNearlinkServiceManage;

    private NearlinkAdapter mNearlinkAdapter;

    private final Map<NearlinkAnnounceCallback, NearlinkAnnounceCallbackWrapper> mLegacyAnnouncers = new HashMap<>();

    private final Map<NearlinkAnnounceCallbackWrapper, INearlinkAnnounceCallback> mCallbackWrappers = Collections.synchronizedMap(new HashMap<>());

    /**
     * @hide
     */
    public NearlinkAnnouncer(INearlinkManager iNearlinkManager) {
        this.mNearlinkAdapter = NearlinkAdapter.getDefaultAdapter();
        this.mNearlinkServiceManage = iNearlinkManager;
        this.mHandler = new Handler(Looper.getMainLooper());
    }

    /**
     * @param settings
     * @param nearlinkAnnounceData
     * @param nearlinkSeekRespData
     * @param nearlinkAnnounceCallback
     * @return
     */
    public boolean startAnnounce(NearlinkAnnounceSettings settings, NearlinkPublicData nearlinkAnnounceData,
                                 NearlinkPublicData nearlinkSeekRespData, NearlinkAnnounceCallback nearlinkAnnounceCallback) {
        synchronized (mLegacyAnnouncers) {
            // check state on
            NearlinkUtils.checkAdapterStateOn(mNearlinkAdapter);
            if (nearlinkAnnounceCallback == null) {
                throw new IllegalArgumentException("callback cannot be null");
            }
            if (mLegacyAnnouncers.get(nearlinkAnnounceCallback) != null) {
                Log.e(TAG, "NearlinkAnnounceCallback is exist!");
                postStartedFailure(nearlinkAnnounceCallback, NearlinkAnnounceCallbackConstants.ANNOUNCE_CALLBACK_DUPLICATE);
                return false;
            }
            int timeoutMillis = settings.getTimeoutMillis();
            if (timeoutMillis < 0 || timeoutMillis > NearlinkAnnounceSettings.MAX_TIMEOUT_MILLIS) {
                throw new IllegalArgumentException("timeoutMillis out of range: " + timeoutMillis);
            }
            byte[] announceData = nearlinkAnnounceData.getData();
            byte[] seekRespData = nearlinkSeekRespData.getData();
            if (isPublicDataExceedMaxLen(announceData, seekRespData)) {
                throw new IllegalArgumentException("public data total exceed max len"
                        + ((NearlinkAdapter.MAX_PUBLIC_DATA_LEN - ANNOUNCE_LEVEL_LEN
                        - NearlinkAdapter.MAX_LOCAL_DEVICE_NAME_LEN)));
            }
            NearlinkAnnounceCallbackWrapper wrapped = wrapToNearlinkAnnounceCallbackWrapper(nearlinkAnnounceCallback);
            mLegacyAnnouncers.put(nearlinkAnnounceCallback, wrapped);
            NearlinkAnnounceParam nearlinkAnnounceParam = NearlinkAnnounceParam.builder()
                    .announceMode(settings.getAnnounceMode())
                    .build();
            return startAnnounce(timeoutMillis, nearlinkAnnounceParam, nearlinkAnnounceData,
                    nearlinkSeekRespData, wrapped, new Handler(Looper.getMainLooper()));

        }
    }

    /**
     * @hide
     */
    private boolean startAnnounce(int timeoutMillis,
                                  NearlinkAnnounceParam nearlinkAnnounceParam,
                                  NearlinkPublicData nearlinkAnnounceData,
                                  NearlinkPublicData nearlinkSeekRespData,
                                  NearlinkAnnounceCallbackWrapper callbackWrapper, Handler handler) {

        INearlinkDiscovery iNearlinkDiscovery;

        try {
            iNearlinkDiscovery = mNearlinkServiceManage.getNearlinkDiscovery();
        } catch (RemoteException e) {
            Log.e(TAG, "Failed to get iNearlinkDiscovery ", e);
            postCallbackWrapperStartedFailure(handler, callbackWrapper,
                    NearlinkAnnounceCallbackConstants.ANNOUNCE_FAILED_INTERNAL_ERROR);
            return false;
        }

        INearlinkAnnounceCallback iNearlinkAnnounceCallback = wrapToINearlinkAnnounceCallback(callbackWrapper, handler);

        if (mCallbackWrappers.putIfAbsent(callbackWrapper, iNearlinkAnnounceCallback) != null) {
            throw new IllegalArgumentException("callback instance already associated with announce");
        }

        try {
            iNearlinkDiscovery.startAnnounce(timeoutMillis, nearlinkAnnounceParam,
                    nearlinkAnnounceData, nearlinkSeekRespData, iNearlinkAnnounceCallback);
        } catch (RemoteException e) {
            Log.e(TAG, "Failed to start advertising set - ", e);
            postCallbackWrapperStartedFailure(handler, callbackWrapper, NearlinkAnnounceCallbackConstants.ANNOUNCE_FAILED_INTERNAL_ERROR);
            return false;
        }

        return true;

    }

    /**
     * 停止广播
     *
     * @param nearlinkAnnounceCallback 启动广播传入的那个callback
     */
    public void stopAnnounce(NearlinkAnnounceCallback nearlinkAnnounceCallback) {
        NearlinkUtils.checkAdapterStateOn(mNearlinkAdapter);
        synchronized (mLegacyAnnouncers) {
            if (nearlinkAnnounceCallback == null) {
                Log.e(TAG, "stopAnnounce NearlinkAnnounceCallback can not null ");
                throw new IllegalArgumentException("callback cannot be null");
            }
            NearlinkAnnounceCallbackWrapper wrapper = mLegacyAnnouncers.get(nearlinkAnnounceCallback);
            if (wrapper == null) {
                Log.e(TAG, "NearlinkAnnounceCallbackWrapper is null from mLegacyAnnouncers,return");
                return;
            }
            stopAnnounce(wrapper);
            mLegacyAnnouncers.remove(nearlinkAnnounceCallback);
        }
    }

    /**
     * @hide
     */
    private void stopAnnounce(NearlinkAnnounceCallbackWrapper wrapper) {
        if (wrapper == null) {
            throw new IllegalArgumentException("NearlinkAnnounceCallbackWrapper cannot be null");
        }
        INearlinkAnnounceCallback wrapped = mCallbackWrappers.remove(wrapper);
        if (wrapped == null) {
            Log.e(TAG, "INearlinkAnnounceCallback is null from mCallbackWrappers,return");
            return;
        }
        INearlinkDiscovery iNearlinkDiscovery;
        try {
            iNearlinkDiscovery = mNearlinkServiceManage.getNearlinkDiscovery();
            iNearlinkDiscovery.stopAnnounce(wrapped);
        } catch (RemoteException e) {
            Log.e(TAG, "INearlinkDiscovery Failed to stopAnnounce - ", e);
        }
    }

    /**
     * @hide
     */
    private NearlinkAnnounceCallbackWrapper wrapToNearlinkAnnounceCallbackWrapper(NearlinkAnnounceCallback nearlinkAnnounceCallback) {

        return new NearlinkAnnounceCallbackWrapper() {

            @Override
            public void onAnnounceEnabled(int status) {

                if (NearlinkAnnounceCallbackConstants.ANNOUNCE_SUCCESS == status) {
                    postStartedSuccess(nearlinkAnnounceCallback);
                    return;
                }

                postStartedFailure(nearlinkAnnounceCallback, status);

            }

            @Override
            public void onAnnounceDisabled(int status) {

                if (NearlinkAnnounceCallbackConstants.ANNOUNCE_SUCCESS == status) {
                    postStoppedSuccess(nearlinkAnnounceCallback);
                    return;
                }

                postStoppedFailure(nearlinkAnnounceCallback, status);

            }

            @Override
            public void onAnnounceTerminaled() {
                Log.e(TAG, "AnnounceCallbackWrapper onAnnounceTerminaled ");
                postTerminaled(nearlinkAnnounceCallback);

            }
        };
    }

    /**
     * @hide
     */
    private INearlinkAnnounceCallback wrapToINearlinkAnnounceCallback(NearlinkAnnounceCallbackWrapper nearlinkAnnounceCallbackWrapper, Handler handler) {

        return new INearlinkAnnounceCallback.Stub() {
            @Override
            public void onAnnounceEnabled(int announceId, int status) throws RemoteException {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (status != NearlinkAnnounceCallbackConstants.ANNOUNCE_SUCCESS) {
                            nearlinkAnnounceCallbackWrapper.onAnnounceEnabled(status);
                            mCallbackWrappers.remove(nearlinkAnnounceCallbackWrapper);
                            removeLegacyAnnouncers(nearlinkAnnounceCallbackWrapper);
                            return;
                        }
                        nearlinkAnnounceCallbackWrapper.setAnnounceId(announceId);
                        nearlinkAnnounceCallbackWrapper.onAnnounceEnabled(status);
                    }
                });
            }

            @Override
            public void onAnnounceDisabled(int announceId, int status) throws RemoteException {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        mCallbackWrappers.remove(nearlinkAnnounceCallbackWrapper);
                        removeLegacyAnnouncers(nearlinkAnnounceCallbackWrapper);
                        nearlinkAnnounceCallbackWrapper.onAnnounceDisabled(status);
                    }
                });
            }

            @Override
            public void onAnnounceTerminaled(int announceId) throws RemoteException {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        Log.e(TAG, "INearlinkAnnounceCallback.Stub onAnnounceTerminaled ");
                        mCallbackWrappers.remove(nearlinkAnnounceCallbackWrapper);
                        removeLegacyAnnouncers(nearlinkAnnounceCallbackWrapper);
                        nearlinkAnnounceCallbackWrapper.onAnnounceTerminaled();
                    }
                });
            }
        };
    }

    private void removeLegacyAnnouncers(NearlinkAnnounceCallbackWrapper removeWrapper) {
        synchronized (mLegacyAnnouncers) {

            Iterator<Map.Entry<NearlinkAnnounceCallback, NearlinkAnnounceCallbackWrapper>> legacyAnnouncersIt = mLegacyAnnouncers.entrySet().iterator();

            while (legacyAnnouncersIt.hasNext()) {
                Map.Entry<NearlinkAnnounceCallback, NearlinkAnnounceCallbackWrapper> entry = legacyAnnouncersIt.next();
                NearlinkAnnounceCallbackWrapper wrapper = entry.getValue();
                if (removeWrapper != wrapper) {
                    continue;
                }
                legacyAnnouncersIt.remove();
                break;
            }
        }
    }

    private void postStartedSuccess(final NearlinkAnnounceCallback callback) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callback.onStartedSuccess();
            }
        });
    }

    private void postStartedFailure(final NearlinkAnnounceCallback callback, final int error) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callback.onStartedFailure(error);
            }
        });
    }

    private void postStoppedSuccess(final NearlinkAnnounceCallback callback) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callback.onStoppedSuccess();
            }
        });
    }

    private void postStoppedFailure(final NearlinkAnnounceCallback callback, final int error) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callback.onStoppedFailure(error);
            }
        });
    }

    private void postTerminaled(final NearlinkAnnounceCallback callback) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Log.e(TAG, "exe Runnable postTerminaled");
                callback.onTerminaled();
            }
        });
    }


    private void postCallbackWrapperStartedFailure(Handler mHandler, final NearlinkAnnounceCallbackWrapper callbackWrapper, final int error) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                callbackWrapper.onAnnounceEnabled(error);
            }
        });
    }

    private boolean isPublicDataExceedMaxLen(byte[] announceData, byte[] seekRespData) {
        int announceDataLen = 0;
        int seekRespDataLen = 0;
        if (announceData != null) {
            announceDataLen = announceData.length;
        }
        if (seekRespData != null) {
            seekRespDataLen = seekRespData.length;
        }

        if (announceDataLen + seekRespDataLen
                + ANNOUNCE_LEVEL_LEN
                + NearlinkAdapter.MAX_LOCAL_DEVICE_NAME_LEN
                <= NearlinkAdapter.MAX_PUBLIC_DATA_LEN) {
            return false;
        }

        Log.e(TAG, "announceData.len = " + announceDataLen
                + " + seekRespData.len = " + seekRespDataLen
                + " exceed max len"
                + (NearlinkAdapter.MAX_PUBLIC_DATA_LEN - NearlinkAdapter.MAX_LOCAL_DEVICE_NAME_LEN - ANNOUNCE_LEVEL_LEN));
        return true;


    }

}