
#### danmaku2ass ####
add_subdirectory(danmaku2ass_cpp)

#### hlsdl ####
if (UNIX)
    if(APPLE)
        # Fix CMake's search for OpenSSL, see https://stackoverflow.com/a/62063357
        IF(EXISTS /opt/homebrew/opt/)
            set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@1.1)
        ELSE()
            set(OPENSSL_ROOT_DIR /usr/local/opt/openssl@1.1)
        ENDIF()
        set(OPENSSL_CRYPTO_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/../libmpv/libcrypto.a CACHE FILEPATH "" FORCE)
        set(OPENSSL_SSL_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/../libmpv/libssl.a CACHE FILEPATH "" FORCE)
    endif()
    find_package(OpenSSL REQUIRED)
    find_package(CURL REQUIRED)
    find_package(Threads REQUIRED)

    add_executable(hlsdl
        hlsdl/src/main.c
        hlsdl/src/aes_openssl.c
        hlsdl/src/curl.c
        hlsdl/src/hls.c
        hlsdl/src/misc.c
        hlsdl/src/msg.c
        hlsdl/src/mpegts.c)

    target_include_directories(hlsdl PRIVATE ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})
    target_link_libraries(hlsdl PRIVATE ${OPENSSL_CRYPTO_LIBRARY} ${CURL_LIBRARIES} Threads::Threads)
    set_target_properties($hlsdl PROPERTIES LINK_FLAGS_RELEASE -s)

    if (NOT APPLE)
        install(TARGETS hlsdl RUNTIME DESTINATION bin)
    endif()
endif()
