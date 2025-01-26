#-------------------------------------------------
#
# Project created by QtCreator 2016-10-05T14:38:35
#
#-------------------------------------------------

QT       += widgets

# TARGET = labeless_ida_70
# TARGET = labeless_ida_83
# TARGET = labeless_ida_90
TEMPLATE = lib
CONFIG += plugin c++11
CONFIG -= debug
CONFIG *= release force_debug_info

# configuration options meaning:
# - ea64 - bitness of opened targets
# CONFIG += ea64


contains(CONFIG, labeless_ida_70) {
    TARGET = labeless_ida_70
} else: contains(CONFIG, labeless_ida_83) {
    TARGET = labeless_ida_83
} else: contains(CONFIG, labeless_ida_90) {
    TARGET = labeless_ida_90
} else {
    error("No target specified, add to CONFIG one of the following: labeless_ida_70|labeless_ida_83|labeless_ida_90")
}

# `x64` deprecated
#CONFIG += x64

# `is_ida7` deprecated
#CONFIG += is_ida7
QT_NAMESPACE = QT

SDK_PATH = $$(SDK_PATH)
IDA_PATH = $$(IDA_PATH)

isEmpty(SDK_PATH) | isEmpty(IDA_PATH) {
    error("both SDK_PATH and IDA_PATH env variables should be set")
}

equals(TARGET, "labeless_ida_70") {
#    SDK_PATH = $$PWD/../../idasdk70
#    IDA_PATH = $$PWD/../../idafree-7.0
} 
equals(TARGET, "labeless_ida_83") {
#    SDK_PATH = $$PWD/../../idasdk_pro83
#    IDA_PATH = $$PWD/../../idafree-8.4
}
equals(TARGET, "labeless_ida_90") {
#    SDK_PATH = $$PWD/../../idasdk90sp1
#    IDA_PATH = $$PWD/../../ida-free-pc-9.0
    mac {
        IDA_PATH = /Applications/IDA\ Free\ 9.0.app/Contents/MacOS
    }
}
message("SDK_PATH: $$SDK_PATH, IDA_PATH: $$IDA_PATH")
# add IDA SDK paths
INCLUDEPATH += $${SDK_PATH}/include
DEFINES += __IDP__ \
    NO_OBSOLETE_FUNCS \
    LL_LIBRARY \
    __QT__ \
    __X64__ \
    QT_NO_UNICODE_LITERAL

TARGET_PROCESSOR_NAME = x64

# set rpath on linux
linux:LIBS += -z \
    defs \
    -z \
    origin \
    -z \
    now \
    -Wl,-rpath=\'\$\$ORIGIN\'


# FIXME: adjust right paths below
win32 {
    
}
unix {
    #QMAKE_CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
    #QMAKE_CFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
}


ea64 {
    TARGET = $${TARGET}_64
}

!ea64:equals(TARGET, "labeless_ida_90") {
    error("IDA 9 should have ea64 config set for both 32 and 64-bit targets")
}


win32 {
    TARGET_EXT = .dll
    DEFINES += __NT__
    SYSNAME = win
    COMPILER_NAME = vc
    LIBS += -L$$PWD/../3rdparty/libs -llibprotobuf_v140 -lws2_32
}
else:!mac:unix {
    TARGET_EXT = .so
    
    DEFINES += __LINUX__ \
               _FORTIFY_SOURCE=0
    SYSNAME = linux
    COMPILER_NAME = gcc
    # avoid linking GLIBC_2.11 symbols (longjmp_chk)
    # CFLAGS += -D_FORTIFY_SOURCE=0
}
mac { # scope name must be 'mac'
    TARGET_EXT = .dylib
    DEFINES += __MAC__ GOOGLE_PROTOBUF_NO_RDTSC
    # unfortunately `GOOGLE_PROTOBUF_NO_RDTSC` is required due to the issue of `processor_t` redefinition while both IDA SDK and protobuf are included in the same source file :(
    SYSNAME = mac
    COMPILER_NAME = clang
    CONFIG += macx-clang
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 15
    # QMAKE_INFO_PLIST = Info.plist
    TARGET_PROCESSOR_NAME = arm64
}


CONFIG(debug, debug|release) {
  DEFINES += _DEBUG
}

ea64 {
    DEFINES += __EA64__
    SUFF64 = 64x
    ADRSIZE = 64
}
else {
    # TARGET_PROCESSOR_NAME = x86
    # is64 {
    #     DEFINES += __EA64__
    #     SUFF64 = 64
    #     ADRSIZE = 64
    # }
    # else {
    #     ADRSIZE = 32
    # }
    ADRSIZE = 32
}

SYSDIR = $${TARGET_PROCESSOR_NAME}_$${SYSNAME}_$${COMPILER_NAME}_$${ADRSIZE}
equals(TARGET, "labeless_ida_83")|equals(TARGET, "labeless_ida_83_64") {
    # sh1tf*ck
    SYSDIR = $${SYSDIR}_pro
}
OBJDIR = obj/$${SYSDIR}/
# message($$SYSDIR)


# add library directory
# LIBDIR = $${SDK_PATH}/lib/$${SYSDIR}/
# LIBS += -L$${LIBDIR} $${LIBDIR}/pro.a
# Build protobuf for macos with the following:
# cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/Users/al/dev/labeless/3rdparty/protobuf-3.20.3/dist -DCMAKE_POSITION_INDEPENDENT_CODE=ON -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_INSTALL=ON -Dprotobuf_BUILD_SHARED_LIBS=OFF ../cmake

PROTOBUF_BUILD_DIR = $${PWD}/../3rdparty/protobuf-3.20.3/dist
ea64:!equals(TARGET, "labeless_ida_90_64") {
    IDA_LIB = ida64
}
else {
    IDA_LIB = ida
}

win32 {
    LIBS += -L$${SDK_PATH}/lib/$${SYSDIR}/ -lida
}
else:!mac:unix {
    #INCLUDEPATH += /usr/include/python2.7
    #LIBS += -lpython2.7
    !equals(TARGET, "labeless_ida_70_64") {
        # seems like IDA FREE 7.0 has obfuscated exports so link with it will fail, so don't add IDA dir as a lib dir
        LIBS += -L$${IDA_PATH}
    }
    LIBS +=  -l$${IDA_LIB} -L$${SDK_PATH}/lib/$${SYSDIR}/ -L$${PROTOBUF_BUILD_DIR}/lib -lprotobuf
}

INCLUDEPATH += $${PROTOBUF_BUILD_DIR}/include
+mac {
    LIBS += $${SDK_PATH}/lib/$${SYSDIR}/lib$${IDA_LIB}.dylib
    LIBS += $${PROTOBUF_BUILD_DIR}/lib/libprotobuf.a
    QMAKE_APPLE_DEVICE_ARCHS = arm64
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
    QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-int-in-bool-context -Wno-ignored-qualifiers -Wno-class-memaccess -Wno-cast-function-type -Wno-error
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
    pausenotificationlistener.cpp \
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
    util/util_protobuf.cpp

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
    pausenotificationlistener.h \
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
    util/util_protobuf.h

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

message($${INCLUDEPATH})
message($${LIBS})
