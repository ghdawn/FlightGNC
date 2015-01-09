cmake_minimum_required(VERSION 2.8.4)
project(PosEstimate)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES main.cpp observe.cpp laserurg.cpp)
add_executable(PosEstimate ${SOURCE_FILES})

target_link_libraries( PosEstimate "${PosEstimate_SOURCE_DIR}/../../iTRLib/itrbase/bin/debug/libitrbase.a" )
target_link_libraries( PosEstimate "${PosEstimate_SOURCE_DIR}/../../iTRLib/itrvision/bin/debug/libitrvision.a" )
target_link_libraries( PosEstimate "${PosEstimate_SOURCE_DIR}/../../iTRLib/itralgorithm/bin/debug/libitralgorithm.a" )
target_link_libraries( PosEstimate "${PosEstimate_SOURCE_DIR}/../../iTRLib/itrsystem/bin/debug/libitrsystem.a" )
target_link_libraries( PosEstimate "${PosEstimate_SOURCE_DIR}/../../iTRLib/itrdevice/bin/debug/libitrdevice.a" )
target_link_libraries( PosEstimate "${PosEstimate_SOURCE_DIR}/../../iTRLib/ix264/bin/debug/libix264.a" )
target_link_libraries(PosEstimate "pthread")

include_directories( "${PosEstimate_SOURCE_DIR}/../../iTRLib/3rdparty/alglib")
include_directories( "${PosEstimate_SOURCE_DIR}/../../iTRLib/itrbase")
include_directories("${PosEstimate_SOURCE_DIR}/../../iTRLib/itrvision")
include_directories("${PosEstimate_SOURCE_DIR}/../../iTRLib/itralgorithm")
include_directories("${PosEstimate_SOURCE_DIR}/../../iTRLib/itrsystem")
include_directories("${PosEstimate_SOURCE_DIR}/../../iTRLib/itrdevice")
include_directories("${PosEstimate_SOURCE_DIR}/../../iTRLib/ix264")