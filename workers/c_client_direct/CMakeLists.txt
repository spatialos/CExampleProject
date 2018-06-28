# Builds a SpatialOS worker using the C API

# Replace project name below with your own worker name
# !!! This needs to match the artifact_name in spatialos.<name>.worker.json
# !!! because ${PROJECT_NAME} is used below when creating the zip target
project(ClientDirect C)
cmake_minimum_required(VERSION 3.0)

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
add_executable(${PROJECT_NAME} client.c)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
target_link_libraries(${PROJECT_NAME} CWorker)

# Set artifact subdirectories.
# WORKER_BUILD_DIR should not be changed so that spatial local launch
# and spatial upload can find the worker assemblies
set(WORKER_BUILD_DIR "${APPLICATION_ROOT}/build/assembly/worker")
set(WORKER_PACKAGE_COMMAND "spatial")

# The worker in a zip.
add_custom_target(
  ${PROJECT_NAME}Zip ALL
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${WORKER_BUILD_DIR}"
  COMMAND ${WORKER_PACKAGE_COMMAND} file zip -b "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
          -o "${WORKER_BUILD_DIR}/${PROJECT_NAME}"
          --worker_platform=current
          "$<TARGET_FILE_NAME:${PROJECT_NAME}>"
  DEPENDS ${PROJECT_NAME})
