/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package android.nearlink;

import android.nearlink.NearlinkAddress;
import android.nearlink.NearlinkAuthInfoEvt;

/**
 * 连接管理回调
 *
 * @since 2023-12-01
 */
interface INearlinkConnectionCallback {
    // 认证完成的回调函数 NearlinkAuthInfoEvt:linkKey,cryptoAlgo,keyDerivAlgo,integrChkInd
    void onAuthCompleteCallback(in NearlinkAddress nearlinkAddress, int connId, in NearlinkAuthInfoEvt authInfo, in int errCode);
}