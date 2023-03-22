cmake_minimum_required(VERSION 3.1)
project(PillowResize_Tests)

enable_testing()
find_package(GTest REQUIRED)


# Define test source files
set(PILLOW_RESIZE_TEST_SOURCE_FILES
    main.cc    
    src/PillowResize/ResizeTest.cc)

# Create test target
add_executable(resize_tests ${PILLOW_RESIZE_TEST_SOURCE_FILES})

target_link_libraries(
    resize_tests
    PRIVATE
        # Gtest 
        gtest
        # Opencv libraries
        "${OpenCV_LIBS}"         
        # magic_enum
        magic_enum::magic_enum
        # PillowResize lib
        PillowResize)


# Define compile features for this target: c++17 for tests
target_compile_features(resize_tests PRIVATE cxx_std_17)
set_target_properties(resize_tests PROPERTIES CXX_EXTENSIONS OFF)

gtest_discover_tests(resize_tests WORKING_DIRECTORY ${PROJECT_BINARY_DIR} DISCOVERY_TIMEOUT 60)

# copy asset folder
file(COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})