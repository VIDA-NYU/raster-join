TEMPLATE = app
QT += core
QT -= gui
CONFIG += c++11


SOURCES += main.cpp \
    src/HashGridIndex.cpp \
    src/Dataset.cpp \
    src/BufferedPartitionedFile.cpp \
    src/PartitioningManager.cpp \
    src/Record.cpp

HEADERS += \
    include/HashGridIndex.hpp \
    include/Dataset.hpp \
    include/Record.hpp \
    include/TaxiRecord.hpp \
    include/TwitterRecord.hpp \
    include/PartitioningManager.hpp \
    common/Common.h \
    include/QueryResult.hpp \
    include/BufferedFile.hpp \
    include/BufferedPartitionedFile.hpp \
    common/Utils.h

INCLUDEPATH += -I ./include/
INCLUDEPATH += -I ./common/


# Unix configuration
unix:!macx{
    DEFINES += "LINUX"

    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp


    INCLUDEPATH += -I /usr/local/include/
    INCLUDEPATH += -I /usr/include/boost/


    LIBS += -L/usr/local/lib/ -lGLEW -lboost_iostreams
}

win32{
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS   += -fopenmp

    #INCLUDE
    INCLUDEPATH += C:/local/msys64/mingw64/include/

    #LIBS
    LIBS += -LC:/local/msys64/mingw64/lib -lboost_iostreams-mt
    LIBS += "-ladvapi32"
}
