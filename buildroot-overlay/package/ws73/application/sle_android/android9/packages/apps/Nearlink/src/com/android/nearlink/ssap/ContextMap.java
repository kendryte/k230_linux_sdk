/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.ssap;

import android.app.Service;
import android.os.Binder;
import android.os.IBinder;
import android.os.IInterface;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.Objects;
import java.util.UUID;

/**
 * 参考BLE的ContextMap设计，跟踪管理SSAP应用、回调和连接
 *
 * @since 2023-12-01
 *
 * @hide
 */
class ContextMap<C> {
    private static final String TAG = "SleSsap.ContextMap";

    class App {
        public static final int INVALID_CONN_ID = -1;

        public UUID uuid;
        public int id;
        public String address;
        public int connId;
        public String name;
        public C callback;

        public boolean isCacheLocked;
        public Object cache;

        private IBinder.DeathRecipient mDeathRecipient;

        App(UUID uuid, String address, C callback, String name) {
            this.uuid = uuid;
            this.address = address;
            this.connId = INVALID_CONN_ID;
            this.callback = callback;
            this.name = name;

            isCacheLocked = false;
            cache = null;
        }

        void linkToDeath(IBinder.DeathRecipient deathRecipient) {
            // It might not be a binder object
            if (callback == null) {
                return;
            }
            try {
                IBinder binder = ((IInterface) callback).asBinder();
                binder.linkToDeath(deathRecipient, 0);
                mDeathRecipient = deathRecipient;
            } catch (RemoteException e) {
                Log.e(TAG, "Unable to link deathRecipient for app id " + id);
            }
        }

        void unlinkToDeath() {
            if (mDeathRecipient != null) {
                try {
                    IBinder binder = ((IInterface) callback).asBinder();
                    binder.unlinkToDeath(mDeathRecipient, 0);
                } catch (NoSuchElementException e) {
                    Log.e(TAG, "Unable to unlink deathRecipient for app id " + id);
                }
            }
        }
    }

    private List<App> mApps = new ArrayList<App>();

    App add(UUID uuid, String address, C callback, Service service) {
        int appUid = Binder.getCallingUid();
        String appName = service.getPackageManager().getNameForUid(appUid);
        if (appName == null) {
            // Assign an app name if one isn't found
            appName = "Unknown App (UID: " + appUid + ")";
        }
        synchronized (mApps) {
            App app = new App(uuid, address, callback, appName);
            mApps.add(app);
            return app;
        }
    }

    void remove(int id) {
        synchronized (mApps) {
            Iterator<App> i = mApps.iterator();
            while (i.hasNext()) {
                App entry = i.next();
                if (entry.id == id) {
                    entry.unlinkToDeath();
                    i.remove();
                    break;
                }
            }
        }
    }

    List<Integer> getAllAppsIds() {
        List<Integer> appIds = new ArrayList();
        synchronized (mApps) {
            Iterator<App> i = mApps.iterator();
            while (i.hasNext()) {
                App entry = i.next();
                appIds.add(entry.id);
            }
        }
        return appIds;
    }

    App getById(int id) {
        synchronized (mApps) {
            Iterator<App> i = mApps.iterator();
            while (i.hasNext()) {
                App entry = i.next();
                if (entry.id == id) {
                    return entry;
                }
            }
        }
        Log.e(TAG, "Context not found for ID " + id);
        return null;
    }

    App getByUuid(UUID uuid) {
        synchronized (mApps) {
            Iterator<App> i = mApps.iterator();
            while (i.hasNext()) {
                App entry = i.next();
                if (entry.uuid.equals(uuid)) {
                    return entry;
                }
            }
        }
        Log.e(TAG, "Context not found for UUID " + uuid);
        return null;
    }

    List<App> getAppsByAddress(String address) {
        List<App> apps = new ArrayList<>();
        Iterator<App> i = mApps.iterator();
        while (i.hasNext()) {
            App entry = i.next();
            if (Objects.equals(entry.address, address)) {
                apps.add(entry);
            }
        }
        return apps;
    }

    /**
     * Erases all application context entries.
     */
    void clear() {
        synchronized (mApps) {
            Iterator<App> i = mApps.iterator();
            while (i.hasNext()) {
                App entry = i.next();
                entry.unlinkToDeath();
                i.remove();
            }
        }
    }
}
