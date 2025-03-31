/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.nearlink.nlservice;

import android.os.Message;

import android.nearlink.NearlinkAdapter;
import android.util.Log;

import com.android.internal.util.State;
import com.android.internal.util.StateMachine;

/**
 * 状态机
 *
 * @since 2023-12-04
 */
public class NearlinkState extends StateMachine {

    private static final boolean DBG = true;
    private static final String TAG = NearlinkState.class.getSimpleName();

    static final int NEARLINK_TURN_ON = 3;
    static final int NEARLINK_TURN_OFF = 4;
    static final int NEARLINK_STARTED = 7;
    static final int NEARLINK_STOPPED = 8;
    static final int NEARLINK_STOP_TIMEOUT = 11;
    static final int NEARLINK_START_TIMEOUT = 12;
    static final int NEARLINK_START_TIMEOUT_DELAY = 4000;
    static final int NEARLINK_STOP_TIMEOUT_DELAY = 1000;

    private int mPrevState = NearlinkAdapter.STATE_OFF;
    private AdapterService mAdapterService;
    private TurningOnState mTurningOnState = new TurningOnState();
    private TurningOffState mTurningOffState = new TurningOffState();
    private OnState mOnState = new OnState();
    private OffState mOffState = new OffState();

    private NearlinkState(AdapterService mAdapterService) {
        super(TAG);
        addState(mOffState);
        addState(mOnState);
        addState(mTurningOnState);
        addState(mTurningOffState);
        this.mAdapterService = mAdapterService;
        setInitialState(mOffState);
    }

    public static NearlinkState make(AdapterService service) {
        Log.d(TAG, "make() - Creating NearlinkState");
        NearlinkState as = new NearlinkState(service);
        as.start();
        return as;
    }

    public void doQuit() {
        quitNow();
    }

    private void cleanup() {
        if (mAdapterService != null) {
            mAdapterService = null;
        }
    }

    @Override
    protected void onQuitting() {
        cleanup();
    }

    @Override
    protected String getLogRecString(Message msg) {
        return messageString(msg.what);
    }

    private abstract class BaseAdapterState extends State {

        abstract int getStateValue();

        @Override
        public void enter() {
            Log.e(TAG, "BaseAdapterState enter");
            int currState = getStateValue();
            Log.e(TAG, NearlinkAdapter.nameForState(getStateValue()) + " PrevState : " + mPrevState + " currState " + currState);
            mAdapterService.updateAdapterState(mPrevState, currState);
            mPrevState = currState;
        }

        void infoLog(String msg) {
            Log.e(TAG, NearlinkAdapter.nameForState(getStateValue()) + " : " + msg);
        }

        void errorLog(String msg) {
            Log.e(TAG, NearlinkAdapter.nameForState(getStateValue()) + " : " + msg);
        }
    }

    private String messageString(int message) {
        switch (message) {
            case NEARLINK_TURN_ON: return "NEARLINK_TURN_ON";
            case NEARLINK_TURN_OFF: return "NEARLINK_TURN_OFF";
            case NEARLINK_STARTED: return "NEARLINK_STARTED";
            case NEARLINK_STOPPED: return "NEARLINK_STOPPED";
            case NEARLINK_STOP_TIMEOUT: return "NEARLINK_STOP_TIMEOUT";
            case NEARLINK_START_TIMEOUT: return "NEARLINK_START_TIMEOUT";
            case NEARLINK_START_TIMEOUT_DELAY: return "NEARLINK_START_TIMEOUT_DELAY";
            case NEARLINK_STOP_TIMEOUT_DELAY: return "NEARLINK_STOP_TIMEOUT_DELAY";
            default: return "Unknown message (" + message + ")";
        }
    }

    private class OffState extends BaseAdapterState {

        @Override
        int getStateValue() {
            return NearlinkAdapter.STATE_OFF;
        }

        @Override
        public boolean processMessage(Message msg) {

            infoLog("at OffState do processMessage- " + messageString(msg.what));

            switch (msg.what) {

                case NEARLINK_TURN_ON:
                    transitionTo(mTurningOnState);
                    break;

                default:
                    infoLog("OffState Unhandled message - " + messageString(msg.what));
                    return false;
            }
            return true;
        }
    }

    private class TurningOnState extends BaseAdapterState {

        @Override
        int getStateValue() {
            return NearlinkAdapter.STATE_TURNING_ON;
        }

        @Override
        public void enter() {
            super.enter();
            sendMessageDelayed(NEARLINK_START_TIMEOUT, NEARLINK_START_TIMEOUT_DELAY);
            // try to enable nl
            infoLog("NearlinkState try to startNearlink");
            mAdapterService.startNearlink();
        }

        @Override
        public void exit() {
            removeMessages(NEARLINK_START_TIMEOUT);
            super.exit();
        }

        @Override
        public boolean processMessage(Message msg) {

            infoLog("at TurningOnState do processMessage- " + messageString(msg.what));

            switch (msg.what) {
                case NEARLINK_STARTED:
                    transitionTo(mOnState);
                    break;

                case NEARLINK_START_TIMEOUT:
                    errorLog(messageString(msg.what));
                    transitionTo(mTurningOffState);
                    break;

                default:
                    infoLog("TurningOnState Unhandled message - " + messageString(msg.what));
                    return false;
            }
            return true;
        }
    }

    private class OnState extends BaseAdapterState {

        @Override
        int getStateValue() {
            return NearlinkAdapter.STATE_ON;
        }

        @Override
        public boolean processMessage(Message msg) {

            infoLog("at OnState do processMessage- " + messageString(msg.what));

            switch (msg.what) {
                case NEARLINK_TURN_OFF:
                    transitionTo(mTurningOffState);
                    break;

                default:
                    infoLog("OnState Unhandled message - " + messageString(msg.what));
                    return false;
            }
            return true;
        }
    }


    private class TurningOffState extends BaseAdapterState {

        @Override
        int getStateValue() {
            return NearlinkAdapter.STATE_TURNING_OFF;
        }

        @Override
        public void enter() {
            super.enter();
            sendMessageDelayed(NEARLINK_STOP_TIMEOUT, NEARLINK_STOP_TIMEOUT_DELAY);
            mAdapterService.stopNearlinkNative();
        }

        @Override
        public void exit() {
            removeMessages(NEARLINK_STOP_TIMEOUT);
            super.exit();
        }

        @Override
        public boolean processMessage(Message msg) {

            infoLog("at TurningOffState do processMessage- " + messageString(msg.what));

            switch (msg.what) {
                case NEARLINK_STOPPED:
                    transitionTo(mOffState);
                    break;

                case NEARLINK_STOP_TIMEOUT:
                    errorLog(messageString(msg.what));
                    transitionTo(mOffState);
                    break;

                default:
                    infoLog("TurningOffState Unhandled message - " + messageString(msg.what));
                    return false;
            }
            return true;
        }
    }


}
