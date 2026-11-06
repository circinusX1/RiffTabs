QT += core gui widgets multimedia network

# Qt 6 moved QOpenGLWidget into its own module
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
} else {
    QT += opengl
}

VERSION = 1.0.1

# Pass it to your C++ code as a macro
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG += c++17

TARGET = RiffTabs
TEMPLATE = app


win32 {
    INCLUDEPATH += C:/vcpkg/installed/x64-windows-static/include
    INCLUDEPATH += $$PWD/../vamp-plugin-sdk
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    fretboardwidget.cpp \
    glwaveformwidget.cpp \
    tabwidget.cpp

HEADERS += \
    glwaveformwidget.h \
    mainwindow.h \
    fretboardwidget.h \
    knobwidget.h \
    tabwidget.h

#
# sudo apt update
# sudo apt install -y build-essential cmake pkg-config git \
#                     libfftw3-dev libyaml-cpp-dev libtag1-dev \
#                     libsamplerate0-dev libavcodec-dev libavformat-dev \
#                     python3 python3-pip  libeigen3-dev
# sudo apt update
# sudo apt install -y libsoundtouch-dev
#

unix:!macx {
    # Include paths for Eigen3, SoundTouch, and Vamp Host SDK
    INCLUDEPATH += /usr/local/include \
                   /usr/include/eigen3
    INCLUDEPATH += /usr/include/soundtouch \
                   /usr/include/vamp-hostsdk

    # Removed -lessentia, added -lvamp-hostsdk
    LIBS += -L/usr/local/lib -lSoundTouch -lvamp-hostsdk -lfftw3 -lyaml-cpp
}

win32 {
    VCPKG = C:/vcpkg/installed/x64-windows-static-md
    VAMPLIB = $$PWD/../vamp-plugin-sdk

    INCLUDEPATH += $$VCPKG/include

    CONFIG(debug, debug|release) {
        VCPKG_LIB = $$VCPKG/debug/lib
    } else {
        VCPKG_LIB = $$VCPKG/lib
    }

    LIBS += \
        $$VCPKG_LIB/SoundTouch.lib \
        $$VCPKG_LIB/fftw3f.lib


    CONFIG(debug, debug|release) {
        message("Configuring for Windows DEBUG build")
        LIBS += \
            $$VCPKG_LIB/yaml-cppd.lib \
            $$VAMPLIB/build/Desktop_x86_windows_msvc2026_pe_64bit_Debug/vamp-hostsdk.lib
            $$VAMPLIB/build/Desktop_x86_windows_msvc2026_pe_64bit_Release/vamp-sdk.lib
    }
    else{
        message("Configuring for Windows RELEASE build")
        LIBS += \
            $$VCPKG_LIB/yaml-cpp.lib \
            $$VAMPLIB/build/Desktop_x86_windows_msvc2026_pe_64bit_Release/vamp-hostsdk.lib
            $$VAMPLIB/build/Desktop_x86_windows_msvc2026_pe_64bit_Release/vamp-sdk.lib
    }
    # LIBS += -LC:/vcpkg/installed/x64-windows-static/lib -lvamp-sdk

}

# WINDOWS
# git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
# cd C:\vcpkg
# .\bootstrap-vcpkg.bat
# sudo apt install libvamp-hostsdk1v5 vamp-plugin-sdk
# .\vcpkg install fftw3:x64-windows yaml-cpp:x64-windows essentia:x64-windows
# .\vcpkg install soundtouch:x64-windows


# Copy the binary to ./bin after a successful build
DESTDIR = $$PWD/bin

unix:!macx {
    QMAKE_POST_LINK += mkdir -p $$DESTDIR && cp -f $$OUT_PWD/$$TARGET $$DESTDIR/
}

win32 {
    QMAKE_POST_LINK += $$quote(if not exist $$shell_path($$DESTDIR) mkdir $$shell_path($$DESTDIR) && copy /Y $$shell_path($$OUT_PWD/$$TARGET.exe) $$shell_path($$DESTDIR/))
}

DISTFILES += \
    bin/run.sh

