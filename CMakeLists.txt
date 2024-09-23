cmake_minimum_required(VERSION 3.7)
project(AromaSFDL CXX)

set(CMAKE_TOOLCHAIN_FILE $ENV{DEVKITPRO}/wut/share/wut.toolchain.cmake)
find_package(CURL REQUIRED)

add_executable(aromaSFDL src/main.cpp)

# Add library directories
target_link_directories(aromaSFDL PRIVATE $ENV{DEVKITPRO}/wut/usr/lib)

# Find the Mocha library
find_library(MOCHA_LIBRARY mocha HINTS $ENV{DEVKITPRO}/wut/usr/lib)

if(NOT MOCHA_LIBRARY)
    message(FATAL_ERROR "Mocha library not found.")
endif()

# Link libraries
target_link_libraries(aromaSFDL
    wut
    ${MOCHA_LIBRARY}
    ${CURL_LIBRARIES}
    mbedtls
    mbedx509
    mbedcrypto
    z
)

# Include directories
target_include_directories(aromaSFDL PRIVATE
    ${CURL_INCLUDE_DIRS}
    $ENV{DEVKITPRO}/wut/usr/include
)

wut_create_rpx(aromaSFDL)
