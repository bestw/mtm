# arm-none-eabi-gcc arm cortex-m7 settings
# Author: WANG Jun
# Created Time: 2017-07-15
#

if(__GCC_CM7_TOOLCHAIN_INCLUDED)
    return()
endif()
set(__GCC_CM7_TOOLCHAIN_INCLUDED TRUE)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_CROSSCOMPILING TRUE)

if(CMAKE_HOST_WIN32)
    set(EXE_SUFFIX ".exe")
else()
    set(EXE_SUFFIX "")
endif()
set(TOOLCHAIN_PREFIX "arm-none-eabi-")

set(CMAKE_SHARED_LIBRARY_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP "")

set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "")
set(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG_SEP "")

set(CMAKE_LINK_LIBRARY_SUFFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".a")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CMAKE_DL_LIBS "")

set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

find_path(TOOLCHAIN_ROOT_PATH NAMES bin/${TOOLCHAIN_PREFIX}gcc${EXE_SUFFIX})
if(TOOLCHAIN_ROOT_PATH STREQUAL "TOOLCHAIN_ROOT_PATH-NOTFOUND")
    message(FATAL_ERROR "CAN NOT find TOOLCHAIN_ROOT_PATH")
endif()

find_program(CMAKE_C_COMPILER NAMES ${TOOLCHAIN_PREFIX}gcc${EXE_SUFFIX})
find_program(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}g++${EXE_SUFFIX})
find_program(CMAKE_ASM_COMPILER NAMES ${TOOLCHAIN_PREFIX}gcc${EXE_SUFFIX})
find_program(CMAKE_AR NAMES ${TOOLCHAIN_PREFIX}ar${EXE_SUFFIX})
find_program(CMAKE_RANLIB NAMES ${TOOLCHAIN_PREFIX}ranlib${EXE_SUFFIX})
find_program(CMAKE_LINKER NAMES ${TOOLCHAIN_PREFIX}gcc${EXE_SUFFIX})
find_program(CMAKE_NM NAMES ${TOOLCHAIN_PREFIX}nm${PATHEXT})
find_program(CMAKE_OBJCOPY NAMES ${TOOLCHAIN_PREFIX}objcopy${PATHEXT})
find_program(CMAKE_OBJDUMP NAMES ${TOOLCHAIN_PREFIX}objdump${PATHEXT})
find_program(CMAKE_STRIP NAMES ${TOOLCHAIN_PREFIX}strip${PATHEXT})
find_program(CMAKE_SIZE NAMES ${TOOLCHAIN_PREFIX}size${PATHEXT})
find_program(CMAKE_DEBUGGER NAMES ${TOOLCHAIN_PREFIX}gdb${PATHEXT})

message(STATUS "TOOLCHAIN_ROOT_PATH -> ${TOOLCHAIN_ROOT_PATH}")
message(STATUS "CMAKE_C_COMPILER    -> ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER  -> ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_ASM_COMPILER  -> ${CMAKE_ASM_COMPILER}")
message(STATUS "CMAKE_RANLIB        -> ${CMAKE_RANLIB}")
message(STATUS "CMAKE_LINKER        -> ${CMAKE_LINKER}")
message(STATUS "CMAKE_NM            -> ${CMAKE_NM}")
message(STATUS "CMAKE_OBJCOPY       -> ${CMAKE_OBJCOPY}")
message(STATUS "CMAKE_OBJDUMP       -> ${CMAKE_OBJDUMP}")
message(STATUS "CMAKE_STRIP         -> ${CMAKE_STRIP}")
message(STATUS "CMAKE_SIZE          -> ${CMAKE_SIZE}")
message(STATUS "CMAKE_DEBUGGER      -> ${CMAKE_DEBUGGER}\n")

enable_language(ASM)

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_ROOT_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS "-std=c99 -mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -Wall")#-fdata-sections -ffunction-sections
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -gdwarf-2 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")

set(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m7 -Wall -ffunction-sections -fdata-sections -x assembler-with-cpp -Wa,-mimplicit-it=thumb")
set(CMAKE_ASM_FLAGS_DEBUG "-g -gdwarf-2 -DDEBUG")
set(CMAKE_ASM_FLAGS_RELEASE "")

set(CMAKE_CXX_FLAGS "-std=c++11 -mthumb -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -Wall -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -gdwarf-2 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections,-cref,-u,Reset_Handler,-Map=frameworks.map -T frameworks.ld --specs=nosys.specs")# -static -nostartfiles

# doesn't support shared libs
set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

# try static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_CXX_LINK_SHARED_LIBRARY )
set(CMAKE_CXX_LINK_MODULE_LIBRARY )
set(CMAKE_C_LINK_SHARED_LIBRARY )
set(CMAKE_C_LINK_MODULE_LIBRARY )

# vim:set et ts=4 sts=4 sw=4:
