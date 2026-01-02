cmake_minimum_required(VERSION 3.20)

project(Math)

set(TARGET_NAME gx-math)

add_library(${TARGET_NAME} INTERFACE)
target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${TARGET_NAME} INTERFACE gx)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER Gx)

### Install
if (GX_LIBS_INSTALL_DIR)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/gx DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math/include)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gx-math.cmake DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math)
    install(TARGETS ${TARGET_NAME}
            RUNTIME DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math/bin
            LIBRARY DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math/lib
            ARCHIVE DESTINATION ${GX_LIBS_INSTALL_DIR}/gx-math/lib
            )
endif ()
