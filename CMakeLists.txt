cmake_minimum_required(VERSION 2.8)

project(Calibration)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(include)
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_executable(Calibration src/main.cpp)

target_link_libraries(Calibration ${OpenCV_LIBS} DsSdk GdiPlus HCCore HCNetSDK PlayCtrl)




