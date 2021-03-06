# Builds a SpatialOS worker using the C API

# Replace project name below with your own worker name
# !!! This needs to match the artifact_name in spatialos.<name>.worker.json
# !!! because ${PROJECT_NAME} is used below when creating the zip target
project(ClientVtable C)
cmake_minimum_required(VERSION 3.7)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(APPLICATION_ROOT "${PROJECT_SOURCE_DIR}/../..")
set(WORKER_SDK_DIR "${APPLICATION_ROOT}/dependencies")

if(MSVC)
  add_definitions(/W2)
else()
  add_definitions(-Wall -Wextra -Werror -pedantic -Wno-unused-parameter -Wno-missing-field-initializers -Wno-missing-braces)
endif()

# Set up the Worker SDK target.
add_subdirectory(${WORKER_SDK_DIR} "${CMAKE_CURRENT_BINARY_DIR}/WorkerSdk")

# The worker binary.
add_executable(${PROJECT_NAME} client.c schema_types.h schema_types.c)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
target_link_libraries(${PROJECT_NAME} CWorker)

# Creates a zip file with the worker executable and the dynamic worker library.
create_worker_zip(${PROJECT_NAME})
