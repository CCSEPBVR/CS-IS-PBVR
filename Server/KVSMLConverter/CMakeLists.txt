cmake_minimum_required(VERSION 3.24)
project(kvsml-converter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(VTK REQUIRED)

if(NOT VTK_FOUND)
  message(FATAL_ERROR "${TARGET_NAME}: Unable to find the VTK build folder.")
endif()

if(MSVC)
  find_package(GLEW REQUIRED)

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

  if(CMAKE_BUILD_TYPE MATCHES PROFILE)
    add_compile_options(-pg)
    add_link_options(-pg)
  endif()
endif()

set(CVT_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/FileFormat/NetCDF/Netcdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Importer/VtkImport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Converter/ConverterTask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Converter/ConverterInputs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Converter/ConverterTaskInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Converter/ConverterTaskOutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Exporter/StructuredVolumeObjectExporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Exporter/UnstructuredVolumeObjectExporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/FileFormat/VtkCompositeDataSetFileFormat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/PBVRFileInformation/UnstructuredPfi.cpp)

set(TARGET_NAME "kvsml-converter")

add_executable(
  ${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/Source/kvsml-converter.cpp
                 ${CVT_SOURCE})
target_include_directories(
  ${TARGET_NAME} PRIVATE ${MPI_INCLUDE_PATH} ${VTK_INCLUDE_PATH}
                         ${CMAKE_CURRENT_SOURCE_DIR}/Source)
if(MSVC)
  target_link_libraries(${TARGET_NAME} PRIVATE ${KVS_DIR}/lib/kvsCore.lib
                                               ${VTK_LIBRARIES} VTK::IONetCDF)
else()
  target_link_libraries(${TARGET_NAME} PRIVATE ${VTK_LIBRARIES} VTK::IONetCDF kvsCore
                                               stdc++fs)
endif()

if(CVT_MPI)
  set(MPI_TARGET_NAME "kvsml-converter-mpi")

  find_package(MPI REQUIRED)

  if(NOT MPI_FOUND)
    message(
      FATAL_ERROR "${TARGET_NAME}: KVS_MPI is enabled, but MPI was not found")
  endif()

  add_executable(
    ${MPI_TARGET_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/kvsml-converter-mpi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/MPIRunner/MpiMainProcess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/MPIRunner/MpiSubProcess.cpp
    ${CVT_SOURCE})
  target_include_directories(
    ${MPI_TARGET_NAME} PRIVATE ${MPI_INCLUDE_PATH} ${VTK_INCLUDE_PATH}
                               ${CMAKE_CURRENT_SOURCE_DIR}/Source)
  if(MSVC)
    target_link_libraries(${MPI_TARGET_NAME} PRIVATE ${KVS_DIR}/lib/kvsCore.lib
                                                     ${VTK_LIBRARIES} VTK::IONetCDF)
  else()
    target_link_libraries(${MPI_TARGET_NAME} PRIVATE ${VTK_LIBRARIES} VTK::IONetCDF kvsCore
                                                     stdc++fs)
  endif()
endif()

if(CVT_EXAMPLE)
  set(EXAMPLE_NAME "kvsml-example")

  set(CVT_EXAMPLE_SOURCE
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/StlExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtpExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtiExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtrExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtsExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/AvsExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/NetcdfExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtuExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/Plot3dExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/VtmExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/PvtsExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/PvtuExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/EnSightExample.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/Example/CgnsExample.cpp)

  add_executable(${EXAMPLE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/Example/example.cpp
                                 ${CVT_SOURCE} ${CVT_EXAMPLE_SOURCE})
  target_include_directories(
    ${EXAMPLE_NAME} PRIVATE ${VTK_INCLUDE_PATH}
                            ${CMAKE_CURRENT_SOURCE_DIR}/Source)
  if(MSVC)
    target_link_libraries(${EXAMPLE_NAME} PRIVATE ${KVS_DIR}/lib/kvsCore.lib
                                                  ${VTK_LIBRARIES} VTK::IONetCDF)
  else()
    target_link_libraries(${EXAMPLE_NAME} PRIVATE ${VTK_LIBRARIES} VTK::IONetCDF kvsCore
                                                  stdc++fs)
  endif()
  vtk_module_autoinit(TARGETS ${EXAMPLE_NAME} MODULES ${VTK_LIBRARIES})
endif()
