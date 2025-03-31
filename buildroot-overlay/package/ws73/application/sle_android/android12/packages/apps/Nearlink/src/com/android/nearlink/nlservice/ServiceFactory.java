/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import com.android.nearlink.connection.ConnectionService;
import com.android.nearlink.hid.HidHostService;

/**
 * 简单工厂
 *
 * @since 2023-12-04
 */
public class ServiceFactory {
    /**
     * 简单工厂读取HidHostService
     *
     * @return HidHostService
     */
    public HidHostService getHidHostService() {
        return HidHostService.getHidHostService();
    }

    /**
     * 简单工厂读取ConnectionService
     *
     * @return ConnectionService
     */
    public ConnectionService getConnectionService() {
        return ConnectionService.getConnectionService();
    }
}
