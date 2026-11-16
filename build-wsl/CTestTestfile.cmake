# CMake generated Testfile for 
# Source directory: /home/mofolo/Documents/Insure-Risk
# Build directory: /home/mofolo/Documents/Insure-Risk/build-wsl
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("/home/mofolo/Documents/Insure-Risk/build-wsl/gtest_runner[1]_include.cmake")
add_test(Catch2Tests "/home/mofolo/Documents/Insure-Risk/build-wsl/catch_tests")
set_tests_properties(Catch2Tests PROPERTIES  _BACKTRACE_TRIPLES "/home/mofolo/Documents/Insure-Risk/CMakeLists.txt;96;add_test;/home/mofolo/Documents/Insure-Risk/CMakeLists.txt;0;")
add_test(QtLibraryTests "/home/mofolo/Documents/Insure-Risk/build-wsl/qt_tests")
set_tests_properties(QtLibraryTests PROPERTIES  _BACKTRACE_TRIPLES "/home/mofolo/Documents/Insure-Risk/CMakeLists.txt;107;add_test;/home/mofolo/Documents/Insure-Risk/CMakeLists.txt;0;")
