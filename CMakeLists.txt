CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(edhclient)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

FIND_PACKAGE(Qt5Core)
FIND_PACKAGE(Qt5Network)
FIND_PACKAGE(Qt5WebSockets)

GET_TARGET_PROPERTY(Qt5Core_INCLUDE_DIRS Qt5::Core INTERFACE_INCLUDE_DIRECTORIES)
GET_TARGET_PROPERTY(Qt5Network_INCLUDE_DIRS Qt5::Network INTERFACE_INCLUDE_DIRECTORIES)
GET_TARGET_PROPERTY(Qt5WebSockets_INCLUDE_DIRS Qt5::WebSockets INTERFACE_INCLUDE_DIRECTORIES)

ADD_DEFINITIONS(-DEXPORT_LIBEDRILLINGHUB)

ADD_LIBRARY(edhclient_objects
    OBJECT

    edhtypes.cpp
    edhclient.cpp
    edhprotocol.cpp
    edhclient_socket.cpp
    edhclient_ws.cpp

    serialization.cpp
)
TARGET_INCLUDE_DIRECTORIES(
    edhclient_objects
    PRIVATE
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5WebSockets_INCLUDE_DIRS}
)
SET_TARGET_PROPERTIES(edhclient_objects PROPERTIES POSITION_INDEPENDENT_CODE True)

ADD_LIBRARY(edhclient_static
    STATIC
    edhclient_resources.qrc
    $<TARGET_OBJECTS:edhclient_objects>
)
TARGET_LINK_LIBRARIES(edhclient_static
    Qt5::Network
    Qt5::WebSockets
)

ADD_LIBRARY(edhclient
    SHARED
    edhclient_resources.qrc
    $<TARGET_OBJECTS:edhclient_objects>
)
TARGET_LINK_LIBRARIES(edhclient
    Qt5::Network
    Qt5::WebSockets
)
SET_TARGET_PROPERTIES(edhclient PROPERTIES VERSION 1.0 SOVERSION 1.0.0)

INSTALL(TARGETS edhclient edhclient_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
INSTALL(FILES edhclient.h edhprotocol.h edhmatrix.h edhtypes.h DESTINATION include)
