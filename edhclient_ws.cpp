#include "edhclient_ws.h"
#include "edhclient_private.h"

#include <iostream>

#include <QFile>

#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslConfiguration>
#include <QtWebSockets/QWebSocket>

using namespace eDrillingHub;

WebsocketClient::WebsocketClient(bool secure) : _ssl_socket(secure) {
    _ws.reset(new QWebSocket);

    connect(_ws.get(), &QWebSocket::textMessageReceived, this, [this](const QString &message) {
        handle(message);
    });
    connect(_ws.get(), &QWebSocket::binaryFrameReceived, this, [this](const QByteArray &frame, bool isLastFrame) {
        Q_UNUSED(isLastFrame)
        handleDownload(frame);
    });
    connect(_ws.get(), &QWebSocket::stateChanged, this, [this](QAbstractSocket::SocketState state) {
        switch (state)  {
        case QAbstractSocket::ConnectedState:
            emit connected();
            break;
        case QAbstractSocket::UnconnectedState:
            emit disconnected();
            break;
        default:
            break;
        }
    });
}

WebsocketClient* WebsocketClient::create(bool secure) {
    std::unique_ptr<WebsocketClient> client;

    client.reset(new WebsocketClient(secure));
    if (secure) {
        QFile ca(":/edhclient/CA-eDrilling.crt");
        if (! ca.open(QIODevice::ReadOnly)) {
            std::cerr << "eDrilling CA Certificate not found, compilation error" << std::endl;
            return nullptr;
        }

        auto ssl_config = client->_ws->sslConfiguration();
        auto ca_certs = ssl_config.caCertificates();
        ca_certs.append(QSslConfiguration::systemCaCertificates());
        ca_certs.append(QSslCertificate(&ca, QSsl::Pem));
        ssl_config.setCaCertificates(ca_certs);
        client->_ws->setSslConfiguration(ssl_config);
    }

    return client.release();
}

void WebsocketClient::open() {
    QUrl url;

    if (_ssl_socket) {
        url.setScheme("wss");
    } else {
        url.setScheme("ws");
    }

    url.setHost(_priv->host);
    url.setPort(_priv->port);
    url.setPath("/edh");

    if (_networkProxy) {
        _ws->setProxy(*_networkProxy);
    }
    _ws->open(url);
}

void WebsocketClient::close() {
    _ws->close();
}

void WebsocketClient::setIgnoreSslErrors(bool enable) {
    if (enable) {
        QObject::connect(_ws.get(), &QWebSocket::sslErrors, &WebsocketClient::_onSslError);
        QObject::connect(_ws.get(), &QWebSocket::sslErrors,
                         _ws.get(), static_cast<void (QWebSocket::*)(const QList<QSslError> &errors)>(&QWebSocket::ignoreSslErrors));
    } else {
        QObject::disconnect(_ws.get(), &QWebSocket::sslErrors, nullptr, nullptr);
    }
}

QString WebsocketClient::errorString() {
    return _ws->errorString();
}

void WebsocketClient::write(const QString &message) {
    _ws->sendTextMessage(message);
}

void WebsocketClient::writeBinary(const QByteArray &data) {
    _ws->sendBinaryMessage(data);
}

void WebsocketClient::_onSslError(const QList<QSslError> &errors) {
    for (const auto& error : errors) {
        std::cerr << "SocketClient: SSL Error: " << error.errorString().toStdString() << std::endl;
    }
}
