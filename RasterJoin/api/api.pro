TARGET = Rasterjoin
TEMPLATE = lib
QT += opengl
CONFIG += c++11

DEFINES += RASTERJOIN_LIB

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    interface/SpatialAggregation.cpp \
    triangulation/clip2tri/clip2tri.cpp \
    triangulation/clipper/clipper.cpp \
    triangulation/poly2tri/common/shapes.cc \
    triangulation/poly2tri/sweep/advancing_front.cc \
    triangulation/poly2tri/sweep/cdt.cc \
    triangulation/poly2tri/sweep/sweep.cc \
    triangulation/poly2tri/sweep/sweep_context.cc \
    common/UsefulFuncs.cpp \
    db/DataHandler.cpp \
    db/PolyHandler.cpp \
    opengl/OpenGLFrameBufferObject.cpp \
    opengl/OpenGLShaderProgram.cpp \
    join/GLData.cpp \
    join/GLFunction.cpp \
    join/GLHandler.cpp \
    join/RasterJoin.cpp \
    join/IndexJoin.cpp \
    join/HybridJoin.cpp \
    join/RasterJoinBounds.cpp

HEADERS += \
    interface/NPArray.hpp \
    interface/SpatialAggregation.hpp \
    triangulation/clip2tri/clip2tri.h \
    triangulation/clipper/clipper.hpp \
    triangulation/poly2tri/common/shapes.h \
    triangulation/poly2tri/common/utils.h \
    triangulation/poly2tri/sweep/advancing_front.h \
    triangulation/poly2tri/sweep/cdt.h \
    triangulation/poly2tri/sweep/sweep.h \
    triangulation/poly2tri/sweep/sweep_context.h \
    triangulation/poly2tri/poly2tri.h \
    triangulation/Triangle.hpp \
    triangulation/TestTriangulation.hpp \
    common/Common.h \
    common/UsefulFuncs.hpp \
    common/Utils.h \
    db/DataHandler.hpp \
    db/PolyHandler.hpp \
    opengl/OpenGLFrameBufferObject.hpp \
    opengl/OpenGLShaderProgram.hpp \
    join/GLData.hpp \
    join/GLFunction.hpp \
    join/GLHandler.hpp \
    join/ShaderBuffer.hpp \
    join/RasterJoin.hpp \
    join/IndexJoin.hpp \
    join/HybridJoin.hpp \
    join/RasterJoinBounds.hpp

INCLUDEPATH += -I ./triangulation/

DEFINES     += "TRILIBRARY"
DEFINES     += "ANSI_DECLARATORS"
DEFINES     += "NO_TIMER"

CONFIG(debug, debug|release) {
} else {
#    DEFINES += "QT_NO_DEBUG_OUTPUT"
}

#DEFINES += "FULL_SUMMARY_GL"

# Unix configuration
unix:!macx{
    CONFIG(debug, debug|release) {
    } else {
        QMAKE_CXXFLAGS += -O3
    }
    DEFINES += "LINUX"
#    DEFINES += "USE_EGL"

    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    INCLUDEPATH += -I /usr/local/include/
    INCLUDEPATH += -I /usr/local/cuda/include/

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

    #LIBS
    LIBS += -LC:/local/msys64/mingw64/lib -lglew32
    LIBS += "-ladvapi32"
}

win32-msvc*{
    CONFIG += console

    INCLUDEPATH += $$(VCPKG_HOME)/installed/x64-windows/include

    #http://stackoverflow.com/questions/5134245/how-to-set-different-qmake-configuration-depending-on-debug-release
    CONFIG(debug, debug|release) {
        WINDOWS_BIN_PATH = debug/
        LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/lib" -lglew32d
    } else {
        WINDOWS_BIN_PATH = ./
        LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/lib" -lglew32
    }

    QMAKE_CXXFLAGS += -openmp

    LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/bin"
    LIBS += "-ladvapi32"
    LIBS += -lopengl32 -lRpcRT4
}

RESOURCES += \
    shaders.qrc
