# This script is included by worker and library builds
# It is not meant to be built as a standalone library

# This is defined in spatialos_worker_packages.json (usually in the directory of each project
# that requires the SDK)
set(WORKER_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/worker_sdk")

# Worker SDK libraries shared by both the C and C++ APIs.
find_package(Threads)

# On Windows this finds the import library (.lib) file, which contains information on how to link the .dll.
find_library(LIB_IMPROBABLE_WORKER
  NAMES improbable_worker
  PATHS "${WORKER_SDK_DIR}/lib"
  NO_DEFAULT_PATH)

find_file(DLL_IMPROBABLE_WORKER
  NAMES improbable_worker.dll
  PATHS "${WORKER_SDK_DIR}/lib"
  NO_DEFAULT_PATH)

# C API.
add_library(CWorker INTERFACE)
target_include_directories(CWorker SYSTEM INTERFACE "${WORKER_SDK_DIR}/c_headers/include")
target_link_libraries(CWorker INTERFACE
  ${LIB_IMPROBABLE_WORKER}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CMAKE_DL_LIBS})

# C++ API.
add_library(CppWorker INTERFACE)
target_include_directories(CppWorker SYSTEM INTERFACE "${WORKER_SDK_DIR}/cpp_headers/include")
target_link_libraries(CppWorker INTERFACE
  ${LIB_IMPROBABLE_WORKER}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CMAKE_DL_LIBS})

# As the worker SDK is implemented using C++ under the hood, it depends on the C++ standard library
# being linked. As a result, we need to explicitly link against the C++ standard library to
# avoid linker errors (despite building C code).
if (UNIX AND NOT APPLE)
  # On Linux, we need to link against libm and libstdc++ directly.
  target_link_libraries(CWorker INTERFACE m stdc++)
  target_link_libraries(CppWorker INTERFACE m stdc++)
elseif(APPLE)
  # On macOS, we need to link against libc++ directly.
  target_link_libraries(CWorker INTERFACE c++)
  target_link_libraries(CppWorker INTERFACE c++)
endif()


function(CREATE_WORKER_ZIP WORKER)
  # Set artifact subdirectories.
  set(WORKER_PACKAGE_COMMAND "spatial")
  # WORKER_BUILD_DIR should not be changed so that spatial local launch
  # and spatial upload can find the worker assemblies
  set(WORKER_BUILD_DIR "${APPLICATION_ROOT}/build/assembly/worker")

  if (MSVC)
    set(WORKER_RUNTIME_LIBRARY ${DLL_IMPROBABLE_WORKER})
  else()
    SET(WORKER_RUNTIME_LIBRARY ${LIB_IMPROBABLE_WORKER})
  endif()

  # Place the dynamic worker library next to the worker executable.
  add_custom_command(TARGET ${WORKER} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${WORKER_RUNTIME_LIBRARY}
          $<TARGET_FILE_DIR:${WORKER}>)

  # File name of the dynamic worker library.
  get_filename_component(WORKER_RUNTIME_LIBRARY_FILE_NAME ${WORKER_RUNTIME_LIBRARY} NAME)

  # Zip the executable and dynamic worker library.
  add_custom_target(
    ${WORKER}Zip ALL
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${WORKER_BUILD_DIR}"
    COMMAND ${WORKER_PACKAGE_COMMAND} file zip -b "$<TARGET_FILE_DIR:${WORKER}>"
            -o "${WORKER_BUILD_DIR}/${WORKER}"
            --worker_platform=current
            "$<TARGET_FILE_NAME:${WORKER}>"
            ${WORKER_RUNTIME_LIBRARY_FILE_NAME}
    DEPENDS ${WORKER})
endfunction()
