cmake_minimum_required(VERSION 3.28)
project(test_simple)


#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------


include("../cmake/utils.cmake")
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" IS_TOP_LEVEL)

if(IS_TOP_LEVEL)
    enable_testing()
endif()


#----------------------------------------------------------------------------------------------------------------------
# testing framework
#----------------------------------------------------------------------------------------------------------------------


if(IS_TOP_LEVEL AND DEFINED ENV{VCPKG_ROOT})
    include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()


#----------------------------------------------------------------------------------------------------------------------
# tests dependencies
#----------------------------------------------------------------------------------------------------------------------


if(IS_TOP_LEVEL)
    find_package(erslib REQUIRED)
endif()

find_package(Catch2 3 CONFIG REQUIRED)
include(Catch)


add_library(test_support
    STATIC
        "support/temp_tree.cpp"
)

target_include_directories(test_support PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test_support PUBLIC erslib::core)


#----------------------------------------------------------------------------------------------------------------------
# stateless tests
#----------------------------------------------------------------------------------------------------------------------


add_library(test_simple_main OBJECT "simple.cpp")
target_link_libraries(test_simple_main PUBLIC Catch2::Catch2WithMain test_support)

set(SIMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/simple")

file(GLOB SIMPLE_MODULES CONFIGURE_DEPENDS
    LIST_DIRECTORIES true
    RELATIVE "${SIMPLE_DIR}"
    "${SIMPLE_DIR}/*"
)

# Builds every per-file test, so "test_simple" and "test_simple_<module>" still exist in VS
add_custom_target(test_simple)

foreach(MODULE IN LISTS SIMPLE_MODULES)
    if(NOT IS_DIRECTORY "${SIMPLE_DIR}/${MODULE}")
        continue()
    endif()

    if(NOT TARGET erslib::${MODULE})
        message(STATUS "Skipping simple tests for '${MODULE}': erslib::${MODULE} is not built")
        continue()
    endif()

    add_custom_target(test_simple_${MODULE})
    add_dependencies(test_simple test_simple_${MODULE})

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${SIMPLE_DIR}/${MODULE}/*.cpp")

    foreach(TEST_SOURCE IN LISTS TEST_SOURCES)
        # simple/core/sub/string.cpp -> "core/sub/string" -> test_simple_core_sub_string
        file(RELATIVE_PATH TEST_NAME "${SIMPLE_DIR}" "${TEST_SOURCE}")
        string(REGEX REPLACE "\\.cpp$" "" TEST_NAME "${TEST_NAME}")
        string(MAKE_C_IDENTIFIER "test_simple_${TEST_NAME}" TEST_TARGET)

        add_executable(${TEST_TARGET} "${TEST_SOURCE}")
        target_link_libraries(${TEST_TARGET} PRIVATE test_simple_main erslib::${MODULE})
        set_target_properties(${TEST_TARGET} PROPERTIES FOLDER "tests/simple/${MODULE}")
        target_compile_definitions(${TEST_TARGET} PRIVATE TEST_TAGS="[simple]")

        if(NOT IS_TOP_LEVEL)
            win_copy_deps_to_target_dir(${TEST_TARGET} erslib::${MODULE})
        endif()

        catch_discover_tests(${TEST_TARGET} TEST_PREFIX "${TEST_NAME}::" DISCOVERY_MODE PRE_TEST ADD_TAGS_AS_LABELS)
        add_dependencies(test_simple_${MODULE} ${TEST_TARGET})

        message(STATUS "Discovered simple test: ${TEST_NAME} -> Target: ${TEST_TARGET}")
    endforeach()
endforeach()

if(TARGET erslib_core)
    target_compile_definitions(erslib_core
        PRIVATE
            ERS_ASSERT_INFO
    )
endif()


#----------------------------------------------------------------------------------------------------------------------
# helpers for per-directory tests
#----------------------------------------------------------------------------------------------------------------------


# Both helpers are meant to be called from a test's own CMakeLists.txt (added via
# add_subdirectory below). They derive everything from the calling directory, so a
# test's CMakeLists.txt only needs the single call. The target is named
# test_<category>_<directory-name> and all *.cpp in the directory are compiled.

function(add_stateful_test)
    get_filename_component(TEST_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    set(TEST_TARGET "test_stateful_${TEST_NAME}")

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

    add_executable(${TEST_TARGET} ${TEST_SOURCES})

    target_link_libraries(${TEST_TARGET}
        PRIVATE
            erslib::core erslib::contrib erslib::flux erslib::modkit erslib::easy_ecs erslib::luaschema
            Catch2::Catch2WithMain
    )

    target_include_directories(${TEST_TARGET} PRIVATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")

    target_compile_definitions(${TEST_TARGET}
        PRIVATE
            TEST_CWD="${CMAKE_CURRENT_BINARY_DIR}"
            TEST_TAGS="[stateful]"
    )

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
        add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/res"
                "${CMAKE_CURRENT_BINARY_DIR}/res"
            COMMENT "Copying resources for ${TEST_TARGET}"
        )
    endif()

    catch_discover_tests(${TEST_TARGET} TEST_PREFIX "${TEST_NAME}::" DISCOVERY_MODE PRE_TEST ADD_TAGS_AS_LABELS)

    message(STATUS "Discovered stateful test: ${TEST_NAME} -> Target: ${TEST_TARGET}")
endfunction()


function(add_sandbox_test)
    get_filename_component(TEST_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    set(TEST_TARGET "test_sandbox_${TEST_NAME}")

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

    add_executable(${TEST_TARGET} ${TEST_SOURCES})

    target_link_libraries(${TEST_TARGET}
        PRIVATE
            erslib::core erslib::contrib erslib::flux erslib::modkit erslib::easy_ecs erslib::luaschema
    )

    target_compile_definitions(${TEST_TARGET}
        PRIVATE
            TEST_CWD="${CMAKE_CURRENT_SOURCE_DIR}"
    )

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
        add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/res"
                "${CMAKE_CURRENT_BINARY_DIR}/res"
            COMMENT "Copying resources for ${TEST_TARGET}"
        )
    endif()

    # Sandboxes are interactive (windows, manual checks), so they're not registered with
    # CTest and never show up in Test Explorer / ctest runs. Launch them as startup items.

    message(STATUS "Discovered sandbox test: ${TEST_NAME} -> Target: ${TEST_TARGET}")
endfunction()


#----------------------------------------------------------------------------------------------------------------------
# stateful + sandbox test discovery
#----------------------------------------------------------------------------------------------------------------------


# Each test under stateful/ and sandbox/ owns a CMakeLists.txt (typically a single
# add_stateful_test()/add_sandbox_test() call). Drop in a new directory with its
# own CMakeLists.txt and it is picked up on the next configure.
foreach(CATEGORY IN ITEMS stateful sandbox)
    set(CATEGORY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${CATEGORY}")

    if(NOT IS_DIRECTORY "${CATEGORY_DIR}")
        continue()
    endif()

    file(GLOB ENTRIES CONFIGURE_DEPENDS
        LIST_DIRECTORIES true
        RELATIVE "${CATEGORY_DIR}"
        "${CATEGORY_DIR}/*"
    )

    foreach(ENTRY IN LISTS ENTRIES)
        if(IS_DIRECTORY "${CATEGORY_DIR}/${ENTRY}" AND EXISTS "${CATEGORY_DIR}/${ENTRY}/CMakeLists.txt")
            add_subdirectory("${CATEGORY}/${ENTRY}")
        endif()
    endforeach()
endforeach()
