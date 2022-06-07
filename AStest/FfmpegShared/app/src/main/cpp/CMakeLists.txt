cmake_minimum_required(VERSION 3.10.2)
project("ffmpegshared")
#导入头文件（可以让项目找到头文件，这样才能调用函数）
include_directories("${CMAKE_SOURCE_DIR}/include")
#配置动态链接库所在的目录
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}../../../../libs/${ANDROID_ABI}")
add_library( # Sets the name of the library.
        ffmpegshared
             SHARED
             native-lib.cpp )

find_library( # Sets the name of the path variable.
              log-lib
              log )
target_link_libraries( # Specifies the target library.
        ffmpegshared
                        #下面这些是链接的库名称（其实就是so文件去掉前面的lib和后面的.so）
                        avcodec-58
                        avdevice-58
                        avfilter-7
                        avformat-58
                        avutil-56
                        postproc-55
                        swresample-3
                        swscale-5
                        android
                        z
                        OpenSLES
                       ${log-lib} )