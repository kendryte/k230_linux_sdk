/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

package com.android.settings.nearlink;

import android.content.Context;
import android.util.AttributeSet;

import com.android.settings.ProgressCategory;
import com.android.settings.R;

/**
 * A Nearlink discovery progress category
 *
 * @since 2023-12-04
 */
public class NearlinkProgressCategory extends ProgressCategory {
    public NearlinkProgressCategory(Context context) {
        super(context);
        init();
    }

    public NearlinkProgressCategory(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public NearlinkProgressCategory(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    public NearlinkProgressCategory(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    private void init() {
        setEmptyTextRes(R.string.nearlink_no_devices_found);
    }
}
