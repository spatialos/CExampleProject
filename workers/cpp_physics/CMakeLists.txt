# Builds a SpatialOS worker using the C++ API

# Replace project name below with your own worker name
# !!! This needs to match the artifact_name in spatialos.<name>.worker.json
# !!! because ${PROJECT_NAME} is used below when creating the zip target
project(Physics)
cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(APPLICATION_ROOT "${PROJECT_SOURCE_DIR}/../..")
set(WORKER_SDK_DIR "${APPLICATION_ROOT}/dependencies")

if(MSVC)
  add_definitions(/W2 /bigobj)
else()
  add_definitions(-Wall -Wextra -Werror -pedantic -Wno-unused-parameter -Wno-missing-field-initializers -Wno-missing-braces)
endif()

# Set up the Worker SDK target.
add_subdirectory(${WORKER_SDK_DIR} "${CMAKE_CURRENT_BINARY_DIR}/WorkerSdk")

# The worker binary.
file(GLOB_RECURSE GENERATED_CODE_FILES
    "generated_code/*.cc"
    "generated_code/*.h")
add_executable(${PROJECT_NAME} physics.cc ${GENERATED_CODE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE generated_code)
target_link_libraries(${PROJECT_NAME} CppWorker)

# Creates a zip file with the worker executable and the dynamic worker library.
create_worker_zip(${PROJECT_NAME})
