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
include(cmake/dbio.cmake)
include(GNUInstallDirs)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" IS_TOP_LEVEL)

option(ERSLIB_SHARED_LIBS "Build as .dll/.so" OFF)
option(ERSLIB_BUILD_TESTS "Build erslib tests" OFF)
option(ERSLIB_INSTALL "Generate target for installing erslib" ${IS_TOP_LEVEL})
option(ERSLIB_STANDALONE "Build as standalone project (includes tests/executables)" OFF)

option(ERSLIB_BUILD_CONTRIB "Build erslib::contrib target" OFF)
option(ERSLIB_BUILD_DBIO "Build erslib::dbio target" OFF)
option(ERSLIB_BUILD_AENGINE "Build erslib::aengine target" OFF)
option(ERSLIB_BUILD_EASY_ECS "Build erslib::easy_ecs target" OFF)
option(ERSLIB_BUILD_AESCRIPT "Build erslib::aescript target" OFF)

set_if_undefined(ERSLIB_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/erslib"
    CACHE STRING "Install path for erslib package-related CMake files"
)

option(ERSLIB_DBIO_OWN_QUERY_STORE "Init own 'QueryStore queries' for sql." OFF)

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


#----------------------------------------------------------------------------------------------------------------------
# helper functions
#----------------------------------------------------------------------------------------------------------------------


function(erslib_add_library target_name alias_name)
    if(ERSLIB_SHARED_LIBS)
        add_library(${target_name} SHARED)
    else()
        add_library(${target_name} STATIC)
    endif()

    add_library(erslib::${alias_name} ALIAS ${target_name})

    set_target_properties(${target_name} PROPERTIES EXPORT_NAME ${alias_name})

    target_compile_features(${target_name} PUBLIC cxx_std_23)

    target_include_directories(${target_name}
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
    )

    target_compile_definitions(${target_name}
        PUBLIC
            "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:ERSLIB_STATIC_DEFINE>"
    )

    message(STATUS "erslib: added target ${target_name} with alias ${alias_name}")
endfunction()


function(erslib_optional_package target_name package_name dep_target_name compile_definition)
    find_package(${package_name} CONFIG QUIET)

    if(TARGET ${dep_target_name})
        message(STATUS "erslib: found optional dependency ${package_name} for ${target_name}")
        target_compile_definitions(${target_name} PUBLIC ${compile_definition})
        target_link_libraries(${target_name} PUBLIC ${dep_target_name})
    else()
        message(STATUS "erslib: optional dependency ${package_name} not found")
    endif()
endfunction()


function(erslib_required_package target_name package_name dep_target_name)
    find_package(${package_name} CONFIG REQUIRED)
    target_link_libraries(${target_name} PUBLIC ${dep_target_name})
endfunction()


set(ERSLIB_TARGETS "")


#----------------------------------------------------------------------------------------------------------------------
# erslib::core
#----------------------------------------------------------------------------------------------------------------------


erslib_add_library(erslib_core core)

target_sources(erslib_core
    PRIVATE
        "include/erslib/export.hpp"

        "src/erslib/core/assert.cpp"

        "src/erslib/core/exception/internal.cpp"

        "src/erslib/core/splitting/regular.cpp"
        "src/erslib/core/splitting/smart.cpp"

        "src/erslib/core/type/error.cpp"

        "src/erslib/core/util/file.cpp"
        "src/erslib/core/util/string.cpp"
)

erslib_optional_package(erslib_core boost_optional Boost::optional _HAS_BOOST_OPTIONAL)
erslib_optional_package(erslib_core boost_smart_ptr Boost::smart_ptr _HAS_BOOST_SMART_PTR)
erslib_optional_package(erslib_core boost_thread Boost::thread _HAS_BOOST_THREAD)
erslib_optional_package(erslib_core cpptrace cpptrace::cpptrace _HAS_CPPTRACE)

if(IS_TOP_LEVEL)
    target_compile_definitions(erslib_core
        PRIVATE
            ERS_ASSERT_INFO
    )
endif()

set(ERS_TRACE_VERBOSITY "1" CACHE STRING "Trace verbosity level for erslib (0 = off, 1 = basic, 2 = full)")

target_compile_definitions(erslib_core
    PUBLIC
        ERS_TRACE_VERBOSITY=${ERS_TRACE_VERBOSITY}
)

if(ERS_TRACE_VERBOSITY STREQUAL "2")
    set(ERS_TRACE_MAX_DEPTH "0" CACHE STRING "Trace verbosity max depth")

    target_compile_definitions(erslib_core
        PUBLIC
            ERS_TRACE_MAX_DEPTH=${ERS_TRACE_MAX_DEPTH}
    )
endif()

list(APPEND ERSLIB_TARGETS erslib_core)


#----------------------------------------------------------------------------------------------------------------------
# erslib::contrib
#----------------------------------------------------------------------------------------------------------------------


if(ERSLIB_BUILD_CONTRIB)
    erslib_add_library(erslib_contrib contrib)

    target_sources(erslib_contrib
        PRIVATE
            "src/erslib/contrib/json.cpp"
    )

    target_link_libraries(erslib_contrib
        PUBLIC
            erslib_core
    )

    erslib_optional_package(erslib_contrib boost_unordered Boost::unordered _HAS_BOOST_UNORDERED)

    list(APPEND ERSLIB_TARGETS erslib_contrib)
endif()


#----------------------------------------------------------------------------------------------------------------------
# erslib::dbio
#----------------------------------------------------------------------------------------------------------------------


if(ERSLIB_BUILD_DBIO)
    erslib_add_library(erslib_dbio dbio)

    if(ERSLIB_DBIO_OWN_QUERY_STORE)
        target_compile_definitions(erslib_dbio
            PUBLIC
                ERS_DBIO_GLOBAL_QUERY_STORE
                ERS_DBIO_GLOBAL_QUERY_STORE_INIT
        )
    endif()

    target_sources(erslib_dbio
        PRIVATE
            "src/erslib/dbio/database.cpp"
            "src/erslib/dbio/query_store.cpp"
    )

    target_link_libraries(erslib_dbio
        PUBLIC
            erslib_core
    )

    find_package(PostgreSQL)
    erslib_required_package(erslib_dbio libpqxx libpqxx::pqxx)

    list(APPEND ERSLIB_TARGETS erslib_dbio)
endif()


#----------------------------------------------------------------------------------------------------------------------
# erslib::aengine
#----------------------------------------------------------------------------------------------------------------------


if(ERSLIB_BUILD_AENGINE)
    erslib_add_library(erslib_aengine aengine)

    target_sources(erslib_aengine
        PRIVATE
            "src/erslib/aengine/dependency.cpp"
            "src/erslib/aengine/dependency_graph.cpp"
            "src/erslib/aengine/mod.cpp"

            "src/erslib/aengine/resource/texture.cpp"

            "src/erslib/aengine/util/stage.cpp"
    )

    target_link_libraries(erslib_aengine
        PUBLIC
            erslib_core
    )

    if(ERSLIB_BUILD_CONTRIB)
        target_link_libraries(erslib_aengine
            PRIVATE
                erslib_contrib
        )
    endif()

    erslib_required_package(erslib_aengine boost_container Boost::container)
    erslib_required_package(erslib_aengine boost_regex Boost::regex)
    erslib_required_package(erslib_aengine boost_unordered Boost::unordered)
    erslib_required_package(erslib_aengine sol2 sol2::sol2)
    
    find_package(PkgConfig)
    pkg_check_modules(LuaJIT IMPORTED_TARGET luajit)
    target_compile_definitions(erslib_aengine PUBLIC _HAS_LUAJIT)
    target_link_libraries(erslib_aengine PUBLIC PkgConfig::LuaJIT)

    erslib_required_package(erslib_aengine SDL3 SDL3::SDL3)
    erslib_required_package(erslib_aengine quill quill::quill)
    erslib_required_package(erslib_aengine RmlUi RmlUi::RmlUi)

    list(APPEND ERSLIB_TARGETS erslib_aengine)
endif()


#----------------------------------------------------------------------------------------------------------------------
# erslib::easy_ecs
#----------------------------------------------------------------------------------------------------------------------


if(ERSLIB_BUILD_EASY_ECS)
    if(NOT ERSLIB_BUILD_AENGINE)
        message(FATAL_ERROR "ERSLIB_BUILD_EASY_ECS requires ERSLIB_BUILD_AENGINE")
    endif()

    erslib_add_library(erslib_easy_ecs easy_ecs)

    target_sources(erslib_easy_ecs
        PRIVATE
            "src/erslib/easy_ecs/entity.cpp"
            "src/erslib/easy_ecs/group.cpp"
            "src/erslib/easy_ecs/registry.cpp"
    )

    target_link_libraries(erslib_easy_ecs
        PUBLIC
            erslib_aengine
    )

    erslib_required_package(erslib_easy_ecs frozen frozen::frozen _HAS_FROZEN)

    list(APPEND ERSLIB_TARGETS erslib_easy_ecs)
endif()


#----------------------------------------------------------------------------------------------------------------------
# erslib::aescript
#----------------------------------------------------------------------------------------------------------------------


if(ERSLIB_BUILD_AESCRIPT)
    if(NOT ERSLIB_BUILD_AENGINE)
        message(FATAL_ERROR "ERSLIB_BUILD_AESCRIPT requires ERSLIB_BUILD_AENGINE")
    endif()

    erslib_add_library(erslib_aescript aescript)

    target_sources(erslib_aescript
        PRIVATE
            "src/erslib/aescript/impl/field.cpp"
            "src/erslib/aescript/impl/layout.cpp"
            "src/erslib/aescript/impl/util.cpp"

            "src/erslib/aescript/properties/exclusive_with.cpp"
            "src/erslib/aescript/properties/inclusive_with.cpp"
            "src/erslib/aescript/properties/one_of.cpp"
            "src/erslib/aescript/properties/presence.cpp"
    )

    target_link_libraries(erslib_aescript
        PUBLIC
            erslib_aengine
    )

    list(APPEND ERSLIB_TARGETS erslib_aescript)
endif()


#----------------------------------------------------------------------------------------------------------------------
# export header
#----------------------------------------------------------------------------------------------------------------------


include(GenerateExportHeader)
set(EXPORT_FILE_NAME "export_shared.hpp")

if(NOT BUILD_SHARED_LIBS)
    set(EXPORT_FILE_NAME "export_static.hpp")
endif()

generate_export_header(erslib_core
    BASE_NAME erslib
    EXPORT_FILE_NAME include/erslib/${EXPORT_FILE_NAME}
)


#----------------------------------------------------------------------------------------------------------------------
# install
#----------------------------------------------------------------------------------------------------------------------


if(ERSLIB_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        cmake/erslib-config.cmake.in
        erslib-config.cmake
        INSTALL_DESTINATION "${ERSLIB_INSTALL_CMAKEDIR}"
    )

    write_basic_package_version_file(
        erslib-config-version.cmake
        COMPATIBILITY SameMajorVersion
    )

    install(TARGETS ${ERSLIB_TARGETS} EXPORT erslib_export
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

    # dbio codegen integration: the dbio_generate() function plus the Python
    # codegen package it drives, so downstream find_package(erslib) can generate.
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dbio.cmake"
        COMPONENT erslib_dev
        DESTINATION "${ERSLIB_INSTALL_CMAKEDIR}"
    )

    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/"
        COMPONENT erslib_dev
        DESTINATION "${ERSLIB_INSTALL_CMAKEDIR}/scripts"
        FILES_MATCHING
            PATTERN "*.py"
            PATTERN "*.jinja"
    )
endif()


#----------------------------------------------------------------------------------------------------------------------
# other targets
#----------------------------------------------------------------------------------------------------------------------


if(ERSLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
