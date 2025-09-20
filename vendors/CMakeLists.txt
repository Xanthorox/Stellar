include(ExternalProject)

# GoogleTest
ExternalProject_Add(googletest PREFIX googletest
        URL ${CMAKE_CURRENT_SOURCE_DIR}/googletest-release-1.10.0.tgz
        URL_MD5 ad777f4af22026493ea2782c611b8809
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=RELWITHDEBINFO
        BUILD_IN_SOURCE 1
        BUILD_COMMAND make
        ${DOWNLOAD_TIMESTAMP_ARG})

ExternalProject_Get_Property(googletest INSTALL_DIR)
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)
add_library(gtest STATIC IMPORTED GLOBAL)
add_dependencies(gtest googletest)
set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib64/libgtest.a)
set_property(TARGET gtest PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include)
set_property(TARGET gtest PROPERTY INTERFACE_LINK_LIBRARIES pthread)

#libcjson-1.7.17
ExternalProject_Add(cjson PREFIX cjson
                        URL ${CMAKE_CURRENT_SOURCE_DIR}/libcjson_v1.7.17.tar.gz
                        URL_MD5 4b2ab12cf065c079004aa44495ade04a
                        CONFIGURE_COMMAND ""
                        BUILD_COMMAND make
                        INSTALL_COMMAND make install
                        BUILD_IN_SOURCE 1
                        ${DOWNLOAD_TIMESTAMP_ARG} )

ExternalProject_Get_Property(cjson INSTALL_DIR)
message(STATUS "cjson will be installed in: ${INSTALL_DIR}")
file(MAKE_DIRECTORY ${INSTALL_DIR}/src/cjson/include/)

add_library(cjson-static STATIC IMPORTED GLOBAL)
add_dependencies(cjson-static cjson)
set_property(TARGET cjson-static PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/src/cjson/libcjson.a)
set_property(TARGET cjson-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/src/cjson/include/)

#llhttp-9.1.3
ExternalProject_Add(llhttp PREFIX llhttp
                    URL ${CMAKE_CURRENT_SOURCE_DIR}/llhttp-release-v9.2.1.tar.gz
                    URL_MD5 355ecc90e622035e3e1693a96a0c233b
		    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_STATIC_LIBS=ON -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_C_FLAGS="-fPIC"
                    ${DOWNLOAD_TIMESTAMP_ARG})
ExternalProject_Get_Property(llhttp INSTALL_DIR)
message(STATUS "llhttp will be installed in: ${INSTALL_DIR}")
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

add_library(llhttp-static STATIC IMPORTED GLOBAL)
add_dependencies(llhttp-static llhttp)

set_property(TARGET llhttp-static  PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include)
set_property(TARGET llhttp-static  PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib64/libllhttp.a)

#brotli-v1.0.9
ExternalProject_Add(brotli PREFIX brotli
                    URL ${CMAKE_CURRENT_SOURCE_DIR}/brotli-v1.0.9.tar.gz
                    URL_MD5 c2274f0c7af8470ad514637c35bcee7d
		    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                    ${DOWNLOAD_TIMESTAMP_ARG})
ExternalProject_Get_Property(brotli INSTALL_DIR)
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

add_library(brotli-common-static STATIC IMPORTED GLOBAL)
add_dependencies(brotli-common-static brotli)
set_property(TARGET brotli-common-static  PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include)
set_property(TARGET brotli-common-static  PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib64/libbrotlicommon-static.a)

add_library(brotli-dec-static STATIC IMPORTED GLOBAL)
add_dependencies(brotli-dec-static brotli)
set_property(TARGET brotli-dec-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include)
set_property(TARGET brotli-dec-static PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib64/libbrotlidec-static.a)

add_library(brotli-enc-static STATIC IMPORTED GLOBAL)
add_dependencies(brotli-enc-static brotli)
set_property(TARGET brotli-enc-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include)
set_property(TARGET brotli-enc-static PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib64/libbrotlienc-static.a)

# Libevent
ExternalProject_Add(libevent
        PREFIX libevent
        URL ${CMAKE_CURRENT_SOURCE_DIR}/libevent-2.1.12-stable.tar.gz
        URL_MD5 b5333f021f880fe76490d8a799cd79f4
        CMAKE_ARGS -DEVENT__DISABLE_OPENSSL=ON -DEVENT__DISABLE_BENCHMARK=ON  -DBUILD_TESTING=OFF -DEVENT__DISABLE_TESTS=ON -DEVENT__LIBRARY_TYPE=BOTH -DCMAKE_C_FLAGS="-fPIC"
                -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                ${DOWNLOAD_TIMESTAMP_ARG} )

ExternalProject_Get_Property(libevent INSTALL_DIR)
file(MAKE_DIRECTORY ${INSTALL_DIR}/include)

add_library(libevent-static STATIC IMPORTED GLOBAL)
add_dependencies(libevent-static libevent)
set_property(TARGET libevent-static PROPERTY IMPORTED_LOCATION ${INSTALL_DIR}/lib/libevent.a)
set_property(TARGET libevent-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${INSTALL_DIR}/include)
set_property(TARGET libevent-static PROPERTY INTERFACE_LINK_LIBRARIES pthread)
