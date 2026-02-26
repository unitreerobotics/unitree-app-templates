cmake_minimum_required(VERSION 3.12)
project(g1_controller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(yaml-cpp REQUIRED)

# aarch or x64
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _proc)
if(_proc MATCHES "^(aarch64|arm64)$")
  set(ONNX_DIR "${PROJECT_SOURCE_DIR}/third_party/aarch64/onnxruntime")
  message(STATUS "ONNX_DIR is set to ${ONNX_DIR}")
elseif(_proc MATCHES "^(x86_64|amd64|x64)$")
  set(ONNX_DIR "${PROJECT_SOURCE_DIR}/third_party/x64/onnxruntime")
  message(STATUS "ONNX_DIR is set to ${ONNX_DIR}")
else()
  message(FATAL_ERROR "Unsupported arch: ${CMAKE_SYSTEM_PROCESSOR}. Set paths manually.")
endif()

message(STATUS "ONNX include: ${ONNX_DIR}/include")
message(STATUS "ONNX libs: ${ONNX_DIR}/lib")

# ------------------------
# Other include dirs
# ------------------------
include_directories(
  /usr/include/eigen3
  include
  /usr/local/include/ddsc # dds
  /usr/local/include/ddscxx # dds
  ${ONNX_DIR}/include
)

link_directories(
  ${ONNX_DIR}/lib
)

# ------------------------
# Link libs
# ------------------------
link_libraries(
  unitree_sdk2
  ddsc
  ddscxx
  rt
  pthread
  libboost_program_options.a
  libyaml-cpp.a
  fmt
  ${ONNX_DIR}/lib/libonnxruntime.so.1.22.0
)

# ------------------------
# Source files
# ------------------------
file(GLOB_RECURSE ADD_SRC_LIST
  ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_library(${PROJECT_NAME}_lib ${ADD_SRC_LIST})
link_libraries(${PROJECT_NAME}_lib)

add_executable(g1_ctrl main.cpp)

# rpath
set(_REL_RPATH "\$ORIGIN;\$ORIGIN/../lib;\$ORIGIN/../config")

set_target_properties(g1_ctrl PROPERTIES
  BUILD_WITH_INSTALL_RPATH ON
  INSTALL_RPATH "${_REL_RPATH}"
)

target_link_options(g1_ctrl PRIVATE 
  -Wl,--enable-new-dtags
  "-Wl,-rpath,\$ORIGIN/../lib"
  "-Wl,-rpath,\$ORIGIN/../config"
)

# install
install(TARGETS g1_ctrl RUNTIME DESTINATION bin)

file(GLOB ONNX_SO "${ONNX_DIR}/lib/libonnxruntime.so*")
install(FILES ${ONNX_SO} DESTINATION lib)
# Install DDS shared libraries so the deployed binary can load them from ../lib
file(GLOB DDS_SO
  /usr/local/lib/libddsc*.so*
  /usr/local/lib/libddscxx*.so*
  /usr/lib/libddsc*.so*
  /usr/lib/libddscxx*.so*
  /lib/libddsc*.so*
  /lib/libddscxx*.so*
)

if(DDS_SO)
  install(FILES ${DDS_SO} DESTINATION lib)
else()
  message(WARNING "No DDS libraries found to install (searched /usr/local/lib, /usr/lib, /lib).")
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/config/ DESTINATION config)