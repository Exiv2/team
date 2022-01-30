cmake_minimum_required(VERSION 3.8)
project(book VERSION 0.0.1 LANGUAGES CXX)
include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD                  11 )
set(CMAKE_CXX_EXTENSIONS                ON )

# don't build for 32bit (size_t/sprintf unhappiness)
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    message(FATAL_ERROR "32 bit build is not supported")  
    error()
endif()

# the programs
add_executable(visitor     visitor.cpp    )
add_executable(tvisitor    tvisitor.cpp   )
add_executable(tcp++       tcp++.cpp      )
add_executable(dmpf        dmpf.cpp       )
add_executable(csv         csv.cpp        )
add_executable(parse       parse.cpp      )

# options
option(    EXIV2_TEAM_USE_SANITIZERS  "Enable ASAN when available"                  OFF )

if ( ZLIB_FOUND ) 
    target_link_libraries ( tvisitor ${ZLIB_LIBRARIES} )
    target_compile_options( tvisitor PUBLIC -DHAVE_LIBZ)
endif()

if(WIN32)
    find_library(WSOCK32_LIBRARY   wsock32)
    find_library(WS2_32_LIBRARY    ws2_32)
    target_link_libraries(parse    wsock32 ws2_32)
    target_link_libraries(tvisitor wsock32 ws2_32)
endif()

# ASAN (not on Windows)
if ( EXIV2_TEAM_USE_SANITIZERS AND NOT (CYGWIN OR MINGW OR MSYS OR MSVC) ) 
    check_cxx_compiler_flag(                                  -fno-omit-frame-pointer       HAS_NO_EMIT)
    if(HAS_NO_EMIT)
        add_compile_options(                                  -fno-omit-frame-pointer )
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-omit-frame-pointer")
    endif()
    check_cxx_compiler_flag(                                  -fsanitize=address,undefined  HAS_FSAU)
    if(HAS_FSAU)
        add_compile_options(                                  -fsanitize=address,undefined )
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
    endif()
    check_cxx_compiler_flag(                                  -fno-sanitize-recover=all     HAS_FSRA)
    if(HAS_FSRA)
        add_compile_options(                                  -fno-sanitize-recover=all )
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-sanitize-recover=all")
    endif()
endif()

# Test harness (in ../test)
add_custom_target(test  COMMAND ../test/run.sh )
add_custom_target(tests COMMAND ../test/run.sh )

# This is intentionally commented off
# See Chapter 8 Testing for discussion about building libtiff
if ( 0 ) 
	include_directories(/usr/local/include)
	link_directories(/usr/local/lib)
	add_executable(create_tiff create_tiff.cpp)
	target_link_libraries(create_tiff z tiff jpeg)
endif()

# That's all Folks!
##
