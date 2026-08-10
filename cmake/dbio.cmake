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
#   IMPORT_DIR    <dir>
#   HPP_DIR       <dir>
#   CPP_DIR       <dir>
#   IN_QUERY_DIR  <dir>
#   OUT_QUERY_DIR <dir>
#   [TARGET    <target>]
#   [OUT_VAR   <var>]
#   [NAMESPACE <ns>]
#   [WORKING_DIRECTORY <dir>]
#   [FILE_LIST <file>]
# )
#
# IN_QUERY_DIR is scanned (read-only) alongside IMPORT_DIR so that hand-authored *.sql / *.g.json
# content living there (e.g. sql_file references) triggers a reconfigure when edited. OUT_QUERY_DIR
# is where codegen actually writes generated SQL and is passed through as --query-dir. The two are
# almost always different directories (source tree vs. build tree) - keeping them separate avoids
# mixing hand-authored and generated content under the same path.
function(dbio_generate)
    set(one_value_args TARGET OUT_VAR IMPORT_DIR HPP_DIR CPP_DIR IN_QUERY_DIR OUT_QUERY_DIR NAMESPACE WORKING_DIRECTORY FILE_LIST)
    set(multi_value_args "")
    cmake_parse_arguments(ARG "" "${one_value_args}" "${multi_value_args}" ${ARGN})

    foreach(required IMPORT_DIR HPP_DIR CPP_DIR IN_QUERY_DIR OUT_QUERY_DIR)
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

    find_package(Python3 3.14 COMPONENTS Interpreter REQUIRED)

    get_filename_component(_dbio_import_abs "${ARG_IMPORT_DIR}" ABSOLUTE BASE_DIR "${ARG_WORKING_DIRECTORY}")
    file(GLOB_RECURSE _dbio_gen_inputs CONFIGURE_DEPENDS
        "${_dbio_import_abs}/*.g.json"
        "${_dbio_import_abs}/*.sql")

    get_filename_component(_dbio_query_abs "${ARG_IN_QUERY_DIR}" ABSOLUTE BASE_DIR "${ARG_WORKING_DIRECTORY}")
    file(GLOB_RECURSE _dbio_query_gen_inputs CONFIGURE_DEPENDS
        "${_dbio_query_abs}/*.g.json"
        "${_dbio_query_abs}/*.sql")
    if(_dbio_query_gen_inputs)
        list(FILTER _dbio_query_gen_inputs EXCLUDE REGEX "\\.g\\.sql$")
        list(APPEND _dbio_gen_inputs ${_dbio_query_gen_inputs})
    endif()

    if(_dbio_gen_inputs)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_dbio_gen_inputs})
    endif()

    message(STATUS "dbio_generate: generating from '${ARG_IMPORT_DIR}' (namespace ${ARG_NAMESPACE})")

    set(_dbio_extra_args "")

    if(ARG_FILE_LIST)
        list(APPEND _dbio_extra_args --file-list "${ARG_FILE_LIST}")
    endif()

    execute_process(
        WORKING_DIRECTORY "${ARG_WORKING_DIRECTORY}"
        COMMAND "${Python3_EXECUTABLE}" "${ERSLIB_DBIO_SCRIPTS_DIR}" codegen
            --dir "${ARG_IMPORT_DIR}"
            --hpp-dir "${ARG_HPP_DIR}"
            --cpp-dir "${ARG_CPP_DIR}"
            --in-query-dir "${ARG_IN_QUERY_DIR}"
            --out-query-dir "${ARG_OUT_QUERY_DIR}"
            --runtime-namespace "${ARG_NAMESPACE}"
            ${_dbio_extra_args}
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
