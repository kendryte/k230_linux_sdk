/*
 * Copyright (c) @CompanyNameMagicTag 2023. All rights reserved.
 */

package com.nearlink.demo;

/**
 * Filter to max the length of a Bluetotoh device name to 248 bytes, as defined by the spec.
 */
public class NearlinkLengthDeviceNameFilter extends Utf8ByteLengthFilter {
    private static final int NEARLINK_NAME_MAX_LENGTH_BYTES = 248;

    public NearlinkLengthDeviceNameFilter() {
        super(NEARLINK_NAME_MAX_LENGTH_BYTES);
    }
}
