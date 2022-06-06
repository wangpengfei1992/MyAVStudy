package com.wpf.ffmpegtest

/**
 *  Author: feipeng.wang
 *  Time:   2022/6/6
 *  Description : This is description.
 */
object FfmpegTools {
    /**
     * A native method that is implemented by the 'ffmpegtest' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    init {
        System.loadLibrary("ffmpegtest")
    }
}