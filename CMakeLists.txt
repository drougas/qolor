cmake_minimum_required (VERSION 2.6)
project (qolor)

# The version number.
set(qolor_VERSION_MAJOR 0)
set(qolor_VERSION_MINOR 0)
set(qolor_VERSION_PATCH 1)
set(qolor_VERSION_FULL "${qolor_VERSION_MAJOR}.${qolor_VERSION_MINOR}.${qolor_VERSION_PATCH}")
set(qolor_INSTALL_PREFIX "/usr/local")

#set(CMAKE_BUILD_TYPE None)
set(CMAKE_USE_RELATIVE_PATHS ON)
set(EXECUTABLE_OUTPUT_PATH ${qolor_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${qolor_BINARY_DIR}/lib)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11 -fexceptions -fcxx-exceptions")
  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")

elseif (CMAKE_COMPILER_IS_GNUCXX)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_OSX_SYSROOT "")
  endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")

endif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(Sqlite3 REQUIRED)
include_directories(${SQLITE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${SQLITE3_LIBRARIES})
#add_library(sqlite3 SHARED IMPORTED)
#set_property(TARGET sqlite3 PROPERTY IMPORTED_LOCATION ${SQLITE3_LIBRARIES})

if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
  # Add cleanall command
  # http://stackoverflow.com/a/13714219
  add_custom_target(clean-cmake-files
    COMMAND ${CMAKE_COMMAND} -P ${qolor_SOURCE_DIR}/cmake/clean-cmake-files.cmake
  )

  add_custom_target(cleanall
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND ${CMAKE_COMMAND} -P ${qolor_SOURCE_DIR}/cmake/clean-cmake-files.cmake
  )
endif (CMAKE_GENERATOR STREQUAL "Unix Makefiles")


include_directories (${qolor_SOURCE_DIR}/include)
include_directories (${qolor_BINARY_DIR}/include)

add_subdirectory(include)
add_subdirectory(src)

enable_testing()
add_custom_target(runtests ${CMAKE_CTEST_COMMAND} -V)
add_subdirectory(test)
#add_subdirectory(test EXCLUDE_FROM_ALL)
