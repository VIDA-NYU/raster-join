TARGET = Rasterjoin
TEMPLATE = app
QT += opengl
CONFIG += c++11


SOURCES += \
    main.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += -I ../api/

CONFIG(debug, debug|release) {
} else {
#    DEFINES += "QT_NO_DEBUG_OUTPUT"
}

# Unix configuration
unix:!macx{
    CONFIG(debug, debug|release) {
    } else {
        QMAKE_CXXFLAGS += -O3
    }
    DEFINES += "LINUX"
#    DEFINES += "USE_EGL"
#    DEFINES += "FULL_SUMMARY_GL"

    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    INCLUDEPATH += -I /usr/local/include/
    INCLUDEPATH += -I /usr/local/cuda/include/

    LIBS += -L../api -lRasterjoin
    LIBS += -L/usr/local/lib/ -lGLEW #-lEGL
}

# Win32 with msys64 toolchain
win32-g++{
    CONFIG(debug, debug|release) {
    } else {
        QMAKE_CXXFLAGS += -O3
    }
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    #INCLUDE
    INCLUDEPATH += "$$(CUDA_PATH)/include"
    INCLUDEPATH += C:/local/msys64/mingw64/include/

    CONFIG(debug, debug|release) {
        LIBS += "-L../api/debug/" -lRasterjoin
    } else {
        LIBS += "-L../api/release/" -lRasterjoin
    }
    #LIBS
    LIBS += -LC:/local/msys64/mingw64/lib -lglew32
    LIBS += "-ladvapi32"
}

win32-msvc*{
    CONFIG += console

    INCLUDEPATH += $$(VCPKG_HOME)/installed/x64-windows-static/include

    #http://stackoverflow.com/questions/5134245/how-to-set-different-qmake-configuration-depending-on-debug-release
    CONFIG(debug, debug|release) {
        WINDOWS_BIN_PATH = debug/
        LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows-static/$${WINDOWS_BIN_PATH}/lib" -lglew32d
        LIBS += "-L../api/debug/" -lRasterjoin
    } else {
        WINDOWS_BIN_PATH = ./
        LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows-static/$${WINDOWS_BIN_PATH}/lib" -lglew32
        LIBS += "-L../api/release/" -lRasterjoin
    }

    QMAKE_CXXFLAGS += -openmp

    LIBS += "-ladvapi32"
    LIBS += -lopengl32 -lRpcRT4

    DEFINES += STATIC_BUILD
    CONFIG+=static
}

