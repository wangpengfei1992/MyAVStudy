# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
#SET(CMAKE_C_FLAGS "-Wl,-z,notext")
#SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -fPIC")
#SET(CMAKE_EXE_LINKER_FLAGS " -no-pie")
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64"OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
    message(STATUS "x86_64 architecture detected - setting flag -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fPIC")
endif()
# Declares and names the project.

project("ffmpegtest")
message(AUTHOR_WARNING  "CMAKE_MODULE_PATH>> ${ANDROID_ABI}")
message(AUTHOR_WARNING  "CMAKE_CURRENT_SOURCE_DIR>> ${CMAKE_CURRENT_SOURCE_DIR}")

#设置路径别名
set(jnilibs ${CMAKE_SOURCE_DIR}/../jniLibs)
message(AUTHOR_WARNING  "jnilibs>> ${jnilibs}")

#包含文件夹
include_directories(include util)
#静态库文件
link_directories(${jnilibs}/${ANDROID_ABI})
message(AUTHOR_WARNING  "link>> ${jnilibs}/${ANDROID_ABI}")

#包含所有cpp文件
file(GLOB src_file ${CMAKE_SOURCE_DIR}/*.cpp)


add_library( # Sets the name of the library.
        ffmpegtest

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${src_file})

find_library( # Sets the name of the path variable.
        log-lib

        log)

set(third-lib
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        )


#导入第三方库
target_link_libraries( # Specifies the target library.
        ffmpegtest

        ${log-lib}
        ${third-lib})

#[[引人静态库]]
#
# 引入静态库
#       ① 参数 1 ( add ) : 设置引入的静态库名称
#       ② 参数 2 ( SHARED ) : 设置引入的函数库类型 : ① 静态库 STATIC ② 动态库 SHARED
#       ③ 参数 3 ( IMPORTED ) : 表示引入第三方静态库 , 导入静态库 , 相当于预编译静态库
#                                   后续还需要设置导入路径 , 配合该配置使用
#[[
add_library(
        # 设置引入的静态库名称
        add

        # 设置引入的函数库类型为静态库
        STATIC

        # 表示引入第三方静态库
        IMPORTED)

        ]]

# 设置上述静态库的导入路径
#       设置目标属性参数 :
#           ① 参数 1 ( add ) : 要设置哪个函数库的属性
#           ② 参数 2 ( PROPERTIES ) : 设置目标属性
#           ③ 参数 3 ( IMPORTED_LOCATION ) : 设置导入路径
#           ④ 参数 4 : 配置静态库的文件路径
#[[
set_target_properties(
        # 设置目标
        add

        # 设置属性
        PROPERTIES

        # 导入路径
        IMPORTED_LOCATION

        # ${CMAKE_SOURCE_DIR} 是本 CMakeList.txt 构建脚本的路径 , 是 CMake 工具内置的变量
        #       Android CMake 也内置了一些变量 , 如 ANDROID_ABI
        ${CMAKE_SOURCE_DIR}/../jniLibs/armeabi-v7a/libadd.a)

        ]]

#设置动态库
#[[
# 设置变量
# CMAKE_CXX_FLAGS 表示会将 C++ 的参数传给编译器
# CMAKE_C_FLAGS 表示会将 C 参数传给编译器

# 参数设置 : 传递 CMAKE_CXX_FLAGS C+= 参数给编译器时 , 在 该参数后面指定库的路径
#   CMAKE_SOURCE_DIR 指的是当前的文件地址
#   -L 参数指定动态库的查找路径
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}")
]]
