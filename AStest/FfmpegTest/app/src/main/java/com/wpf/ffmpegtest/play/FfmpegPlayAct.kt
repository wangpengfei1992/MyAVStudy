package com.wpf.ffmpegtest.play

import android.Manifest
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.Settings
import android.util.Log
import androidx.annotation.Nullable
import com.tbruyelle.rxpermissions3.Permission
import com.tbruyelle.rxpermissions3.RxPermissions
import com.wpf.ffmpegtest.BaseActivity
import com.wpf.ffmpegtest.FfmpegTools
import com.wpf.ffmpegtest.databinding.ActFfmpegplayBinding
import java.io.File
import java.io.IOException


/**
 *  Author: feipeng.wang
 *  Time:   2022/6/6
 *  Description : This is description.
 */
class FfmpegPlayAct : BaseActivity<ActFfmpegplayBinding>(){
    private val REQUEST_PERMISSION_CODE = 100

    override fun init() {
        binding.btPlay.setOnClickListener {
            val rxPermissions : RxPermissions =  RxPermissions(this)
            rxPermissions
                .requestEach(
/*                    Manifest.permission.CAMERA,
                    Manifest.permission.READ_PHONE_STATE,*/
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                )
                .subscribe { permission: Permission ->  // will emit 2 Permission objects
                    if (permission.granted) {
                        // `permission.name` is granted !
                        Log.e(TAG,"-----------granted")
                    } else if (permission.shouldShowRequestPermissionRationale) {
                        // Denied permission without ask never again
                        Log.e(TAG,"-----------shouldShowRequestPermissionRationale")
                    } else {
                        // Denied permission with ask never again
                        // Need to go to the settings
                        Log.e(TAG,"-----------Need to go to the settings")
                        goToOpenSetting()
                    }
                }
        }
    }
    fun goToOpenSetting(){
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            // 适配android11读写权限
            if (Environment.isExternalStorageManager()) {
                //已获取android读写权限
                Log.e(TAG,"-----------已获取android读写权限")
                readFileInStore()
            } else {
                val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION)
                intent.data = Uri.parse("package:$packageName")
                startActivityForResult(intent, REQUEST_PERMISSION_CODE)
            }
            return
        }
    }
    override fun onActivityResult(requestCode: Int, resultCode: Int, @Nullable data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == REQUEST_PERMISSION_CODE && Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (Environment.isExternalStorageManager()) {
                //已获取android读写权限
                Log.e(TAG,"----onActivityResult-------已获取android读写权限")
            } else {
                //存储权限获取失败
                Log.e(TAG,"----onActivityResult-------存储权限获取失败")
            }
        }
    }
    /*判断是否可以读写文件*/
    private fun readFileInStore(){
        val filePath = RootPath + "play/114215.mp4"
        var file = File(filePath)
        try {
            if (file.exists()) {
                FfmpegTools.native_start_play(filePath,binding.sfvPlayer.holder.surface)
            }
        } catch (e: IOException) {
            e.printStackTrace()
        }

    }
}