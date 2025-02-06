cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Find GTest
find_package(GTest REQUIRED)

# Automatically find all test source files
file(GLOB TEST_FILES *.cpp)

add_executable(queueTests ${TEST_FILES})

target_link_libraries(queueTests PRIVATE queue GTest::GTest GTest::Main Threads::Threads)

# Add all tests automatically
foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_test(NAME ${test_name} COMMAND queueTests)
endforeach()
