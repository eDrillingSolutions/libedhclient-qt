#include "edhclient.h"
#include "edhclient_private.h"

#include "edhclient_socket.h"
#include "edhclient_ws.h"

#include "serialization.h"
#include "file_session.h"

#include <iostream>

#include <QStringList>
#include <QMetaEnum>

#include <QtNetwork/QNetworkProxy>

using namespace eDrillingHub;

static const QRegularExpression _commandSplitter("(?<!\\\\)\\|");
static const QRegularExpression _hashListSplitter("(?<!\\\\)#");
static QMetaEnum _qualityEnum() {
    static QMetaEnum _enum = QMetaEnum::fromType<Tag::Quality::Value>();
    return _enum;
}

Client::Client() {
    qRegisterMetaType<ReadTagHolder>();
    qRegisterMetaType<DownloadSession::FailReason>();
    qRegisterMetaType<UploadSession::FailReason>();
}

Client::~Client() {
}

Client* Client::create(const QUrl& url) {
    QStringList supported_schemas = {"edh", "edhs", "wsedh", "wssedh"};
    QString scheme = url.scheme();
    if (! supported_schemas.contains(scheme)) {
        std::cerr << "Invalid Scheme for eDrilling Hub, supported protocols are edh(s) and ws(s)edh" << std::endl;
        return nullptr;
    }

    auto host = url.host();
    auto port = url.port(0);

    if (port <= 0 || port > std::numeric_limits<quint16>::max()) {
        std::cerr << "Invalid port number" << std::endl;
        return nullptr;
    }

    std::unique_ptr<Client> client;
    if (scheme == "edh") {
        client.reset(SocketClient::create(false));
    } else if (scheme == "edhs") {
        client.reset(SocketClient::create(true));
    } else if (scheme == "wsedh") {
        client.reset(WebsocketClient::create(false));
    } else if (scheme == "wssedh") {
        client.reset(WebsocketClient::create(true));
    } else {
        std::cerr << "Unhandled schema" << scheme.toStdString() << std::endl;
        return nullptr;
    }

    if (! client) {
        return nullptr;
    }

    client->_priv.reset(new ClientPrivate());
    client->_priv->host = host;
    client->_priv->port = static_cast<quint16>(port);

    return client.release();
}

void Client::proxy(const QNetworkProxy &networkProxy) {
    _networkProxy.reset(new QNetworkProxy(networkProxy));
}

void Client::updateTagValue(const QString &tagName, qint64 timestamp, const QString &type, const QString &value) {
    bool ok;
    QMetaType::Type metaType = static_cast<QMetaType::Type>(type.toInt(&ok));
    if (! ok) {
        qWarning() << QString("Dropped tagUpdate on %1, unknown metaType '%2'").arg(tagName, type);
        return;
    }

    QVariant variantValue = Serialization::deserializeTagValue(metaType, value);
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestamp);

    emit tagValueUpdated(tagName, dt, metaType, variantValue);
}

void Client::updateTagQuality(const QString &tagName, const QString &quality) {
    bool qOk;
    Tag::Quality::Value edhQuality = static_cast<Tag::Quality::Value>(_qualityEnum().keyToValue(quality.toUtf8(), &qOk));
    if (qOk) {
        emit tagQualityUpdated(tagName, edhQuality);
    }
}

void Client::updateTagUnit(const QString &tagName, const QString &unit) {
    emit tagUnitUpdated(tagName, unit);
}

void Client::updateTag(const QString& tagName, qint64 timestamp, const QString& type, const QString& value, const QString& unit, const QString& quality) {
    updateTagUnit(tagName, unit);
    updateTagQuality(tagName, quality);
    updateTagValue(tagName, timestamp, type, value);
}

void Client::handle(const QString &line) {
    QStringList splits = line.trimmed().split(_commandSplitter);

    if (splits.size() == 0) {
        return;
    }

    QString main = splits[0];
    if (main == QStringLiteral("servertime")) {

    } else if (main == QStringLiteral("browse")) {
        if (splits.size() == 1) {
            return;
        }

        if (splits.size() < 7) {
            QString type = splits[1];
            if (type == QString("end")) {
                emit tagsImported();
            }

            return;
        }

        QString tagName = splits[1];
        qint64 timestamp = splits[2].toLongLong();
        QString type = splits[3];
        QString value = splits[4];
        QString unit = splits[5];
        QString quality = splits[6];

        updateTag(tagName, timestamp, type, value, unit, quality);
    } else if (main == QStringLiteral("subscription")) {
        if (splits.size() < 4) {
            qWarning() << "Invalid subscription from server";
            return;
        }

        QString updateType = splits[1];
        QString tagName = splits[2];
        if (updateType == QStringLiteral("value")) {
            if (splits.size() < 6) {
                qWarning() << "Invalid value-subscription from server";
                return;
            }

            qint64 timestamp = splits[3].toLongLong();
            QString type = splits[4];
            QString value = splits[5];

            updateTagValue(tagName, timestamp, type, value);
        } else if (updateType == QStringLiteral("quality")) {
            QString quality = splits[3];
            updateTagQuality(tagName, quality);
        } else if (updateType == QStringLiteral("unit")) {
            QString unit = splits[3];
            updateTagUnit(tagName, unit);
        } else {
            qWarning() << "Unknown subscription from server" << updateType;
            return;
        }
    } else if (main == QStringLiteral("read")) {
        if (splits.size() < 7) {
            QString command = splits[2];
            if (command == QString("queued")) {
                // ignore, server is just polite
            } else {
                qWarning() << "Unknown read reply from server" << splits;
            }

            return;
        }

        QString tagName = splits[1];
        if (_readingTags.contains(tagName)) {
            QString timestamp = splits[2];
            QMetaType::Type type = static_cast<QMetaType::Type>(splits[3].toInt());
            QString value = splits[4];

            QVariant variantValue = Serialization::deserializeScalarValue(type, value);
            qint64 ts = timestamp.toLongLong();
            _readingTags[tagName][0].timestamps.append(QDateTime::fromMSecsSinceEpoch(ts).toUTC());
            _readingTags[tagName][0].values.append(variantValue);
        } else if (splits.size() >= 7) {
            // direct read
            QString tagName = splits[1];
            qint64 timestamp = splits[2].toLongLong();
            QString type = splits[3];
            QString value = splits[4];
            QString unit = splits[5];
            QString quality = splits[6];

            updateTag(tagName, timestamp, type, value, unit, quality);
        } else {
            qWarning() << "readReply from server, unknown tagName" << tagName;
        }
    } else if (main == QStringLiteral("readStart")) {
        if (splits.size() < 4) {
            qWarning() << "Unknown readStart command from server";
            return;
        }
        QString tag = splits[1];
        ReadTagHolder holder;
        holder.from = QDateTime::fromMSecsSinceEpoch(splits[2].toLongLong()).toUTC();
        holder.to   = QDateTime::fromMSecsSinceEpoch(splits[3].toLongLong()).toUTC();
        _readingTags[tag].append(holder);
    } else if (main == QStringLiteral("readEnd")) {
        if (splits.size() < 2) {
            qWarning() << "Unknown readEnd command from server";
            return;
        }

        QString tag = splits[1];
        emit tagRead(tag, _readingTags[tag][0]);

        _readingTags[tag].removeFirst();
        if (_readingTags[tag].isEmpty()) {
            _readingTags.remove(tag);
        }
    } else if (main == QStringLiteral("subscribe")) {
        QString subscribeReply = splits[1];
        if (subscribeReply == QStringLiteral("ok")) {
            QString tagName = splits[2];
            qint64 timestamp = splits[3].toLongLong();
            QString type = splits[4];
            QString value = splits[5];
            QString unit = splits[6];
            QString quality = splits[7];

            updateTag(tagName, timestamp, type, value, unit, quality);
        }
    } else if (main == QStringLiteral("file")) {
        if (splits.size() < 2) {
            qWarning() << "Unknown file reply from server";
            return;
        }

        QString status = splits[1];
        if (status == QStringLiteral("ok")) {
            if (splits.size() < 3) {
                if (! _downloads.empty()) {
                    _downloads.removeFirst();
                }
                qWarning() << "Unknown file OK reply from server";
                return;
            }
            auto& download = _downloads.first();
            download.size = splits[2].toLongLong();

            emit downloadStarted(download);
        } else if (status == QStringLiteral("error")) {
            if (_downloads.empty()) {
                qWarning() << "No downloads are active when file error was received from server";
                return;
            }

            auto d = _downloads.takeFirst();
            if (splits.size() < 3) {
                d.session->fail(DownloadSession::FailReason::Unknown, QString());
            } else {
                d.session->fail(DownloadSession::FailReason::Server, splits[2]);
            }
        } else if (status == QStringLiteral("done")) {
            if (_downloads.empty()) {
                qWarning() << "No downloads are active when file done was received from server";
                return;
            }

            auto d = _downloads.takeFirst();
            if (splits.size() < 3) {
                d.session->fail(DownloadSession::FailReason::Unknown, QString());
                qWarning() << "Unknown file done reply from server";
                return;
            }

            QString hash = splits[2];
            if (d.hashfn->result().toHex() == hash) {
                d.session->success();
            } else {
                d.session->fail(DownloadSession::FailReason::Hash, QString());
            }
        } else if (status == QStringLiteral("upload")) {
            if (splits.size() < 3) {
                qWarning() << "Unknown file upload reply from server";
                return;
            }

            if (_uploads.isEmpty()) {
                qWarning() << "file_upload reply from server, but no active upload sessions";
                return;
            }

            auto upload_status = splits[2];
            if (upload_status == QStringLiteral("ready")) {
                auto session = _uploads.first();
                session->server_ready();
            } else if (upload_status == QStringLiteral("success")) {
                auto session = _uploads.takeFirst();
                session->success();
            } else if (upload_status == QStringLiteral("hash_mismatch")) {
                auto session = _uploads.takeFirst();
                session->fail(UploadSession::FailReason::Hash, "HashCode mismatch");
            } else if (upload_status == QStringLiteral("error")) {
                QString msg;
                if (splits.size() > 3) {
                    msg = splits[3];
                }
                auto session = _uploads.takeFirst();
                session->fail(UploadSession::FailReason::Server, msg);
            } else {
                qWarning() << "Unknown file_upload reply from server";
            }
        } else {
            qWarning() << "Unknown file reply";
            return;
        }
    } else if (main == QStringLiteral("db")) {
        if (splits.size() < 3) {
            qWarning() << "Unknown db reply from server" << splits;
            return;
        }

        QString db_query = splits[1];
        QString tag = splits[2];

        if (db_query == "range") {
            if (splits.size() < 5) {
                qWarning() << "Unknown db range reply from server" << splits;
                return;
            }

            emit tagRange(tag, splits[3].toLongLong(), splits[4].toLongLong());
        } else {
            qWarning() << "Unknown db reply" << splits;
        }
    }
}

void Client::handleDownload(const QByteArray &bytes) {
    auto& d = _downloads.first();
    if ((d.received + bytes.size()) >= d.size) {
        qint64 rest = d.size - d.received;
        auto rest_bytes = bytes.mid(0, rest);
        processDownload(rest_bytes);

        emit downloadFinished(d, bytes.mid(rest));
    } else {
        processDownload(bytes);
    }

}

std::shared_ptr<DownloadSession> Client::createDownloadSession() {
    Download download;
    download.session = std::make_shared<DownloadSession>();
    download.hashfn = std::make_shared<QCryptographicHash>(eDrillingHub::Protocol::hashing_algorithm);
    _downloads.append(download);

    connect(download.session.get(), &DownloadSession::download, this, [this](const QString& file) {
        write(eDrillingHub::Protocol::FileTransfer(file));
    });

    return download.session;
}

std::shared_ptr<UploadSession> Client::createUploadSession() {
    auto session = std::make_shared<UploadSession>();
    _uploads.append(session);

    connect(session.get(), &UploadSession::server_request, this, [this, session] {
        write(eDrillingHub::Protocol::FileUploadRequest(session->filename(), session->size()));
    });
    connect(session.get(), &UploadSession::server_ready, this, [this, session] {
        auto tail = eDrillingHub::Protocol::FileUploadTransfer(session, [this](const QByteArray& data) {
            writeBinary(data);
        });
        write(tail);
    });

    return session;
}

void Client::processDownload(const QByteArray &data) {
    auto& d = _downloads.first();
    d.received += data.size();
    d.hashfn->addData(data);
    d.session->progress(data, d.received, d.size);
}
