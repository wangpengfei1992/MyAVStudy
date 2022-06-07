## FfmpegDemo

### 简介

用于日常基于Android中使用Ffmpeg的练习



### 项目环境

```
targetSdk 31
```

```
https\://services.gradle.org/distributions/gradle-7.2-bin.zip
```



```
dependencies {

    implementation 'androidx.core:core-ktx:1.7.0'
    implementation 'androidx.appcompat:appcompat:1.4.1'
    implementation 'com.google.android.material:material:1.6.0'
    implementation 'androidx.constraintlayout:constraintlayout:2.1.3'
    testImplementation 'junit:junit:4.13.2'
    androidTestImplementation 'androidx.test.ext:junit:1.1.3'
    androidTestImplementation 'androidx.test.espresso:espresso-core:3.4.0'
}
```



```
plugins {
    id 'com.android.application' version '7.1.3' apply false
    id 'com.android.library' version '7.1.3' apply false
    id 'org.jetbrains.kotlin.android' version '1.5.30' apply false
}
```



Android Studio版本 ： BumbleBee: 2021.1.1 Patch 3

Ffmpeg版本：4.4