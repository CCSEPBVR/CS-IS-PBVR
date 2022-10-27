cmake_minimum_required(VERSION 3.24)
project(kvsm-converter)

set(TARGET_NAME kvsm-converter)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(VTK)

if(NOT VTK_FOUND)
  message(FATAL_ERROR "${TARGET_NAME}: Unable to find the VTK build folder.")
endif()

include(kvs.cmake)

if(MSVC)
  add_compile_options(/W3 /WX)
else()
  add_compile_options(-Wall)
endif()

add_executable(
  ${TARGET_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/example.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/StlExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Convert.cpp)
target_include_directories(${TARGET_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_link_libraries(${TARGET_NAME} PRIVATE ${VTK_LIBRARIES} kvsCore)
vtk_module_autoinit(TARGETS ${TARGET_NAME} MODULES ${VTK_LIBRARIES})
