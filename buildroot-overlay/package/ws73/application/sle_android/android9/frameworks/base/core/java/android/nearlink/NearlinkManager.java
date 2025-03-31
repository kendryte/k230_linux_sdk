/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.annotation.SystemService;
import android.content.Context;
import android.os.RemoteException;
import android.util.Log;

/**
 * NearlinkManager实例，主要用于启动SSAP Server
 *
 * @since 2023-12-04
 * */
// 等Feature好了之后添加：@RequiresFeature(PackageManager.FEATURE_NEARLINK)
@SystemService(Context.NEARLINK_SERVICE)
public class NearlinkManager {
    private static final String TAG = "NearlinkManager";

    private final NearlinkAdapter mAdapter;

    /**
     * 实例化NearlinkManager对象，由系统调用
     *
     * @hide
     */
    public NearlinkManager(Context context) {
        // Legacy api - getDefaultAdapter does not take in the context
        mAdapter = NearlinkAdapter.getDefaultAdapter();
    }

    /**
     * 获取NearlinkAdapter实例
     *
     * @return NearlinkAdapter实例
     */
    public NearlinkAdapter getAdapter() {
        return mAdapter;
    }

    /**
     * 打开SSAP服务端实例
     *
     * @param context  上下文
     * @param callback 服务端的异步回调类
     * @return SSAP服务端实例
     */
    public NearlinkSsapServer openSsapServer(Context context, NearlinkSsapServerCallback callback) {
        if (context == null || callback == null) {
            throw new IllegalArgumentException("null parameter: " + context + " " + callback);
        }

        try {
            INearlinkManager managerService = mAdapter.getNearlinkManager();
            INearlinkSsapServer iSsapServer = managerService.getNearlinkSsapServer();
            if (iSsapServer == null) {
                Log.e(TAG, "Fail to get SSAP Server connection");
                return null;
            }
            NearlinkSsapServer ssapServer = new NearlinkSsapServer(iSsapServer);
            return ssapServer.registerCallback(callback) ? ssapServer : null;
        } catch (RemoteException e) {
            Log.e(TAG, "", e);
            return null;
        }
    }
}
