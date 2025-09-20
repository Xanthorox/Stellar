cmake_minimum_required(VERSION 3.5)
project(stellar)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(Version)
include(Package)

add_definitions(-D_GNU_SOURCE)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# set warning as error
add_compile_options(-Wall -Wextra -Werror)


if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24")
    set(DOWNLOAD_TIMESTAMP_ARG DOWNLOAD_EXTRACT_TIMESTAMP ON)
else()
    set(DOWNLOAD_TIMESTAMP_ARG)
endif()

message(STATUS "Download timestamp arg: ${DOWNLOAD_TIMESTAMP_ARG}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type (default: RelWithDebInfo)" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set (CMAKE_INSTALL_PREFIX "/opt/tsg/stellar" CACHE PATH "default install path" FORCE)
endif()

find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
if (CMAKE_CXX_CPPCHECK)
    list(
        APPEND CMAKE_CXX_CPPCHECK 
            "--enable=all"
			"--std=c++11"
            "--error-exitcode=1"
            "--suppress=unusedFunction"
            "--suppress=missingInclude"
            "--suppress=uselessAssignmentPtrArg"
            "--suppress=unmatchedSuppression"
            "--suppress=variableScope"
            "--suppress=unreadVariable"
            "--suppress=cstyleCast"
            "--suppress=memleakOnRealloc"
            "--suppress=constParameter"
            "--suppress=uselessAssignmentArg"
            "--suppress=uninitvar"
            "--suppress=unusedStructMember"
            "--suppress=funcArgOrderDifferent"
            "--suppress=unreachableCode"
            "--suppress=internalAstError"
            "--suppress=integerOverflow"
            "--suppress=redundantInitialization"
            "--suppress=*:${CMAKE_SOURCE_DIR}/infra/monitor/stellar-dump/*"
            "--suppress=*:${CMAKE_SOURCE_DIR}/decoders/lpi_plus/libprotoident/*"
            "--suppress=*:${CMAKE_SOURCE_DIR}/deps/*"
    )
    set(CMAKE_C_CPPCHECK ${CMAKE_CXX_CPPCHECK})
else()
    message(FATAL_ERROR "Could not find the program cppcheck.")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE 1)

# for ASAN
set(ASAN_OPTION "OFF" CACHE STRING " set asan type chosen by the user, using OFF as default")
set_property(CACHE ASAN_OPTION  PROPERTY STRINGS OFF ADDRESS THREAD)
message(STATUS "ASAN_OPTION='${ASAN_OPTION}'")

if(ASAN_OPTION MATCHES "ADDRESS")
    set(CMAKE_C_FLAGS "${CMAKADDRESS} -g -DCMAKE_BUILD_TYPE=Debug -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DCMAKE_BUILD_TYPE=Debug -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lasan")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lasan")
elseif(ASAN_OPTION MATCHES "THREAD")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DCMAKE_BUILD_TYPE=Debug -fsanitize=thread -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DCMAKE_BUILD_TYPE=Debug -fsanitize=thread -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lasan")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lasan")
endif()
# end of for ASAN


#common include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/deps)

enable_testing()
add_subdirectory(conf)
add_subdirectory(vendors)
add_subdirectory(deps)
add_subdirectory(infra)
add_subdirectory(decoders)
add_subdirectory(scripts)
add_subdirectory(include)
add_subdirectory(tools)
add_subdirectory(test)

install(DIRECTORY DESTINATION log COMPONENT PROGRAM)
install(DIRECTORY DESTINATION metrics COMPONENT PROGRAM)
install(DIRECTORY DESTINATION module COMPONENT PROGRAM)