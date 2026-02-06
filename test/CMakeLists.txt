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

#----------------------------------------------------------------------------------------------------------------------
# tests sources
#----------------------------------------------------------------------------------------------------------------------

set(SOURCES
    "case/autorng.cpp"
    "case/easy_ecs.cpp"
    "case/error.cpp"
    "case/splitting.cpp"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

#----------------------------------------------------------------------------------------------------------------------
# tests target
#----------------------------------------------------------------------------------------------------------------------

add_executable(erslib_test)
target_sources(erslib_test PRIVATE ${SOURCES})

target_link_libraries(erslib_test
    PRIVATE
        erslib::erslib
        Catch2::Catch2WithMain
)

if(NOT IS_TOP_LEVEL)
    win_copy_deps_to_target_dir(erslib_test erslib::erslib)
endif()

add_test(NAME erslib_test COMMAND erslib_test)
