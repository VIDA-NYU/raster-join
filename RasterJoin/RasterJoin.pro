TEMPLATE = app
QT += opengl
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    triangulation/clip2tri/clip2tri.cpp \
    triangulation/clipper/clipper.cpp \
    triangulation/poly2tri/common/shapes.cc \
    triangulation/poly2tri/sweep/advancing_front.cc \
    triangulation/poly2tri/sweep/cdt.cc \
    triangulation/poly2tri/sweep/sweep.cc \
    triangulation/poly2tri/sweep/sweep_context.cc \
    common/UsefulFuncs.cpp \
    db/BufferedPartitionedFile.cpp \
    db/Dataset.cpp \
    db/HashGridIndex.cpp \
    db/PartitioningManager.cpp \
    db/Record.cpp \
    join/GLData.cpp \
    join/GLFunction.cpp \
    join/GLHandler.cpp \
    join/RasterJoin.cpp \
    opengl/OpenGLFrameBufferObject.cpp \
    opengl/OpenGLShaderProgram.cpp \
    db/DataHandler.cpp \
    db/PolyHandler.cpp \
    join/IndexJoin.cpp \
    join/HybridJoin.cpp

HEADERS += \
    triangulation/clip2tri/clip2tri.h \
    triangulation/clipper/clipper.hpp \
    triangulation/poly2tri/common/shapes.h \
    triangulation/poly2tri/common/utils.h \
    triangulation/poly2tri/sweep/advancing_front.h \
    triangulation/poly2tri/sweep/cdt.h \
    triangulation/poly2tri/sweep/sweep.h \
    triangulation/poly2tri/sweep/sweep_context.h \
    triangulation/poly2tri/poly2tri.h \
    common/Common.h \
    common/TypeFunctions.hpp \
    common/UsefulFuncs.hpp \
    common/Utils.h \
    db/BufferedFile.hpp \
    db/BufferedPartitionedFile.hpp \
    db/Dataset.hpp \
    db/HashGridIndex.hpp \
    db/PartitioningManager.hpp \
    db/qdatabuffer_p.h \
    db/qrbtree_p.h \
    db/QueryResult.hpp \
    db/Record.hpp \
    db/TaxiRecord.hpp \
    db/TwitterRecord.hpp \
    join/GLData.hpp \
    join/GLFunction.hpp \
    join/GLHandler.hpp \
    join/ShaderBuffer.hpp \
    join/RasterJoin.hpp \
    opengl/OpenGLFrameBufferObject.hpp \
    opengl/OpenGLShaderProgram.hpp \
    triangulation/Triangle.hpp \
    db/DataHandler.hpp \
    db/PolyHandler.hpp \
    triangulation/TestTriangulation.hpp \
    join/IndexJoin.hpp \
    join/HybridJoin.hpp \
    test/baseline.hpp

INCLUDEPATH += -I ./triangulation/
INCLUDEPATH += -I ./opengl/
INCLUDEPATH += -I ./join/
INCLUDEPATH += -I ./db/
INCLUDEPATH += -I ./common/
INCLUDEPATH += -I ./test/


DEFINES     += "TRILIBRARY"
DEFINES     += "ANSI_DECLARATORS"
DEFINES     += "NO_TIMER"

CONFIG(debug, debug|release) {
} else {
    DEFINES += "QT_NO_DEBUG_OUTPUT"
    QMAKE_CXXFLAGS += -O3
}

# Unix configuration
unix:!macx{
    DEFINES += "LINUX"
#    DEFINES += "USE_EGL"
#    DEFINES += "FULL_SUMMARY_GL"

    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    INCLUDEPATH += -I /usr/local/include/
    INCLUDEPATH += -I /usr/include/boost/
    INCLUDEPATH += -I /usr/local/cuda/include/

    LIBS += -L/usr/local/lib/ -lGLEW -lboost_iostreams -lboost_program_options #-lEGL
}

# Win32 with msys64 toolchain
win32-g++{
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    #INCLUDE
    INCLUDEPATH += "$$(CUDA_PATH)/include"
    INCLUDEPATH += C:/local/msys64/mingw64/include/

    #LIBS
    LIBS += -LC:/local/msys64/mingw64/lib -lglew32 -lboost_iostreams-mt -lboost_program_options-mt
    LIBS += "-ladvapi32"
}

win32-msvc*{
    CONFIG += console

    INCLUDEPATH += $$(VCPKG_HOME)/installed/x64-windows/include

    #http://stackoverflow.com/questions/5134245/how-to-set-different-qmake-configuration-depending-on-debug-release
    CONFIG(debug, debug|release) {
        WINDOWS_BIN_PATH = debug/
        LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/lib" -lglew32d -lboost_filesystem-vc140-mt-gd -lboost_program_options-vc140-mt-gd -lboost_iostreams-vc140-mt-gd
    } else {
        WINDOWS_BIN_PATH = ./
        LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/lib" -lglew32 -lboost_filesystem-vc140-mt -lboost_program_options-vc140-mt -lboost_iostreams-vc140-mt
    }

    #http://stackoverflow.com/questions/5134245/how-to-set-different-qmake-configuration-depending-on-debug-release
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/bin"
    LIBS += "-ladvapi32"
    LIBS += -lopengl32 -lRpcRT4
}

RESOURCES += \
    shaders.qrc
