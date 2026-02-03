function(erslib_setup_target target_name)
    if(TARGET erslib)
        if(WIN32 AND TARGET ${target_name})
            get_target_property(erslib_type erslib TYPE)
            if(erslib_type STREQUAL "SHARED_LIBRARY")
                add_custom_command(TARGET ${target_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:erslib>"
                    "$<TARGET_FILE_DIR:${target_name}>"
                )
            endif()
        endif()
    endif()
endfunction()
