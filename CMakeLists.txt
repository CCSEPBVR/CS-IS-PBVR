cmake_minimum_required(VERSION 3.24)
project(kvsm-converter)

set(TARGET_NAME kvsm-converter)

find_package(VTK)

if(NOT VTK_FOUND)
  message(FATAL_ERROR "${TARGET_NAME}: Unable to find the VTK build folder.")
endif()

include(kvs.cmake)
# set(KVS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Library/KVS/Source)
# target_include_directories(kvsCore PRIVATE ${KVS_SOURCE})

add_executable(
  ${TARGET_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/example.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/FileFormat/STL/Stl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Importer/PolygonImporter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Exporter/PolygonExporter.cpp)
target_include_directories(${TARGET_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_link_libraries(${TARGET_NAME} PRIVATE ${VTK_LIBRARIES} kvsCore)
vtk_module_autoinit(TARGETS ${TARGET_NAME} MODULES ${VTK_LIBRARIES})
