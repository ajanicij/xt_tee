# Path to the built binary
set(XT_TEE_BINARY $<TARGET_FILE:xt_tee>)

# Add a simple test that runs a shell script
add_test(NAME basic_test
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_basic.sh ${XT_TEE_BINARY})
