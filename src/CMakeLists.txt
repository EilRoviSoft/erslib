set(COMMON_LIBRARIES
    fmt::fmt
    Boost::serialization
)

# erslib

add_library(erslib
    STATIC
        "erslib/error/error.cpp"

        "erslib/splitting/regular.cpp"
        "erslib/splitting/smart.cpp"
 )

target_compile_features(erslib PUBLIC
    cxx_std_23
)

target_link_libraries(erslib
    PRIVATE
        ${COMMON_LIBRARIES}
)

set_target_properties(erslib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(erslib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# ers_main

add_executable(ers_main
    "main.cpp"
)

target_link_libraries(ers_main
    PRIVATE
        erslib
        erslib_headers
        ${COMMON_LIBRARIES}
)
