cmake_minimum_required(VERSION 3.20)

project(GxLib)

set(TARGET_NAME gx)

file(GLOB_RECURSE GX_CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE GX_CORE_HDR ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

if (APPLE)
    list(APPEND GX_CORE_SRC
            ${CMAKE_CURRENT_SOURCE_DIR}/src/gfile.mm
            )
endif (APPLE)

add_library(${TARGET_NAME}
        ${GX_CORE_SRC}
        ${GX_CORE_HDR}
        )
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${TARGET_NAME} PUBLIC gany)
target_link_libraries(${TARGET_NAME} PRIVATE z minizip tweetnacl)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Gx)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(${TARGET_NAME} PRIVATE BUILD_SHARED_LIBS=1)
endif ()

if (MINGW)
    target_link_libraries(${TARGET_NAME} PRIVATE winmm)
endif ()

# Additional dependencies on Unix
if (UNIX AND NOT APPLE AND NOT ANDROID)
    # Threads
    find_package(Threads)
    target_link_libraries(${TARGET_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (UNIX AND NOT APPLE AND NOT EMSCRIPTEN AND NOT ANDROID)
    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        target_link_libraries(${TARGET_NAME} PUBLIC ${RT_LIBRARY})
    endif ()

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        target_link_libraries(${TARGET_NAME} PUBLIC ${MATH_LIBRARY})
    endif ()

    find_library(LibUUID_LIBRARY uuid)
    mark_as_advanced(LibUUID_LIBRARY)
    if (LibUUID_LIBRARY)
        target_link_libraries(${TARGET_NAME} PRIVATE ${LibUUID_LIBRARY})
    endif ()

    target_link_libraries(${TARGET_NAME} PUBLIC -latomic -ldl)
endif ()

if (ANDROID)
    target_link_libraries(${TARGET_NAME} PUBLIC log)
endif ()



if (APPLE)
    if (IOS)
        target_link_libraries(${TARGET_NAME} PUBLIC
                "-framework QuartzCore"
                "-framework Foundation")
    else ()
        find_library(COCOA_LIBRARY Cocoa)
        mark_as_advanced(COCOA_LIBRARY)
        target_link_libraries(${TARGET_NAME} PUBLIC ${COCOA_LIBRARY})
    endif ()
endif ()

if (UNIX AND NOT ANDROID AND GX_USE_ICONV)
    find_package(Iconv)
    if (NOT Iconv_FOUND)
        message(FATAL_ERROR "Iconv not found")
    endif ()
    target_link_libraries(${TARGET_NAME} PUBLIC ${Iconv_LIBRARIES})
    target_compile_definitions(${TARGET_NAME} PRIVATE GX_USE_ICONV=1)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-Wa,-mbig-obj" GNU_BIG_OBJ_FLAG_ENABLE)
    message(STATUS GNU_BIG_OBJ_FLAG_ENABLE=${GNU_BIG_OBJ_FLAG_ENABLE})
endif ()

target_compile_options(${TARGET_NAME}
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${GNU_BIG_OBJ_FLAG_ENABLE}>>:-Wa,-mbig-obj>)

### Install
if (GX_LIBS_INSTALL_DIR)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/gx DESTINATION ${GX_LIBS_INSTALL_DIR}/gx/include)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gx.cmake DESTINATION ${GX_LIBS_INSTALL_DIR}/gx)
    install(TARGETS ${TARGET_NAME}
            RUNTIME DESTINATION ${GX_LIBS_INSTALL_DIR}/gx/bin
            LIBRARY DESTINATION ${GX_LIBS_INSTALL_DIR}/gx/lib
            ARCHIVE DESTINATION ${GX_LIBS_INSTALL_DIR}/gx/lib
            )
endif ()