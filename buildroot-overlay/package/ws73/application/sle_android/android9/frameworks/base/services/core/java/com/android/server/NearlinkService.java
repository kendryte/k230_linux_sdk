/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.server;

import android.content.Context;

import android.nearlink.NearlinkAdapter;

class NearlinkService extends SystemService {
    private NearlinkManagerService mNearlinkManagerService;

    public NearlinkService(Context context) {
        super(context);
        mNearlinkManagerService = new NearlinkManagerService(context);
    }

    @Override
    public void onStart() {
    }

    @Override
    public void onBootPhase(int phase) {
        if (phase == SystemService.PHASE_SYSTEM_SERVICES_READY) {
            publishBinderService(NearlinkAdapter.NEARLINK_MANAGER_SERVICE, mNearlinkManagerService);
        } else if (phase == SystemService.PHASE_ACTIVITY_MANAGER_READY) {
            mNearlinkManagerService.handleOnBootPhase();
        }
    }

    @Override
    public void onUnlockUser(int userHandle) {
        mNearlinkManagerService.handleOnUnlockUser(userHandle);
    }
}
