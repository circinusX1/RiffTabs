cmake_minimum_required(VERSION 3.19)
project(nnls_console LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(nnls_console
    main.cpp
)

target_include_directories(nnls_console PRIVATE
    /usr/local/include
    /usr/include/eigen3
    /usr/include/soundtouch
    /usr/include/vamp-hostsdk
)

find_library(SOUNDTOUCH_LIB NAMES SoundTouch soundtouch HINTS /usr/local/lib /usr/lib)
find_library(VAMP_HOSTSDK_LIB NAMES vamp-hostsdk HINTS /usr/local/lib /usr/lib)
find_library(FFTW3_LIB NAMES fftw3 fftw3f HINTS /usr/local/lib /usr/lib)

if(NOT SOUNDTOUCH_LIB)
    set(SOUNDTOUCH_LIB SoundTouch)
endif()

if(NOT VAMP_HOSTSDK_LIB)
    set(VAMP_HOSTSDK_LIB vamp-hostsdk)
endif()

if(NOT FFTW3_LIB)
    set(FFTW3_LIB fftw3)
endif()

target_link_libraries(nnls_console PRIVATE
    ${SOUNDTOUCH_LIB}
    ${VAMP_HOSTSDK_LIB}
    ${FFTW3_LIB}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(nnls_console PRIVATE dl m)
endif()

include(GNUInstallDirs)

install(TARGETS nnls_console
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
