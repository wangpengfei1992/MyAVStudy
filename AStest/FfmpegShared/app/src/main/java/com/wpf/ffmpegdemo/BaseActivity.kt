package com.wpf.ffmpegdemo

import android.os.Bundle
import android.os.Environment
import android.view.LayoutInflater
import androidx.appcompat.app.AppCompatActivity
import androidx.viewbinding.ViewBinding
import java.lang.reflect.ParameterizedType

/**
 *  Author: feipeng.wang
 *  Time:   2022/6/6
 *  Description : This is description.
 */
abstract class BaseActivity<VB: ViewBinding> : AppCompatActivity() {
    val TAG:String by lazy { this::class.java.simpleName }
    val RootPath:String by lazy { Environment.getExternalStoragePublicDirectory("").toString() + "/ffmpeg/" }

    var mContext:AppCompatActivity ?= null
    val binding: VB by lazy {
        //使用反射得到viewbinding的class
        val type = javaClass.genericSuperclass as ParameterizedType
        val aClass = type.actualTypeArguments[0] as Class<*>
        val method = aClass.getDeclaredMethod("inflate", LayoutInflater::class.java)
        method.invoke(null, layoutInflater) as VB
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(binding.root)
        mContext = this
        init()
    }

    abstract fun init()
}