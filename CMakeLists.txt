cmake_minimum_required(VERSION 3.16)
project(InsureRiskLinux C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(InsureRiskLinux
    include/MainWindow.hpp
    src/main.cpp
    src/MainWindow.cpp
    src/LearningCentre.cpp
    src/Policyholder.cpp
    src/RiskEvaluator.cpp
    src/ActuarialEngine.cpp
    src/DynamicPricingEngine.cpp
    src/RiskAdvisoryEngine.cpp
    src/MonteCarloSimulator.cpp
    src/AuthManager.cpp
    src/DatabaseManager.cpp
    src/sqlite3.c
    resources.qrc
)

find_package(Threads REQUIRED)
target_link_libraries(InsureRiskLinux PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::Charts
    Threads::Threads 
    ${CMAKE_DL_LIBS}
)

# --- Optional Cppcheck Static Analysis Target ---
find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK}
            --enable=all
            --std=c++17
            --inconclusive
            --suppress=missingIncludeSystem
            -I ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
        COMMENT "Running cppcheck static analysis..."
        VERBATIM
    )
endif()

# --- Google Test Suite Option ---
option(BUILD_TESTING "Build Google Test suite" OFF)
if(BUILD_TESTING)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(unit_tests
        tests/test_actuarial.cpp
        src/Policyholder.cpp
        src/RiskEvaluator.cpp
        src/ActuarialEngine.cpp
    src/DynamicPricingEngine.cpp
    src/RiskAdvisoryEngine.cpp
        src/MonteCarloSimulator.cpp
        src/sqlite3.c
    )
    target_link_libraries(unit_tests PRIVATE gtest_main Qt6::Core Threads::Threads ${CMAKE_DL_LIBS})
    target_include_directories(unit_tests PRIVATE include)
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()



