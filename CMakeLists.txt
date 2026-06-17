cmake_minimum_required(VERSION 3.28)

project(erslib
    VERSION 1.2.0
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

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(ERSLIB_SHARED_LIBS)
    add_library(erslib SHARED)
else()
    add_library(erslib STATIC)
endif()

add_library(erslib::erslib ALIAS erslib)

target_compile_features(erslib PUBLIC cxx_std_23)


#----------------------------------------------------------------------------------------------------------------------
# erslib dependencies
#----------------------------------------------------------------------------------------------------------------------


set(DEPENDENCIES "")


function(erslib_optional_package package_name target_name compile_definition)
    find_package(${package_name} CONFIG QUIET)

    if(TARGET ${target_name})
        message(STATUS "erslib: found optional dependency ${package_name}")
        target_compile_definitions(erslib PUBLIC ${compile_definition})
        target_link_libraries(erslib PUBLIC ${target_name})
    else()
        message(STATUS "erslib: optional dependency ${package_name} not found")
    endif()
endfunction()


erslib_optional_package(boost_container Boost::container _HAS_BOOST_CONTAINER)
erslib_optional_package(boost_optional Boost::optional _HAS_BOOST_OPTIONAL)
erslib_optional_package(boost_regex Boost::regex _HAS_BOOST_REGEX)
erslib_optional_package(boost_smart_ptr Boost::smart_ptr _HAS_BOOST_SMART_PTR)
erslib_optional_package(boost_thread Boost::thread _HAS_BOOST_THREAD)
erslib_optional_package(boost_unordered Boost::unordered _HAS_BOOST_UNORDERED)
erslib_optional_package(cpptrace cpptrace::cpptrace _HAS_CPPTRACE)
erslib_optional_package(frozen frozen::frozen _HAS_FROZEN)
erslib_optional_package(sol2 sol2::sol2 _HAS_SOL2)
erslib_optional_package(SDL3 SDL3::SDL3 _HAS_SDL3)
erslib_optional_package(quill quill::quill _HAS_QUILL)
erslib_optional_package(RmlUi RmlUi::RmlUi _HAS_RMLUI)


find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(LuaJIT QUIET IMPORTED_TARGET luajit)

    if(TARGET PkgConfig::LuaJIT)
        message(STATUS "erslib: found optional dependency LuaJIT")
        target_compile_definitions(erslib PUBLIC _HAS_LUAJIT)
        target_link_libraries(erslib PUBLIC PkgConfig::LuaJIT)
    else()
        message(STATUS "erslib: optional dependency LuaJIT not found")
    endif()
else()
    message(STATUS "erslib: PkgConfig not found, skipping LuaJIT")
endif()


target_link_libraries(erslib
    PUBLIC
        ${DEPENDENCIES}
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
    # erslib

    "include/erslib/export.hpp"

    "src/erslib/assert.cpp"

    "src/erslib/exception/internal.cpp"

    "src/erslib/splitting/regular.cpp"
    "src/erslib/splitting/smart.cpp"

    "src/erslib/type/error.cpp"

    "src/erslib/util/file.cpp"
    "src/erslib/util/string.cpp"    


    # contrib

    "src/contrib/json.cpp"


    # aengine

    "src/aengine/core/dependency.cpp"
    "src/aengine/core/dependency_graph.cpp"
    "src/aengine/core/mod.cpp"

    "src/aengine/resource/texture.cpp"

    "src/aengine/util/stage.cpp"


    # aescript

    "src/aescript/impl/field.cpp"
    "src/aescript/impl/layout.cpp"

    "src/aescript/properties/exclusive_with.cpp"
    "src/aescript/properties/presence.cpp"


    # easy_ecs

    "src/easy_ecs/entity.cpp"
    "src/easy_ecs/group.cpp"
    "src/easy_ecs/registry.cpp"
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


if(IS_TOP_LEVEL)
    target_compile_definitions(erslib
        PRIVATE
            ERS_ASSERT_INFO
    )
endif()


set(ERS_TRACE_VERBOSITY "1" CACHE STRING "Trace verbosity level for erslib (0 = off, 1 = basic, 2 = full)")

target_compile_definitions(erslib
    PUBLIC 
        ERS_TRACE_VERBOSITY=${ERS_TRACE_VERBOSITY}
)

if(ERS_TRACE_VERBOSITY STREQUAL "2")
    set(ERS_TRACE_MAX_DEPTH "0" CACHE STRING "Trace verbosity max depth")

    target_compile_definitions(erslib
        PUBLIC 
            ERS_TRACE_MAX_DEPTH=${ERS_TRACE_MAX_DEPTH}
    )
endif()


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

    if(BUILD_SHARED_LIBS)
        set(TARGETS_FILE "erslib-shared-targets.cmake")
    else()
        set(TARGETS_FILE "erslib-static-targets.cmake")
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
