package com.wpf.ffmpegdemo


import android.content.Intent
import com.wpf.ffmpegdemo.databinding.ActivityMainBinding
import com.wpf.ffmpegdemo.play.FfmpegPlayAct

class MainActivity : BaseActivity<ActivityMainBinding>() {

    override fun init() {
        binding.sampleText.text = FfmpegTools.stringFromJNI()
        binding.sampleText.setOnClickListener {
            mContext?.startActivity(Intent(mContext, FfmpegPlayAct::class.java))
        }
    }


}