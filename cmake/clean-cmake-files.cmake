# http://stackoverflow.com/a/13714219

set(cmake_generated CMakeCache.txt cmake_install.cmake Makefile CMakeFiles)

file(GLOB_RECURSE cmake_files RELATIVE ${CMAKE_BINARY_DIR} ${cmake_generated})

foreach(cmake_file ${cmake_files})
  get_filename_component(dir "${cmake_file}" PATH)
  file(REMOVE_RECURSE ${cmake_file})
  file(REMOVE_RECURSE "${dir}/CMakeFiles")
endforeach(cmake_file)
