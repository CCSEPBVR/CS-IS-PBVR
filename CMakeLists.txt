cmake_minimum_required(VERSION 3.24)
project(kvsm-converter)

set(TARGET_NAME kvsm-converter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(VTK REQUIRED)
find_package(GLEW REQUIRED)

if(NOT VTK_FOUND)
  message(FATAL_ERROR "${TARGET_NAME}: Unable to find the VTK build folder.")
endif()

if(MSVC)
  if(NOT KVS_DIR)
    message(FATAL_ERROR "${TARGET_NAME}: Undefined KVS_DIR")
  endif()

  if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    add_compile_options(/W3 /MTd)
  else()
    add_compile_options(/W3 /MT)
  endif()
  include_directories(${KVS_DIR}/include)
else()
  include(kvs.cmake)
  add_compile_options(-Wall)
endif()

add_executable(
  ${TARGET_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/example.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/StlExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtpExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtiExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtrExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtsExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/AvsExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtuExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/Plot3dExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtmExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/PvtsExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/PvtuExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/EnSightExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Example/CgnsExample.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Importer/VtkImport.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Exporter/StructuredVolumeObjectExporter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Exporter/UnstructuredVolumeObjectExporter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/Exporter/VtkExport.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/FileFormat/VtkCompositeDataSetFileFormat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/PBVRFileInformation/UnstructuredPfi.cpp)
target_include_directories(${TARGET_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
if(MSVC)
  target_link_libraries(${TARGET_NAME} PRIVATE ${KVS_DIR}/lib/kvsCore.lib  ${VTK_LIBRARIES})
else()
  target_link_libraries(${TARGET_NAME} PRIVATE ${VTK_LIBRARIES} kvsCore stdc++fs)
endif()
vtk_module_autoinit(TARGETS ${TARGET_NAME} MODULES ${VTK_LIBRARIES})
