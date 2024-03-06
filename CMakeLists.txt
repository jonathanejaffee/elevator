cmake_minimum_required(VERSION 3.28.1)
project(Elevator)
set(CMAKE_BUILD_TYPE Debug) #temp
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
#add_subdirectory(./src)
file(GLOB SOURCES ./src/*.cpp)
add_executable(elevator ${SOURCES})
target_link_libraries( elevator ${OpenCV_LIBS} )