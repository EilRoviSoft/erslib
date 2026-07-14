if(NOT DEFINED ERSLIB_DBIO_SCRIPTS_DIR)
    set(_erslib_dbio_scripts_dir "")
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../scripts/__main__.py")
        get_filename_component(_erslib_dbio_scripts_dir "${CMAKE_CURRENT_LIST_DIR}/../scripts" ABSOLUTE)
    elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/scripts/__main__.py")
        set(_erslib_dbio_scripts_dir "${CMAKE_CURRENT_LIST_DIR}/scripts")
    endif()

    set(ERSLIB_DBIO_SCRIPTS_DIR "${_erslib_dbio_scripts_dir}" CACHE PATH
        "Directory containing the dbio codegen scripts (__main__.py)")
    unset(_erslib_dbio_scripts_dir)
endif()


# dbio_generate(
#   IMPORT_DIR <dir>            # directory scanned recursively for '*.g.json' (required)
#   HPP_DIR    <dir>            # output directory for generated headers   (required)
#   CPP_DIR    <dir>            # output directory for generated sources   (required)
#   QUERY_DIR  <dir>            # output directory for generated SQL        (required)
#   [TARGET    <target>]        # add generated sources + HPP_DIR include to this target
#   [OUT_VAR   <var>]           # set <var> to the ';'-list of generated .cpp files
#   [NAMESPACE <ns>]            # dbio runtime namespace (default: dbio)
#   [WORKING_DIRECTORY <dir>]   # cwd for the generator (default: CMAKE_CURRENT_SOURCE_DIR)
# )
#
# Generation runs at configure time, so the set of generated sources is known
# when the target is built. Re-run CMake configuration after adding/removing
# descriptors.
function(dbio_generate)
    set(options "")
    set(one_value_args TARGET OUT_VAR IMPORT_DIR HPP_DIR CPP_DIR QUERY_DIR NAMESPACE WORKING_DIRECTORY)
    set(multi_value_args "")
    cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    foreach(required IMPORT_DIR HPP_DIR CPP_DIR QUERY_DIR)
        if(NOT ARG_${required})
            message(FATAL_ERROR "dbio_generate: ${required} is required")
        endif()
    endforeach()

    if(NOT ARG_NAMESPACE)
        set(ARG_NAMESPACE "dbio")
    endif()

    if(NOT ARG_WORKING_DIRECTORY)
        set(ARG_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    if(NOT ERSLIB_DBIO_SCRIPTS_DIR)
        message(FATAL_ERROR "dbio_generate: could not locate the dbio codegen scripts; "
            "set ERSLIB_DBIO_SCRIPTS_DIR to the directory containing __main__.py")
    endif()

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    # Re-run configuration (and therefore regeneration) whenever a descriptor or a
    # referenced .sql file under IMPORT_DIR is added, removed, or edited. Generation
    # happens here at configure time via execute_process, so without this the build
    # would keep using stale generated sources.
    get_filename_component(_dbio_import_abs "${ARG_IMPORT_DIR}" ABSOLUTE BASE_DIR "${ARG_WORKING_DIRECTORY}")
    file(GLOB_RECURSE _dbio_gen_inputs CONFIGURE_DEPENDS
        "${_dbio_import_abs}/*.g.json"
        "${_dbio_import_abs}/*.sql")
    if(_dbio_gen_inputs)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_dbio_gen_inputs})
    endif()

    message(STATUS "dbio_generate: generating from '${ARG_IMPORT_DIR}' (namespace ${ARG_NAMESPACE})")

    execute_process(
        WORKING_DIRECTORY "${ARG_WORKING_DIRECTORY}"
        COMMAND "${Python3_EXECUTABLE}" "${ERSLIB_DBIO_SCRIPTS_DIR}" codegen
            --dir "${ARG_IMPORT_DIR}"
            --hpp-dir "${ARG_HPP_DIR}"
            --cpp-dir "${ARG_CPP_DIR}"
            --query-dir "${ARG_QUERY_DIR}"
            --runtime-namespace "${ARG_NAMESPACE}"
        OUTPUT_VARIABLE dbio_sources
        ERROR_VARIABLE dbio_error
        RESULT_VARIABLE dbio_result
    )

    if(NOT dbio_result EQUAL 0)
        message(FATAL_ERROR "dbio_generate: codegen failed:\n${dbio_error}")
    endif()

    if(ARG_TARGET)
        if(dbio_sources)
            target_sources(${ARG_TARGET} PRIVATE ${dbio_sources})
        endif()

        target_include_directories(${ARG_TARGET} PRIVATE "${ARG_HPP_DIR}")

        if(TARGET erslib::dbio)
            target_link_libraries(${ARG_TARGET} PRIVATE erslib::dbio)
        endif()
    endif()

    if(ARG_OUT_VAR)
        set(${ARG_OUT_VAR} "${dbio_sources}" PARENT_SCOPE)
    endif()
endfunction()
