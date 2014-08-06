cmake_minimum_required(VERSION 2.8.3)
project(synthetic_image_generator)

find_package(VTK REQUIRED)
find_package(OpenCV REQUIRED)
include(${VTK_USE_FILE})
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(affine_trafo src/affine_trafo.cpp)
add_executable(image_generator src/image_generator.cpp)
add_executable(handy_linienextraktor src/handy_linienextraktor.cpp)

target_link_libraries(affine_trafo ${VTK_LIBRARIES})
target_link_libraries(image_generator ${VTK_LIBRARIES})
target_link_libraries(image_generator ${OpenCV_LIBRARIES})
target_link_libraries(handy_linienextraktor ${OpenCV_LIBRARIES})
