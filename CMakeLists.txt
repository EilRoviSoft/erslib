cmake_minimum_required(VERSION 3.28)

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(erslib
    VERSION 0.1.0
    DESCRIPTION "EilRoviSoft's universal library"
)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

if(BUILD_TESTS)
    find_package(Catch2 REQUIRED)
endif()

include(GNUInstallDirs)

# packages

find_package(fmt CONFIG REQUIRED)
find_package(boost_serialization CONFIG REQUIRED)

# adding sources

add_library(erslib_headers INTERFACE)

target_include_directories(erslib_headers INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(test)
    enable_testing()
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

install(TARGETS erslib
    EXPORT ers_targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ers_targets
    FILE erslib_config.cmake
    NAMESPACE erslib
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/erslib
)

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
