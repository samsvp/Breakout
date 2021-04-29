cmake_minimum_required(VERSION 3.10)
project(breakout)
set(CMAKE_MODULE_PATH /home/user/Libs/cmake)

find_package(Freetype REQUIRED)

set(CMAKE_CXX_STANDARD 11)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp)

# We need a CMAKE_DIR with some code to find external dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# OpenGL
find_package(OpenGL REQUIRED)

# GLM
find_package(glm REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# GLFW
find_package(glfw3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

set(LIBS GL GLU glfw X11 Xxf86vm Xrandr pthread Xi dl sfml-audio freetype)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

# Define the link libraries
target_link_libraries(${PROJECT_NAME} ${LIBS})

target_link_libraries(breakout ${FREETYPE_LIBRARIES})
target_include_directories(breakout PRIVATE ${FREETYPE_INCLUDE_DIRS})