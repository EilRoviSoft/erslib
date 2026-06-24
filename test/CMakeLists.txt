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

find_package(doctest CONFIG REQUIRED)


#----------------------------------------------------------------------------------------------------------------------
# stateless tests
#----------------------------------------------------------------------------------------------------------------------


file(GLOB SIMPLE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/simple/*.cpp"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SIMPLE_SOURCES})


add_executable(test_simple)
target_sources(test_simple PRIVATE ${SIMPLE_SOURCES} "simple.cpp")

target_link_libraries(test_simple
    PRIVATE
        erslib::core erslib::contrib erslib::dbio erslib::aengine erslib::easy_ecs erslib::aescript
        doctest::doctest
)

target_compile_definitions(test_simple
    PRIVATE
        DOCTEST_CONFIG_USE_STD_HEADERS
)


if(NOT IS_TOP_LEVEL)
    win_copy_deps_to_target_dir(test_simple erslib::core erslib::contrib erslib::aengine erslib::easy_ecs erslib::aescript)
endif()

target_compile_definitions(erslib_core
    PRIVATE
        ERS_ASSERT_INFO
)

add_test(NAME test_simple COMMAND test_simple)


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
            erslib::core erslib::contrib erslib::dbio erslib::aengine erslib::easy_ecs erslib::aescript
            doctest::doctest
    )

    target_compile_definitions(${TEST_TARGET}
        PRIVATE
            DOCTEST_CONFIG_USE_STD_HEADERS
            TEST_CWD="${CMAKE_CURRENT_BINARY_DIR}"
    )

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
        add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/res"
                "${CMAKE_CURRENT_BINARY_DIR}/res"
            COMMENT "Copying resources for ${TEST_TARGET}"
        )
    endif()

    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

    message(STATUS "Discovered stateful test: ${TEST_NAME} -> Target: ${TEST_TARGET}")
endfunction()


function(add_sandbox_test)
    get_filename_component(TEST_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    set(TEST_TARGET "test_sandbox_${TEST_NAME}")

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

    add_executable(${TEST_TARGET} ${TEST_SOURCES})

    target_link_libraries(${TEST_TARGET}
        PRIVATE
            erslib::core erslib::contrib erslib::dbio erslib::aengine erslib::easy_ecs erslib::aescript
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

    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

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
