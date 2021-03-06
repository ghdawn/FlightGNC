cmake_minimum_required(VERSION 2.8.4)
project(ImageServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fpermissive")

set(SOURCE_FILES main.cpp iocontrol.cpp configure.cpp h264_imx.cpp)
add_executable(ImageServer ${SOURCE_FILES})

target_link_libraries( ImageServer "${ImageServer_SOURCE_DIR}/../../iTRLib/itrbase/bin/debug/libitrbase.a" )
target_link_libraries( ImageServer "${ImageServer_SOURCE_DIR}/../../iTRLib/itralgorithm/bin/debug/libitralgorithm.a" )
target_link_libraries( ImageServer "${ImageServer_SOURCE_DIR}/../../iTRLib/itrvision/bin/debug/libitrvision.a" )
target_link_libraries( ImageServer "${ImageServer_SOURCE_DIR}/../../iTRLib/itrsystem/bin/debug/libitrsystem.a" )
target_link_libraries( ImageServer "${ImageServer_SOURCE_DIR}/../../iTRLib/itrdevice/bin/debug/libitrdevice.a" )
target_link_libraries( ImageServer "${ImageServer_SOURCE_DIR}/../../iTRLib/ix264/bin/debug/libix264.a" )

target_link_libraries( ImageServer "avcodec")
target_link_libraries(ImageServer "swscale")
target_link_libraries(ImageServer "avutil")
target_link_libraries(ImageServer "x264")
target_link_libraries(ImageServer "pthread")
target_link_libraries(ImageServer "v4l2")
target_link_libraries(ImageServer "imxvpuapi")

include_directories("${ImageServer_SOURCE_DIR}/../../iTRLib/3rdparty/alglib")
include_directories("${ImageServer_SOURCE_DIR}/../../iTRLib/itrbase")
include_directories("${ImageServer_SOURCE_DIR}/../../iTRLib/itrvision")
include_directories("${ImageServer_SOURCE_DIR}/../../iTRLib/itralgorithm")
include_directories("${ImageServer_SOURCE_DIR}/../../iTRLib/itrsystem")
include_directories("${ImageServer_SOURCE_DIR}/../../iTRLib/itrdevice")
include_directories("${ImageServer_SOURCE_DIR}/../../iTRLib/ix264")
