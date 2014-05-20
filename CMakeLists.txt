cmake_minimum_required(VERSION 2.8.3)
project(synthetic_image_generator)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(affine_trafo src/affine_trafo.cpp)
add_executable(image_generator src/image_generator.cpp)

target_link_libraries(affine_trafo ${VTK_LIBRARIES})
target_link_libraries(image_generator ${VTK_LIBRARIES})
