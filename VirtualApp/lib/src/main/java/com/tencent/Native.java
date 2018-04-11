//
// Decompiled by Jadx - 296ms
//
package com.tencent;

import android.app.Application;
import android.util.Log;


public class Native {
    private static final String TAG = "Tencent-Native";

//    static {
//        try {
//            System.loadLibrary("tencent");
//        } catch (Throwable e) {
//            Log.e(TAG, e.getMessage(), e);
//        }
//    }

    public static native void hook(String p,String s);
}
