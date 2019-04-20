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

SOURCES += \
        main.cpp \
    ../RasterJoin/triangulation/clip2tri/clip2tri.cpp \
    ../RasterJoin/triangulation/clipper/clipper.cpp \
    ../RasterJoin/triangulation/poly2tri/common/shapes.cc \
    ../RasterJoin/triangulation/poly2tri/sweep/advancing_front.cc \
    ../RasterJoin/triangulation/poly2tri/sweep/cdt.cc \
    ../RasterJoin/triangulation/poly2tri/sweep/sweep.cc \
    ../RasterJoin/triangulation/poly2tri/sweep/sweep_context.cc \
    GridIndexF.cpp \
    CPUJoin.cpp \
    ../RasterJoin/common/UsefulFuncs.cpp \
    ../RasterJoin/db/PolyHandler.cpp \
    ../RasterJoin/db/Record.cpp \
    ../RasterJoin/db/PartitioningManager.cpp \
    ../RasterJoin/db/HashGridIndex.cpp \
    ../RasterJoin/db/Dataset.cpp \
    ../RasterJoin/db/DataHandler.cpp \
    ../RasterJoin/db/BufferedPartitionedFile.cpp

HEADERS += \
    ../RasterJoin/triangulation/clip2tri/clip2tri.h \
    ../RasterJoin/triangulation/clipper/clipper.hpp \
    ../RasterJoin/triangulation/poly2tri/common/shapes.h \
    ../RasterJoin/triangulation/poly2tri/common/utils.h \
    ../RasterJoin/triangulation/poly2tri/sweep/advancing_front.h \
    ../RasterJoin/triangulation/poly2tri/sweep/cdt.h \
    ../RasterJoin/triangulation/poly2tri/sweep/sweep.h \
    ../RasterJoin/triangulation/poly2tri/sweep/sweep_context.h \
    ../RasterJoin/triangulation/poly2tri/poly2tri.h \
    GridIndexF.hpp \
    CPUJoin.hpp \
    ../RasterJoin/common/Utils.h \
    ../RasterJoin/common/UsefulFuncs.hpp \
    ../RasterJoin/common/Common.h \
    ../RasterJoin/db/PolyHandler.hpp \
    ../RasterJoin/db/TwitterRecord.hpp \
    ../RasterJoin/db/TaxiRecord.hpp \
    ../RasterJoin/db/Record.hpp \
    ../RasterJoin/db/QueryResult.hpp \
    ../RasterJoin/db/qrbtree_p.h \
    ../RasterJoin/db/qdatabuffer_p.h \
    ../RasterJoin/db/PartitioningManager.hpp \
    ../RasterJoin/db/MappedQueryResult.hpp \
    ../RasterJoin/db/HashGridIndex.hpp \
    ../RasterJoin/db/Dataset.hpp \
    ../RasterJoin/db/DataHandler.hpp \
    ../RasterJoin/db/BufferedPartitionedFile.hpp \
    ../RasterJoin/db/BufferedFile.hpp


INCLUDEPATH += ../RasterJoin/common/
INCLUDEPATH += ../RasterJoin/db/
INCLUDEPATH += ../RasterJoin/triangulation/

CONFIG(debug, debug|release) {
} else {
    DEFINES += "QT_NO_DEBUG_OUTPUT"
}

unix:!macx{
    CONFIG(debug, debug|release) {
    } else {
        QMAKE_CXXFLAGS += -O3
    }
    DEFINES += "LINUX"

    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    INCLUDEPATH += -I /usr/local/include/
    INCLUDEPATH += -I /usr/include/boost/

    LIBS += -L/usr/local/lib/ -lGLEW -lboost_iostreams -lboost_program_options #-lEGL
}

# Win32 with msys64 toolchain
win32-g++{
    CONFIG(debug, debug|release) {
    } else {
        QMAKE_CXXFLAGS += -O3
    }
    CONFIG += console
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    #INCLUDE
    INCLUDEPATH += "$$(CUDA_PATH)/include"
    INCLUDEPATH += C:/local/msys64/mingw64/include/

    #LIBS
    LIBS += -LC:/local/msys64/mingw64/lib -lboost_iostreams-mt -lboost_program_options-mt
    LIBS += "-ladvapi32"
}

win32-msvc*{
    CONFIG += console

    INCLUDEPATH += $$(VCPKG_HOME)/installed/x64-windows/include

    #http://stackoverflow.com/questions/5134245/how-to-set-different-qmake-configuration-depending-on-debug-release
    CONFIG(debug, debug|release) {
        WINDOWS_BIN_PATH = debug/
        LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/lib" -lboost_filesystem-vc140-mt-gd -lboost_program_options-vc140-mt-gd -lboost_iostreams-vc140-mt-gd
    } else {
        WINDOWS_BIN_PATH = ./
        LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/lib" -lboost_filesystem-vc140-mt -lboost_program_options-vc140-mt -lboost_iostreams-vc140-mt
    }

    QMAKE_CXXFLAGS += -openmp

    LIBS += "-L$$(VCPKG_HOME)/installed/x64-windows/$${WINDOWS_BIN_PATH}/bin"
    LIBS += "-ladvapi32"
    LIBS += -lopengl32 -lRpcRT4
}
