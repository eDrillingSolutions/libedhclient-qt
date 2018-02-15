QT += network
QT += websockets

INCLUDEPATH = $$PWD

SOURCES += \
    $$PWD/edhprotocol.cpp \
    $$PWD/edhclient.cpp \
    $$PWD/edhclient_socket.cpp \
    $$PWD/edhclient_ws.cpp \
    $$PWD/../../serialization.cpp \
    $$PWD/../../tag/quality.cpp \
    $$PWD/../../util.cpp \

HEADERS += \
    $$PWD/edhprotocol.h \
    $$PWD/edhclient.h \
    $$PWD/edhclient_socket.h \
    $$PWD/edhclient_ws.h \
    $$PWD/../../serialization.h \
    $$PWD/../../tag/quality.h \
    $$PWD/../../util.h \

RESOURCES += $$PWD/edhclient_resources.qrc \
