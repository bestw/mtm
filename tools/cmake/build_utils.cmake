# CMake additional macros and functions
# Creator: WANG Jun
# Created Time: 2017-06-29
# Last Change: 2017-07-19
#
# update:
# 2017-07-19    WANG Jun    add FW_STATUS()

macro(FW_OPTION variable description value)
    set(__value ${value})
    set(__condition "")
    set(__varname "__value")
    foreach(arg ${ARGN})
        if(arg STREQUAL "IF" OR arg STREQUAL "if")
            set(__varname "__condition")
        else()
            list(APPEND ${__varname} ${arg})
        endif()
    endforeach()
    unset(__varname)
    if(__condition STREQUAL "")
        set(__condition 2 GREATER 1)
    endif()

    if(${__condition})
        if(__value MATCHES ";")
            if(${__value})
                option(${variable} "${description}" ON)
            else()
                option(${variable} "${description}" OFF)
            endif()
        elseif(DEFINED ${__value})
            if(${__value})
                option(${variable} "${description}" ON)
            else()
                option(${variable} "${description}" OFF)
            endif()
        else()
            option(${variable} "${description}" ${__value})
        endif()
    else()
        unset(${variable} CACHE)
    endif()
    unset(__condition)
    unset(__value)
endmacro()

macro(FW_STATUS strings)
    set(__condition "")
    foreach(arg ${ARGN})
        if(arg STREQUAL "IF" OR arg STREQUAL "if")
            unset(__condition)
            set(__condition "")
        else()
            list(APPEND __condition ${arg})
        endif()
    endforeach()
    if(__condition STREQUAL "")
        set(__condition 2 GREATER 1)
    endif()

    if(${__condition})
        message(STATUS ${strings})
    endif()
    unset(__condition)
endmacro()

# Return all sub directories relative path list.
function(list_sub_directories ROOT_DIR SUB_DIR_LIST)
    if(IS_DIRECTORY ${ROOT_DIR})
        file(GLOB ALL_SUB LIST_DIRECTORIES true RELATIVE ${ROOT_DIR} ${ROOT_DIR}/*)
        foreach(__sub ${ALL_SUB})
            if(IS_DIRECTORY ${ROOT_DIR}/${__sub})
                list(APPEND SUB_DIR_LIST ${__sub})
            endif()
        endforeach()
        set(${SUB_DIR_LIST} PARENT_SCOPE)
    endif()
endfunction()

# add_custom_target(dist-clean
#    COMMAND ${CMAKE_BUILD_TOOL} clean
#    COMMAND ${CMAKE_COMMAND} -P clean-all.cmake
# )

# vim:set et ts=4 sts=4 sw=4:
