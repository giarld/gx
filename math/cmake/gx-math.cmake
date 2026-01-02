if (NOT GX_MATH_FIND_PATH)
    message(FATAL_ERROR "Not set: GX_MATH_FIND_PATH")
endif ()

set(GX_MATH_INCLUDE_DIR ${GX_MATH_FIND_PATH}/include)
set(GX_MATH_BIN_DIR ${GX_MATH_FIND_PATH}/bin)

add_library(gx-math INTERFACE)
target_include_directories(gx-math INTERFACE ${GX_MATH_INCLUDE_DIR})
