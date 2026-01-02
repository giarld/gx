cmake_minimum_required(VERSION 3.20)

project(GxSignTool)

set(TARGET_NAME gx-sign-tool)

add_executable(${TARGET_NAME} src/main.cpp)

target_link_libraries(${TARGET_NAME} gany gx getopt)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Gx/Tools)
