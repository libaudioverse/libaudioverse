set(POWERCORES_FILES
thread_pool.cpp
utilities.cpp
)

add_library(powercores STATIC ${POWERCORES_FILES})
target_link_libraries(powercores ${CMAKE_THREAD_LIBS_INIT})