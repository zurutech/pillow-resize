cmake_minimum_required(VERSION 3.1)

add_executable(resize Resize.cc)

target_link_libraries(
  resize
  PUBLIC # Opencv libraries
         "${OpenCV_LIBS}"
         # CLI
         CLI11::CLI11
         # magic_enum
         magic_enum::magic_enum
         # PillowResize lib
         PillowResize)

# Define compile features: c++17 and no extensions
target_compile_features(resize PRIVATE cxx_std_17)
set_target_properties(resize PROPERTIES CXX_EXTENSIONS OFF)

set(EXECUTABLES ${CMAKE_CURRENT_BINARY_DIR}/resize)

# Install binaries into the system
include(GNUInstallDirs)
install(
  FILES ${EXECUTABLES}
  DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}/${PROJECT_NAME}
  PERMISSIONS WORLD_EXECUTE WORLD_WRITE WORLD_READ)
