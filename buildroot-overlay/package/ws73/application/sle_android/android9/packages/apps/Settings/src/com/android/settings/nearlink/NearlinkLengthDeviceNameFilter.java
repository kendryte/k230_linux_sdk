/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

/**
 * Filter to max the length of a Nearlink device name to 248 bytes, as defined by the spec.
 *
 * @since 2023-12-04
 */
public class NearlinkLengthDeviceNameFilter extends Utf8ByteLengthFilter {
    private static final int NEARLINK_NAME_MAX_LENGTH_BYTES = 248;

    public NearlinkLengthDeviceNameFilter() {
        super(NEARLINK_NAME_MAX_LENGTH_BYTES);
    }
}
