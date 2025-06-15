cmake_minimum_required(VERSION 3.10)
project(xt_tee VERSION 1.0 LANGUAGES C)

# Set the output binary name
add_executable(xt_tee xt_tee.c)

# Installation
install(TARGETS xt_tee DESTINATION bin)
install(FILES xt_tee.1 DESTINATION share/man/man1)

# Enable testing
enable_testing()
add_subdirectory(test)
