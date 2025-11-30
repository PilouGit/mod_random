cmake_minimum_required(VERSION 3.10)
project(mod_random)

set(CMAKE_C_STANDARD 99)

# Find Apache development headers
find_path(APACHE_INCLUDE_DIR
    NAMES httpd.h
    PATHS
    /usr/include/apache2
    /usr/include/httpd
    /usr/local/include/apache2
    /usr/local/include/httpd
    /opt/apache2/include
)

find_path(APR_INCLUDE_DIR
    NAMES apr.h
    PATHS
    /usr/include/apr-1.0
    /usr/include/apr-1
    /usr/local/include/apr-1
    /opt/apache2/include
)

if(NOT APACHE_INCLUDE_DIR)
    message(FATAL_ERROR "Apache development headers not found. Please install apache2-dev or httpd-devel package.")
endif()

if(NOT APR_INCLUDE_DIR)
    message(FATAL_ERROR "APR development headers not found. Please install libapr1-dev package.")
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)

if(NOT OPENSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found. Please install libssl-dev package.")
endif()

# Get Apache module directory
execute_process(
    COMMAND apxs -q LIBEXECDIR
    OUTPUT_VARIABLE APACHE_MODULE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT APACHE_MODULE_DIR)
    set(APACHE_MODULE_DIR "/usr/lib/apache2/modules")
    message(WARNING "Could not determine Apache module directory, using default: ${APACHE_MODULE_DIR}")
endif()

# Include directories
include_directories(${APACHE_INCLUDE_DIR})
include_directories(${APR_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})

# Create shared library
add_library(mod_random SHARED
    src/mod_random.c
    src/mod_random_config.c
    src/mod_random_encode.c
    src/mod_random_crypto.c
    src/mod_random_token.c
)

# Set module properties
set_target_properties(mod_random PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# Compiler flags for position independent code
target_compile_options(mod_random PRIVATE -fPIC)

# Link with OpenSSL
target_link_libraries(mod_random PRIVATE OpenSSL::Crypto)

# Installation
install(TARGETS mod_random
    LIBRARY DESTINATION ${APACHE_MODULE_DIR}
)

# Create .load file for easier Apache configuration
configure_file(
    "${CMAKE_SOURCE_DIR}/mod_random.load.in"
    "${CMAKE_BINARY_DIR}/mod_random.load"
    @ONLY
)

install(FILES "${CMAKE_BINARY_DIR}/mod_random.load"
    DESTINATION "/etc/apache2/mods-available"
    OPTIONAL
)

# Configure integration test Apache configuration
set(TEST_INTEGRATION_DIR "${CMAKE_SOURCE_DIR}/tests/integration")
set(MOD_RANDOM_PATH "${CMAKE_BINARY_DIR}/mod_random.so")

configure_file(
    "${CMAKE_SOURCE_DIR}/tests/integration/conf/httpd.conf.in"
    "${CMAKE_SOURCE_DIR}/tests/integration/conf/httpd.conf"
    @ONLY
)

# Print build information
message(STATUS "Apache include directory: ${APACHE_INCLUDE_DIR}")
message(STATUS "APR include directory: ${APR_INCLUDE_DIR}")
message(STATUS "OpenSSL include directory: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
message(STATUS "Apache module directory: ${APACHE_MODULE_DIR}")
message(STATUS "Integration test directory: ${TEST_INTEGRATION_DIR}")
message(STATUS "Module path: ${MOD_RANDOM_PATH}")