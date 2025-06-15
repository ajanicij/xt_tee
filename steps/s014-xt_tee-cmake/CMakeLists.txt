cmake_minimum_required(VERSION 3.10)

project(xt_tee VERSION 1.0 LANGUAGES C)

# Set the output binary name
add_executable(xt_tee xt_tee.c)

# Installation
install(TARGETS xt_tee DESTINATION bin)
install(FILES xt_tee.1 DESTINATION share/man/man1)

# Enable testing
enable_testing()

# Define a basic test
add_test(
    NAME basic_test
    COMMAND bash -c "echo hello | $<TARGET_FILE:xt_tee> > output.txt && grep -q hello output.txt"
)

# Packaging setup for CPack
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "xt_tee")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "Aleksandar Janicijevic <ajanicij@yahoo.com>")
set(CPACK_PACKAGE_VENDOR "Vogonsoft Inc.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "xt_tee - a lightweight clone of tee")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" CACHE FILEPATH "License file")
set(CPACK_GENERATOR "TGZ;DEB")

include(CPack)
