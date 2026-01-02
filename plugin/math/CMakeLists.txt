cmake_minimum_required(VERSION 3.20)

project(GxPlugin)

set(TARGET_NAME gx-math-plug)

file(GLOB_RECURSE GX_MATH_PLUG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE GX_MATH_PLUG_HDR ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

add_library(${TARGET_NAME}
        ${GX_MATH_PLUG_SRC}
        ${GX_MATH_PLUG_HDR}
)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${TARGET_NAME} PUBLIC gx gx-math)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Gx)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(${TARGET_NAME} PRIVATE BUILD_SHARED_LIBS=1)
endif ()

target_compile_options(${TARGET_NAME}
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${GNU_BIG_OBJ_FLAG_ENABLE}>>:-Wa,-mbig-obj>)

### Install
if (GX_LIBS_INSTALL_DIR)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/gx DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math-plug/include)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gx-math-plug.cmake DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math-plug)
    install(TARGETS ${TARGET_NAME}
            RUNTIME DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math-plug/bin
            LIBRARY DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math-plug/lib
            ARCHIVE DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math-plug/lib
            )
endif ()
