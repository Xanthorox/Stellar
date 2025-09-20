# Using autorevision.sh to generate version information
set(__SOURCE_AUTORESIVISION ${CMAKE_SOURCE_DIR}/autorevision.sh)
set(__AUTORESIVISION ${CMAKE_BINARY_DIR}/autorevision.sh)
set(__VERSION_CACHE ${CMAKE_BINARY_DIR}/version.txt)
set(__VERSION_CONFIG ${CMAKE_BINARY_DIR}/version.cmake)

file(COPY ${__SOURCE_AUTORESIVISION} DESTINATION ${CMAKE_BINARY_DIR} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# Execute autorevision.sh to generate version information
execute_process(COMMAND ${__AUTORESIVISION} -t cmake -o ${__VERSION_CACHE} OUTPUT_FILE ${__VERSION_CONFIG})
include(${__VERSION_CONFIG})

# Extract major, minor, patch version from git tag
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VCS_TAG}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VCS_TAG}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VCS_TAG}")
string(REGEX REPLACE "[T\\:\\+\\-]" "" VERSION_DATE "${VCS_DATE}")

if(NOT VERSION_MAJOR)
	set(VERSION_MAJOR 1)
endif()

if(NOT VERSION_MINOR)
	set(VERSION_MINOR 0)
endif()

if(NOT VERSION_PATCH)
	set(VERSION_PATCH 0)
endif()

if(VERSION_DAILY_BUILD)
	set(VERSION_PATCH ${VERSION_PATCH}.${VERSION_DATE})
endif()

set(DESCRIBE "${VCS_SHORT_HASH}")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(STELLAR_GIT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${DESCRIBE}")

# Replace .- with _
string(REGEX REPLACE "[\\.\\-]" "_" VAR_VERSION "${STELLAR_GIT_VERSION}")

# Print information
message(STATUS "Welcome to stellar, Version: ${STELLAR_GIT_VERSION}")
add_definitions(-DSTELLAR_GIT_VERSION=\"${STELLAR_GIT_VERSION}\")
add_definitions(-DVAR_VERSION=${VAR_VERSION})