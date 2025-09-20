#like redis-cli, use hyphen('-') not underscore('_') 
add_executable(stellar-cli monitor_cli.c)
target_link_libraries(stellar-cli libevent-static cjson-static linenoise sds ringbuf monitor)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/stellar-cli DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/ COMPONENT EXECUTABLE)

# add_subdirectory(stellar-dump)

#tcpdump patch
set (STELLAR-DUMP-PATCH-FILE ${CMAKE_SOURCE_DIR}/tools/monitor/stellar-dump.patch)
set (STELLAR-DUMP-PATCH-CMD ${CMAKE_SOURCE_DIR}/tools/monitor/stellar-dump-update.sh)
ExternalProject_Add(tcpdump PREFIX tcpdump
                    URL ${CMAKE_SOURCE_DIR}/vendors/tcpdump-4.99.4.tar.gz 
                    URL_MD5 4f2d4a8a5fab017e5ebda156bfc86378 
                    PATCH_COMMAND sh -c "chmod +x ${STELLAR-DUMP-PATCH-CMD} && ${STELLAR-DUMP-PATCH-CMD}  ${CMAKE_CURRENT_BINARY_DIR}/tcpdump/src/tcpdump  ${STELLAR-DUMP-PATCH-FILE}"
		            CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DWITH_CRYPTO=OFF -DSTELLAR_SOURCE_DIR=${CMAKE_SOURCE_DIR} -DSTELLAR_BINARY_DIR=${CMAKE_BINARY_DIR} -DASAN_OPTION=${ASAN_OPTION}
                    ${DOWNLOAD_TIMESTAMP_ARG}
                    )
add_dependencies(tcpdump libevent)
ExternalProject_Get_Property(tcpdump INSTALL_DIR)
install(PROGRAMS ${INSTALL_DIR}/src/tcpdump-build/stellar-dump DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/ COMPONENT EXECUTABLE)
