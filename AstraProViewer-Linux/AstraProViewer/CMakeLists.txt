cmake_minimum_required(VERSION 3.5.1)
project(AstraProViewer)

set(CMAKE_CXX_STANDARD 11)

include_directories(./3rdparty/OpenNI2/Include)
link_libraries(
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/OpenNI2/Redist/Linux/x64/libOpenNI2.so
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/OpenNI2/Redist/Linux/x64/OpenNI2/Drivers/liborbbec.so
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/OpenNI2/Redist/Linux/x64/OpenNI2/Drivers/libOniFile.so
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/OpenNI2/Redist/Linux/x64/OpenNI2/Drivers/libPSLink.so)

find_package( OpenCV REQUIRED )

set(SOURCE_FILES main.cpp RGB-D-calibration-sample-code.cpp CIniFile.cpp)
add_executable(AstraProViewer ${SOURCE_FILES})
target_link_libraries(AstraProViewer ${OpenCV_LIBS})
