# Examples CMakeLists.txt
# This file is included by the main CMakeLists.txt when BUILD_EXAMPLES is ON

# Example 1: Basic Triangle
# add_executable(example_triangle ${CMAKE_SOURCE_DIR}/src/examples/triangle.cpp)
# target_link_libraries(example_triangle PRIVATE ${OPENGL_LIBRARIES} ${GLUT_TARGET})
# set_target_properties(example_triangle PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/examples
# )

# Example 2: Texture Demo
# add_executable(example_texture ${CMAKE_SOURCE_DIR}/src/examples/texture.cpp)
# target_link_libraries(example_texture PRIVATE ${OPENGL_LIBRARIES} ${GLUT_TARGET})
# set_target_properties(example_texture PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/examples
# )

message(STATUS "Examples directory loaded (no examples configured by default)")
