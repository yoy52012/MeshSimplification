cmake_minimum_required(VERSION 3.17)

include(../cmake/CommonMacros.txt)

include_directories(../third_party/glfw/include)
include_directories(../third_party/glad/include)
include_directories(../third_party/glm)
include_directories(../third_party/tiny_obj_loader)
include_directories(../third_party/trackball)

include_directories(.)

file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_executable(${PROJECT_NAME}  ${SRC_FILES} ${HEADER_FILES})

if(MSVC)
	add_definitions(-D_CONSOLE)
endif()

set(FOLDER_NAME "bin")
SETUP_GROUPS("${SRC_FILES}")
SETUP_GROUPS("${HEADER_FILES}")

SET_OUTPUT_NAMES(${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${FOLDER_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC glad glfw imgui tiny_obj_loader)

