set(COMMON_LIBRARIES)

if(TARGET fmt::fmt)
    list(APPEND COMMON_LIBRARIES fmt::fmt)
elseif(fmt_FOUND)
    list(APPEND COMMON_LIBRARIES fmt)
endif()

if(TARGET Boost::serialization)
    list(APPEND COMMON_LIBRARIES Boost::serialization)
elseif(Boost_FOUND)
    list(APPEND COMMON_LIBRARIES ${Boost_SERIALIZATION_LIBRARY})
endif()

if(TARGET Boost::unordered)
    list(APPEND COMMON_LIBRARIES Boost::unordered)
elseif(Boost_FOUND)
    list(APPEND COMMON_LIBRARIES ${Boost_UNORDERED_LIBRARY})
endif()

# erslib

set(LIBRARY_EXECUTABLES
    "erslib/error/error.cpp"

    "erslib/splitting/regular.cpp"
    "erslib/splitting/smart.cpp"
)

if(ERSLIB_STANDALONE)
    add_library(erslib STATIC ${LIBRARY_EXECUTABLES})
else()
    add_library(erslib ${LIBRARY_EXECUTABLES})
endif()

target_compile_features(erslib
    PUBLIC
        cxx_std_26
)

target_include_directories(erslib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
)

if(COMMON_LIBRARIES)
    target_link_libraries(erslib
        PUBLIC
            ${COMMON_LIBRARIES}
    )
endif()

set_target_properties(erslib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME erslib
)

# ers_main

if(ERSLIB_STANDALONE AND ERSLIB_BUILD_EXECUTABLES)
    add_executable(ers_main "main.cpp")
    target_link_libraries(ers_main PRIVATE erslib)
    
    set_target_properties(ers_main PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
