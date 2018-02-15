#pragma once

#include "edhclient.h"

class QWebSocket;
class QSslError;

namespace eDrillingHub {
    class WebsocketClient : public Client {
    public:
        static WebsocketClient* create(bool secure);

        void open();
        void close();

        void setIgnoreSslErrors(bool enable);
        QString errorString();

        void write(const QString& message);
    private:
        WebsocketClient(bool secure);

        static void _onSslError(const QList<QSslError> &errors);

        std::unique_ptr<QWebSocket> _ws;
        bool _ssl_socket = false;

        int _readBufferIdx = 0;
        int _readBufferPos = 0;
        QByteArray _readBuffer;
    };
}
