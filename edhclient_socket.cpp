#include "edhclient_socket.h"
#include "edhclient_private.h"

#include <iostream>

#include <QFile>

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslConfiguration>

using namespace eDrillingHub;

static const QString message_end_marker = QString("\r\n");

SocketClient::SocketClient(bool secure) {
    if (secure) {
        auto socket = new QSslSocket();
        _socket.reset(socket);
        _ssl_socket = true;
    } else {
        _socket.reset(new QTcpSocket());
        _ssl_socket = false;
    }

    connect(_socket.get(), &QTcpSocket::readyRead, this, &SocketClient::_onSocketReadyRead);
    connect(_socket.get(), &QTcpSocket::stateChanged, this, [this](QAbstractSocket::SocketState state) {
        switch (state) {
        case QAbstractSocket::ConnectedState:
            _socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
            if (_ssl_socket) {
                auto sslsocket = dynamic_cast<QSslSocket*>(_socket.get());
                QObject::connect(sslsocket, &QSslSocket::encrypted, [=] {
                    emit connected();
                });
                sslsocket->startClientEncryption();
            } else {
                emit connected();
            }
            break;
        case QAbstractSocket::UnconnectedState:
            emit disconnected();
            break;
        default:
            break;
        }
    });
}

SocketClient* SocketClient::create(bool secure) {
    std::unique_ptr<SocketClient> client;

    client.reset(new SocketClient(secure));
    if (secure) {
        QFile ca(":/edhclient/CA-eDrilling.crt");
        if (! ca.open(QIODevice::ReadOnly)) {
            std::cerr << "eDrilling CA Certificate not found, compilation error" << std::endl;
            return nullptr;
        }

        auto sslsocket = dynamic_cast<QSslSocket*>(client.get()->_socket.get());
        sslsocket->addCaCertificates(QSslConfiguration::systemCaCertificates());
        sslsocket->addCaCertificate(QSslCertificate(&ca, QSsl::Pem));
        sslsocket->setProtocol(QSsl::TlsV1_2OrLater);
    }

    return client.release();
}

void SocketClient::open() {
    if (_networkProxy) {
        _socket->setProxy(*_networkProxy);
    }
    _socket->connectToHost(_priv->host, _priv->port, QIODevice::ReadWrite);
}

void SocketClient::close() {
    _socket->close();
}

void SocketClient::setIgnoreSslErrors(bool enable) {
    auto ssl_socket = dynamic_cast<QSslSocket*>(_socket.get());
    if (ssl_socket == nullptr) {
        return;
    }

    if (enable) {
        QObject::connect(ssl_socket, static_cast<void (QSslSocket::*)(const QList<QSslError> &errors)>(&QSslSocket::sslErrors), &SocketClient::_onSslError);
        QObject::connect(ssl_socket, static_cast<void (QSslSocket::*)(const QList<QSslError> &errors)>(&QSslSocket::sslErrors),
                         ssl_socket, static_cast<void (QSslSocket::*)(const QList<QSslError> &errors)>(&QSslSocket::ignoreSslErrors));
    } else {
        QObject::disconnect(ssl_socket, static_cast<void (QSslSocket::*)(const QList<QSslError> &errors)>(&QSslSocket::sslErrors), nullptr, nullptr);
    }
}

QString SocketClient::errorString() {
    return _socket->errorString();
}

void SocketClient::write(const QString &message) {
    _socket->write(message.toUtf8());
    _socket->write(message_end_marker.toUtf8());
}

void SocketClient::writeBinary(const QByteArray &data) {
    _socket->write(data);
}

void SocketClient::_onSocketReadyRead() {
    QByteArray bytes = _socket->readAll();
    _readBuffer.append(bytes);

    _readBufferIdx = 0;
    _readBufferPos = _readBuffer.indexOf(message_end_marker);

    if (_readBufferPos >= 0) {
        while (_readBufferPos >= 0) {
            QString reply = QString::fromUtf8(_readBuffer.mid(_readBufferIdx, _readBufferPos - _readBufferIdx));

            _readBufferIdx = _readBufferPos + message_end_marker.length();
            _readBufferPos = _readBuffer.indexOf(message_end_marker, _readBufferIdx);

            handle(reply);
        }

        _readBuffer.remove(0, _readBufferIdx);
    }
}

void SocketClient::_onSslError(const QList<QSslError> &errors) {
    for (const auto& error : errors) {
        std::cerr << "SocketClient: SSL Error: " << error.errorString().toStdString() << std::endl;
    }
}
