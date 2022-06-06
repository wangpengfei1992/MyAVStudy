package com.wpf.ffmpegtest


import android.content.Intent
import com.wpf.ffmpegtest.databinding.ActivityMainBinding
import com.wpf.ffmpegtest.play.FfmpegPlayAct

class MainActivity : BaseActivity<ActivityMainBinding>() {

    override fun init() {
        binding.sampleText.text = FfmpegTools.stringFromJNI()
        binding.sampleText.setOnClickListener {
            mContext?.startActivity(Intent(mContext,FfmpegPlayAct::class.java))
        }
    }


}