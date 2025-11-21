cmake_minimum_required(VERSION 3.16)
project(OOP_lab5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

include_directories(include)

set(SOURCES
    main.cpp
)

add_executable(main ${SOURCES})

find_package(GTest QUIET)
if(GTest_FOUND)
    enable_testing()
    
    add_executable(tests tests/tests.cpp)
    target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(tests GTest::gtest GTest::gtest_main)
    
    add_test(NAME OOP_lab5 COMMAND tests)
else()
    message(WARNING "Google Test not found - tests will not be built")
endif()