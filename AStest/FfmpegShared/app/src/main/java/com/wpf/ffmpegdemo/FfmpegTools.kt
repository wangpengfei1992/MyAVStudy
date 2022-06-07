package com.wpf.ffmpegdemo

import android.view.Surface

/**
 *  Author: feipeng.wang
 *  Time:   2022/6/6
 *  Description : This is description.
 */
object FfmpegTools {

    init {
        System.loadLibrary("ffmpegshared")
    }
    /**
     * A native method that is implemented by the 'ffmpegtest' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    external fun native_start_play(path: String, surface: Surface)
    external fun native_start_play2(path: String, surface: Surface)
}