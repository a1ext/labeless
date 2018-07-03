#-------------------------------------------------
#
# Project created by QtCreator 2016-10-05T14:38:35
#
#-------------------------------------------------

QT       += widgets

TARGET = labeless_ida_70
TEMPLATE = lib
CONFIG += plugin c++11
CONFIG -= debug
CONFIG *= release force_debug_info x64

# CONFIG += is64
CONFIG += is_ida7
QT_NAMESPACE = QT

# FIXME: adjust right paths below
win32 {
    error("Don't use .pro file for windows build. Use Visual Studio project instead.")
    SDK_PATH = $$PWD/sdk/
    IDA_PATH = $$PWD/../../IDA695/
    INCLUDEPATH += $$PWD/../3rdparty/protobuf-2.6.1/src
    LIBS += -L$${SDK_PATH}/lib/x86_win_qt
}
else:unix {
    SDK_PATH = $$PWD/../idasdk70/
    IDA_PATH = $$PWD/../idademo70/
    QMAKE_CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
    QMAKE_CFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
}
else:mac {
    error("mac platform doesn't supported right now")
    # TODO: https://github.com/google/protobuf/releases/download/v2.6.0/protobuf-2.6.0.tar.gz
}


# add include path
INCLUDEPATH += $${SDK_PATH}/include
win32 {
    is64 {
        TARGET_EXT = .p64
    }
    else {
        TARGET_EXT = .plw
    }
    DEFINES += __NT__
    SYSNAME = win
    COMPILER_NAME = vc
    LIBS += -L$$PWD/../3rdparty/libs -llibprotobuf_v140 -lws2_32
}
else:!mac:unix {
    is_ida7 {
        TARGET_EXT = .so
        is64 {
            TARGET = $${TARGET}_64
        }
    }
    else {
        is64 {
            TARGET_EXT = .plx64
        }
        else {
            TARGET_EXT = .plx
        }
    }
    DEFINES += __LINUX__ \
               _FORTIFY_SOURCE=0
    SYSNAME = linux
    COMPILER_NAME = gcc
    # avoid linking GLIBC_2.11 symbols (longjmp_chk)
    CFLAGS += -D_FORTIFY_SOURCE=0
}
mac { # scope name must be 'mac'
    TARGET_EXT = .pmc
    DEFINES += __MAC__
    SYSNAME = mac
    COMPILER_NAME = gcc
    CONFIG += macx
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.3
    QMAKE_INFO_PLIST = Info.plist

    # we compile and link a 32-bit application:
    !x64 {
      CFLAGS = -m32
      QMAKE_LFLAGS_DEBUG += -m32
      QMAKE_LFLAGS_RELEASE += -m32
    }
}

DEFINES += __IDP__ \
    NO_OBSOLETE_FUNCS \
    LL_LIBRARY \
    __QT__ \
    __X64__ \
    QT_NO_UNICODE_LITERAL


CONFIG(debug, debug|release) {
  DEFINES += _DEBUG
}

x64 {
    TARGET_PROCESSOR_NAME = x64
    DEFINES += __EA64__
    SUFF64 = 64x
    ADRSIZE = 64
}
else {
    TARGET_PROCESSOR_NAME = x86
    is64 {
        DEFINES += __EA64__
        SUFF64 = 64
        ADRSIZE = 64
    }
    else {
        ADRSIZE = 32
    }
}

SYSDIR = $${TARGET_PROCESSOR_NAME}_$${SYSNAME}_$${COMPILER_NAME}_$${ADRSIZE}
OBJDIR = obj/$${SYSDIR}/
# message($$SYSDIR)
# set rpath on linux
linux:LIBS += -z \
    defs \
    -z \
    origin \
    -z \
    now \
    -Wl,-rpath=\'\$\$ORIGIN\' 



# add library directory
# LIBDIR = $${SDK_PATH}/lib/$${SYSDIR}/
# LIBS += -L$${LIBDIR} $${LIBDIR}/pro.a
win32 {
    LIBS += -L$${SDK_PATH}/lib/$${SYSDIR}/ -lida
}
else:!mac:unix {
    is64 {
        IDA_LIB = ida64
    }
    else {
        IDA_LIB = ida
    }
    INCLUDEPATH += /usr/include/python2.7
    INCLUDEPATH += $${PWD}/../protobuf-2.6.0/src/
    LIBS += -lpython2.7
    LIBS += -L$${IDA_PATH} -l$${IDA_LIB} -L$${SDK_PATH}/lib/$${SYSDIR}/ -lprotobuf
}

# message($$LIBS)
# set all build directories
MOC_DIR = $${OBJDIR}
OBJECTS_DIR = $${OBJDIR}
RCC_DIR = $${OBJDIR}
UI_DIR = $${OBJDIR}

# set the destination directory for the binary
# DESTDIR = ../../bin/$${TARGET_PROCESSOR_NAME}_$${SYSNAME}_$${COMPILER_NAME}$${OPTSUF}/plugins/

# run install_name_tool after linking
# mac:QMAKE_POST_LINK += install_name_tool -change /idapathsample/libida$${SUFF64}.dylib @executable_path/libida$${SUFF64}.dylib $(TARGET)
#                        do the same for the Qt libraries

# on unix systems make sure to copy the file with the correct name,
# since qmake adds a prefix and suffix, and ignores TARGET_EXT
unix {
    MY_TARGET = ${DESTDIR}$${TARGET}$${TARGET_EXT}
    COPY_TARGET = $(COPY_FILE) ${DESTDIR}${TARGET} $$MY_TARGET;
    COPY_TARGET2 = $(COPY_FILE) $$MY_TARGET $${IDA_PATH}/plugins/;
    QMAKE_POST_LINK += $$COPY_TARGET
    QMAKE_POST_LINK += $$COPY_TARGET2
}

SOURCES += \
    ../common/cpp/rpc.pb.cc \
    sync/sync.cpp \
    choosememorydialog.cpp \
    entry.cpp \
    externsegdata.cpp \
    globalsettingsmanager.cpp \
    highlighter.cpp \
    idadump.cpp \
    idastorage.cpp \
    jedi.cpp \
    jedicompletionworker.cpp \
    labeless_ida.cpp \
    pyollyview.cpp \
    pysignaturetooltip.cpp \
    pythonpalettemanager.cpp \
    rpcdata.cpp \
    rpcthreadworker.cpp \
    settingsdialog.cpp \
    textedit.cpp \
    types.cpp \
    util/util_ida.cpp \
    util/util_idapython.cpp \
    util/util_net.cpp \
    util/util_protobuf.cpp \
    util/util_python.cpp

HEADERS += \
    ../common/cpp/rpc.pb.h \
    ../common/version.h \
    sync/sync.h \
    choosememorydialog.h \
    externsegdata.h \
    globalsettingsmanager.h \
    highlighter.h \
    idadump.h \
    idastorage.h \
    jedi.h \
    jedicompletionworker.h \
    labeless_ida.h \
    palette.h \
    pyollyview.h \
    pysignaturetooltip.h \
    pythonpalettemanager.h \
    rpcdata.h \
    rpcthreadworker.h \
    settingsdialog.h \
    textedit.h \
    types.h \
    util/util_ida.h \
    util/util_idapython.h \
    util/util_net.h \
    util/util_protobuf.h \
    util/util_python.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    ui/pyollyview.ui \
    ui/choosememorydialog.ui \
    ui/settingsdialog.ui

RESOURCES += \
    res/res.qrc
