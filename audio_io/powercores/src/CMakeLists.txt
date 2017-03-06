add_subdirectory(powercores)
if(${POWERCORES_BUILD_TESTS})
add_subdirectory(tests)
endif()