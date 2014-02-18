project (qolor_test)

set(EXECUTABLE_OUTPUT_PATH ${qolor_BINARY_DIR}/test)

file(GLOB srcs RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.c" "*.cc" "*.cpp" "*.cxx")

foreach(src ${srcs})
  string(REGEX REPLACE ".(c|cpp|cxx|C|CXX|CPP)$" "" exe ${src})

  add_executable(${exe} ${src})
  target_link_libraries(${exe} ${LIBS} qolor-${qolor_VERSION_FULL})

  add_test(NAME ${exe} COMMAND ${exe})
  set_tests_properties(${exe} PROPERTIES FAIL_REGULAR_EXPRESSION "FAILURE")
  add_dependencies(runtests ${exe})

endforeach(src)

file(GLOB ress RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.csv" "*.db" "*.sql")
foreach(res ${ress})
  configure_file(${res} ${res} COPYONLY)
endforeach(res)

