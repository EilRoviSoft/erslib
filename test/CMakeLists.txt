cmake_minimum_required(VERSION 3.28)
project(erslib_test)


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

find_package(Catch2 CONFIG REQUIRED)
include(Catch)


#----------------------------------------------------------------------------------------------------------------------
# stateless tests
#----------------------------------------------------------------------------------------------------------------------


file(GLOB SIMPLE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/simple/*.cpp"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SIMPLE_SOURCES})


add_executable(erslib_test)
target_sources(erslib_test PRIVATE ${SIMPLE_SOURCES})

target_link_libraries(erslib_test
    PRIVATE
        erslib::erslib
        Catch2::Catch2WithMain
)

if(NOT IS_TOP_LEVEL)
    win_copy_deps_to_target_dir(erslib_test erslib::erslib)
endif()

target_compile_definitions(erslib
    PRIVATE
        ERS_ASSERT_INFO
)

add_test(NAME erslib_test COMMAND erslib_test)


#----------------------------------------------------------------------------------------------------------------------
# statefull tests
#----------------------------------------------------------------------------------------------------------------------


function(add_stateful_test TEST_TARGET RELATIVE_TEST_DIR)
	set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${RELATIVE_TEST_DIR}")
    set(BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/${RELATIVE_TEST_DIR}")

	file(GLOB TEST_SOURCES "${SRC_DIR}/*.cpp")

	add_executable(${TEST_TARGET} ${TEST_SOURCES})

	target_link_libraries(${TEST_TARGET} 
        PRIVATE
            erslib::erslib
            Catch2::Catch2WithMain
    )

	file(MAKE_DIRECTORY "${BIN_DIR}")

	if(EXISTS "${SRC_DIR}/res")
        add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${SRC_DIR}/res"
                "${BIN_DIR}/res"
            COMMENT "Copying resources for ${TEST_TARGET}"
        )
    endif()

    catch_discover_tests(${TEST_TARGET}
        PROPERTIES WORKING_DIRECTORY "${BIN_DIR}"
    )

    target_compile_definitions(${TEST_TARGET} 
        PRIVATE 
            TEST_CWD="${BIN_DIR}"
    )
endfunction()


set(STATEFUL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stateful")

file(GLOB POTENTIAL_TEST_DIRS CONFIGURE_DEPENDS 
    LIST_DIRECTORIES true 
    RELATIVE "${STATEFUL_DIR}" 
    "${STATEFUL_DIR}/*"
)

foreach(DIR_NAME IN LISTS POTENTIAL_TEST_DIRS)
    set(FULL_DIR_PATH "${STATEFUL_DIR}/${DIR_NAME}")
    if(IS_DIRECTORY "${FULL_DIR_PATH}")
        set(TARGET_NAME "test_stateful_${DIR_NAME}")    
        add_stateful_test(${TARGET_NAME} "stateful/${DIR_NAME}")
        message(STATUS "Discovered stateful test: ${DIR_NAME} -> Target: ${TARGET_NAME}")
    endif()
endforeach()
