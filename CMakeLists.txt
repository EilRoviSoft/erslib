cmake_minimum_required(VERSION 3.28)

project(erslib
    VERSION 1.1.0
    DESCRIPTION "EilRoviSoft's universal library"
    LANGUAGES CXX
)

#----------------------------------------------------------------------------------------------------------------------
# general settings and options
#----------------------------------------------------------------------------------------------------------------------

include(cmake/utils.cmake)
include(GNUInstallDirs)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" IS_TOP_LEVEL)

option(ERSLIB_SHARED_LIBS "Build as .dll/.so" OFF)
option(ERSLIB_BUILD_TESTS "Build erslib tests" OFF)
option(ERSLIB_INSTALL "Generate target for installing erslib" ${IS_TOP_LEVEL})
option(ERSLIB_STANDALONE "Build as standalone project (includes tests/executables)" OFF)

set_if_undefined(ERSLIB_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/erslib"
    CACHE STRING "Install path for erslib package-related CMake files"
)

if(DEFINED ERSLIB_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${ERSLIB_SHARED_LIBS})
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(IS_TOP_LEVEL AND DEFINED ENV{VCPKG_ROOT})
    include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

add_library(erslib)
add_library(erslib::erslib ALIAS erslib)

#----------------------------------------------------------------------------------------------------------------------
# erslib dependencies
#----------------------------------------------------------------------------------------------------------------------

find_package(absl CONFIG REQUIRED)
find_package(boost_optional CONFIG REQUIRED)
find_package(boost_thread CONFIG REQUIRED)
find_package(boost_unordered CONFIG REQUIRED)

target_link_libraries(erslib
    PUBLIC
        absl::flat_hash_map absl::flat_hash_set
        Boost::optional Boost::thread Boost::unordered
)

#----------------------------------------------------------------------------------------------------------------------
# erslib sources
#----------------------------------------------------------------------------------------------------------------------

include(GenerateExportHeader)
set(EXPORT_FILE_NAME "export_shared.hpp")

if(NOT BUILD_SHARED_LIBS)
    set(EXPORT_FILE_NAME "export_static.hpp")
endif()

generate_export_header(erslib EXPORT_FILE_NAME include/erslib/${EXPORT_FILE_NAME})

set(SOURCES
    "include/erslib/export.hpp"
    "src/erslib/boost-fix.cpp"

    "src/erslib/error/error.cpp"

    "src/erslib/splitting/regular.cpp"
    "src/erslib/splitting/smart.cpp"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

#----------------------------------------------------------------------------------------------------------------------
# erslib target
#----------------------------------------------------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

target_sources(erslib
    PRIVATE
        ${SOURCES}
)

target_compile_definitions(erslib
    PUBLIC
        "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:ERSLIB_STATIC_DEFINE>"
)

target_include_directories(erslib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
)

set_target_properties(erslib PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)

if(ERSLIB_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    configure_package_config_file(
        cmake/erslib-config.cmake.in
        erslib-config.cmake
        INSTALL_DESTINATION "${ERSLIB_INSTALL_CMAKEDIR}"
    )

    write_basic_package_version_file(
        erslib-config-version.cmake
        COMPATIBILITY SameMajorVersion
    )

    install(TARGETS erslib EXPORT erslib_export
        RUNTIME COMPONENT erslib
        LIBRARY COMPONENT erslib NAMELINK_COMPONENT erslib_dev
        ARCHIVE COMPONENT erslib_dev
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    install(
        DIRECTORY include/
        TYPE INCLUDE
        COMPONENT erslib_dev
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/erslib/${EXPORT_FILE_NAME}"
        COMPONENT erslib_dev
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/erslib"
    )

    set(TARGETS_FILE "erslib-static-targets.cmake")

    if(NOT BUILD_SHARED_LIBS)
        set(targets_file "erslib-static-targets.cmake")
    endif()

    install(EXPORT erslib_export
        COMPONENT erslib_dev
        FILE "${TARGETS_FILE}"
        DESTINATION "${ERSLIB_INSTALL_CMAKEDIR}"
        NAMESPACE erslib::
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/erslib-config.cmake"
        COMPONENT erslib_dev
        DESTINATION "${ERSLIB_INSTALL_CMAKEDIR}"
    )
endif()

#----------------------------------------------------------------------------------------------------------------------
# other targets
#----------------------------------------------------------------------------------------------------------------------

if(ERSLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
