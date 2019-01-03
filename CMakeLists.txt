

cmake_minimum_required(VERSION 3.10)
project(rim-proyecto)

set(CMAKE_CXX_STANDARD 11)


find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(draw_good_matches draw_good_matches.cpp)
add_executable(create_descriptors create_descriptors.cpp) 
target_link_libraries(draw_good_matches ${OpenCV_LIBS} )
target_link_libraries(create_descriptors ${OpenCV_LIBS} )
