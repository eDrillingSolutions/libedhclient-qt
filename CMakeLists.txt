CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(libedhclient)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)

FIND_PACKAGE(Qt5Network)
FIND_PACKAGE(Qt5WebSockets)

ADD_DEFINITIONS(-DEXPORT_LIBEDRILLINGHUB)

ADD_LIBRARY(edhclient
    SHARED
    edhclient.cpp
    edhprotocol.cpp
    edhclient_socket.cpp
    edhclient_ws.cpp

    ../../serialization.cpp
    ../../tag/quality.cpp

    edhclient_resources.qrc
)

TARGET_LINK_LIBRARIES(edhclient
    Qt5::Network
    Qt5::WebSockets
)

SET_PROPERTY(TARGET edhclient PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET edhclient PROPERTY CXX_STANDARD_REQUIRED true)
