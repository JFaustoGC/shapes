cmake_minimum_required(VERSION 3.1)
project(figuras)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

include_directories(include)

add_executable(figuras main.cpp src/Figure.cpp src/ImageFigureProvider.cpp src/CsvFigureProvider.cpp)
target_link_libraries(figuras ${OpenCV_LIBS})
target_include_directories(figuras PRIVATE ${OpenCV_INCLUDE_DIRS})
