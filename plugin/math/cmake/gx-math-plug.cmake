if (NOT GX_MATH_PLUG_FIND_PATH)
    message(FATAL_ERROR "Not set: GX_MATH_PLUG_FIND_PATH")
endif ()

set(GX_MATH_PLUG_INCLUDE_DIR ${GX_MATH_PLUG_FIND_PATH}/include)
set(GX_MATH_PLUG_BIN_DIR ${GX_MATH_PLUG_FIND_PATH}/bin)
set(GX_MATH_PLUG_LIB_DIR ${GX_MATH_PLUG_FIND_PATH}/lib)

add_library(gx-math-plug INTERFACE)
target_include_directories(gx-math-plug INTERFACE ${GX_MATH_PLUG_INCLUDE_DIR})
target_link_libraries(gx-math-plug INTERFACE
        ${GX_MATH_PLUG_LIB_DIR}/${LINK_LIBRARY_PREFIX}gx-math-plug${LINK_LIBRARY_SUFFIX}
        gx
        )
