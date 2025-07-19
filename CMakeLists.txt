cmake_minimum_required(VERSION 3.31)
project(figuras)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(figuras main.cpp src/Figure.cpp)
target_link_libraries(figuras ${OpenCV_LIBS})
target_include_directories(figuras PRIVATE ${OpenCV_INCLUDE_DIRS})
