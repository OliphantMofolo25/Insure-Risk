# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles/InsureRiskLinux_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/InsureRiskLinux_autogen.dir/ParseCache.txt"
  "CMakeFiles/catch_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/catch_tests_autogen.dir/ParseCache.txt"
  "CMakeFiles/functional_runner_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/functional_runner_autogen.dir/ParseCache.txt"
  "CMakeFiles/gtest_runner_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/gtest_runner_autogen.dir/ParseCache.txt"
  "CMakeFiles/qt_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/qt_tests_autogen.dir/ParseCache.txt"
  "InsureRiskLinux_autogen"
  "catch_tests_autogen"
  "functional_runner_autogen"
  "gtest_runner_autogen"
  "qt_tests_autogen"
  )
endif()
