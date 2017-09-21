cmake_minimum_required(VERSION 2.8)
project( main )

find_package( OpenCV )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( include )

add_library(tm_usage src/tm_usage.cpp)
add_library(mylib STATIC src/util.cpp src/blend.cpp src/warp.cpp src/feature.cpp src/harris.cpp src/moravec.cpp src/sift.cpp)

add_executable( main src/main.cpp )
target_link_libraries( main ${OpenCV_LIBS} mylib tm_usage)

