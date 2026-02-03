cmake_minimum_required(VERSION 3.28)

project(erslib
    VERSION 1.0.1
    DESCRIPTION "EilRoviSoft's universal library"
    LANGUAGES CXX
)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ERSLIB_BUILD_TESTS "Build tests" OFF)
option(ERSLIB_BUILD_EXAMPLES "Build examples" OFF)
option(ERSLIB_BUILD_EXECUTABLES "Build executables" OFF)
option(ERSLIB_STANDALONE "Build as standalone project (includes tests/executables)" OFF)

if(ERSLIB_STANDALONE AND DEFINED ENV{VCPKG_ROOT})
    include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)
if(IS_SUBPROJECT)
    set(ERSLIB_STANDALONE OFF CACHE BOOL "Build as standalone project" FORCE)
endif()

# packages

if(ERSLIB_STANDALONE)
    find_package(fmt CONFIG REQUIRED)
    find_package(boost_unordered CONFIG REQUIRED)
else()
    find_package(fmt QUIET)
    find_package(boost_unordered QUIET)
endif()

if(ERSLIB_STANDALONE AND ERSLIB_BUILD_TESTS)
    find_package(Catch2 REQUIRED)
    add_subdirectory(test)
    enable_testing()
endif()

add_subdirectory(src)

# adding sources

include(GNUInstallDirs)

install(TARGETS erslib
    EXPORT erslib-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/include
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/erslib-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/erslib-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/erslib
)

install(EXPORT erslib-targets
    FILE erslib-targets.cmake
    NAMESPACE erslib::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/erslib
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/erslib-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/erslib
)

message(STATUS "erslib ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
