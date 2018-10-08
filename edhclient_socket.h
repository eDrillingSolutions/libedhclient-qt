#pragma once

#include "edhclient.h"

class QTcpSocket;
class QSslError;

namespace eDrillingHub {
    class SocketClient : public Client {
    public:
        static SocketClient* create(bool secure);

        void open();
        void close();

        void setIgnoreSslErrors(bool enable);
        QString errorString();

        void write(const QString& message);
        void writeBinary(const QByteArray& data);
    private:
        SocketClient(bool secure);

        void _onSocketReadyRead();
        static void _onSslError(const QList<QSslError> &errors);

        std::unique_ptr<QTcpSocket> _socket;
        bool _ssl_socket = false;

        int _readBufferIdx = 0;
        int _readBufferPos = 0;
        QByteArray _readBuffer;
    };
}
