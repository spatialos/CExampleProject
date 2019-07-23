# This script is included by worker and library builds
# It is not meant to be built as a standalone library

# This is defined in spatialos_worker_packages.json (usually in the directory of each project
# that requires the SDK)
set(WORKER_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/worker_sdk")

# C API libraries
find_package(Threads)
find_library(LIB_IMPROBABLE_WORKER improbable_worker "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_RAKNET RakNetLibStatic "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_SSL ssl "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_ZLIB NAMES z zlibstatic PATHS "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)

# C API.
add_library(CWorker INTERFACE)
target_include_directories(CWorker SYSTEM INTERFACE "${WORKER_SDK_DIR}/headers/include")
target_link_libraries(CWorker INTERFACE
  ${LIB_IMPROBABLE_WORKER} ${LIB_RAKNET} ${LIB_SSL} ${LIB_ZLIB}
  ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

# As the C API is implemented using C++ under the hood, it depends on the C++ standard library
# being linked. As a result, we need to explicitly link against the C++ standard library to
# avoid linker errors (despite building C code).
if (UNIX AND NOT APPLE)
  # On Linux, we need to link against libm and libstdc++ directly.
  target_link_libraries(CWorker INTERFACE m stdc++)
elseif(APPLE)
  # On macOS, we need to link against libc++ directly.
  target_link_libraries(CWorker INTERFACE c++)
endif()