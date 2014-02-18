file(GLOB srcs RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.c" "*.cc" "*.cpp" "*.cxx")
add_library (qolor-${qolor_VERSION_FULL} ${srcs})
