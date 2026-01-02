cmake_minimum_required(VERSION 3.20)
project(GX-EX)

# APP_NAME SOURCE_FILE LIBS
function(add_test_app ARG)
    add_executable(${ARGV0} ${ARGV1})
    list(LENGTH ARGV argv_len)
    set(i 2)
    set(LIB_TAR)
    while (i LESS ${argv_len})
        list(GET ARGV ${i} argv_value)
        list(APPEND LIB_TAR ${argv_value})
        math(EXPR i "${i} + 1")
    endwhile ()
    message(STATUS "${PROJECT_NAME} add_test_app APP_NAME: ${ARGV0}, SOURCE_FILE: ${ARGV1}, target_link_libraries: ${LIB_TAR}")
    target_link_libraries(${ARGV0}
            ${LIB_TAR}
            )

    target_compile_options(${ARGV0}
            PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
            $<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${GNU_BIG_OBJ_FLAG_ENABLE}>>:-Wa,-mbig-obj>)

    set_target_properties(${ARGV0} PROPERTIES FOLDER Gx/Examples)
endfunction()

add_test_app(TestGx test.cpp gx gany gx-math-plug)

add_test_app(TestString test_string.cpp gx)

add_test_app(TestByteArray test_bytearray.cpp gany gx)

add_test_app(TestTime test_time.cpp gx)

add_test_app(TestTimer test_timer.cpp gx)

add_test_app(TestMath test_math.cpp gx-math)

add_test_app(TestTaskSystem test_task_system.cpp gx)

add_test_app(TestCrypto test_gcrypto.cpp gany gx)
