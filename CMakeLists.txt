cmake_minimum_required(VERSION 3.10)
project(TIRC++
		VERSION 0.1
		DESCRIPTION "Twitch Chat IRC Library"
		LANGUAGES CXX)

# Default build type is debug
if(NOT DEFINED ${CMAKE_BUILD_TYPE})
	set(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Set compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Add TIRC++ library target
add_library(tirc++ SHARED src/Socket.cpp src/Logger.cpp src/TIRCSocket.cpp)
target_include_directories(tirc++ PUBLIC include)
target_compile_features(tirc++ PRIVATE cxx_std_17)

# Link with Pthread library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(tirc++ PRIVATE Threads::Threads)

# Add TIRC++_test executable target
add_executable(tirc++_test tests/tirc++_test.cpp)
target_include_directories(tirc++_test PRIVATE include)
target_link_libraries(tirc++_test PRIVATE tirc++)
