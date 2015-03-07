cmake_minimum_required(VERSION 2.8.4)
project(RecPlayer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(RecPlayer ${SOURCE_FILES})


target_link_libraries( RecPlayer "${RecPlayer_SOURCE_DIR}/../../iTRLib/itrbase/bin/debug/libitrbase.a" )
target_link_libraries( RecPlayer "${RecPlayer_SOURCE_DIR}/../../iTRLib/itrsystem/bin/debug/libitrsystem.a" )
target_link_libraries( RecPlayer "${RecPlayer_SOURCE_DIR}/../../iTRLib/ix264/bin/debug/libix264.a" )
target_link_libraries(RecPlayer "x264")
target_link_libraries(RecPlayer "pthread")

include_directories( "${RecPlayer_SOURCE_DIR}/../../iTRLib/3rdparty/alglib")
include_directories( "${RecPlayer_SOURCE_DIR}/../../iTRLib/itrbase")
include_directories("${RecPlayer_SOURCE_DIR}/../../iTRLib/itrvision")
include_directories("${RecPlayer_SOURCE_DIR}/../../iTRLib/itralgorithm")
include_directories("${RecPlayer_SOURCE_DIR}/../../iTRLib/itrsystem")
include_directories("${RecPlayer_SOURCE_DIR}/../../iTRLib/itrdevice")
include_directories("${RecPlayer_SOURCE_DIR}/../../iTRLib/ix264")