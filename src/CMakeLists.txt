#this makes everything below here compile using c99 on linux.
if(${CMAKE_COMPILER_IS_GNUCC})
add_definitions(-std=c99)
endif()
add_subdirectory(libaudioverse)
add_subdirectory(examples)
add_subdirectory(utils)