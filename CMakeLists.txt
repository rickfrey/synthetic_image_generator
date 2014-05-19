cmake_minimum_required(VERSION 2.8.3)
project(synthetic_image_generator)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(synthetic_image_generator src/synthetic_image_generator.cpp)


target_link_libraries(synthetic_image_generator ${VTK_LIBRARIES})
