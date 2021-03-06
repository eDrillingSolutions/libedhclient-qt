#pragma once

#include <QObject>
#include <memory>

#include <QtNetwork/QAbstractSocket>

#include "edhtypes.h"
#include "edhprotocol.h"

namespace eDrillingHub {
    struct ClientPrivate;

    class EXPORT_LIBEDRILLINGHUB_SPEC Client : public QObject {
        Q_OBJECT
    public:
        Client();
        virtual ~Client();
        static Client* create(const QUrl& url);

        virtual void open() = 0;
        virtual void close() = 0;

        void proxy(const QNetworkProxy &networkProxy);
        virtual void setIgnoreSslErrors(bool enable) = 0;
        virtual QString errorString() = 0;

        virtual void write(const QString& message) = 0;
        virtual void writeBinary(const QByteArray& data) = 0;
        std::shared_ptr<DownloadSession> createDownloadSession();
        std::shared_ptr<UploadSession> createUploadSession();
    signals:
        void tagValueUpdated(const QString& tagName, const QDateTime& timestamp, QMetaType::Type metaType, const QVariant& variantValue);
        void tagQualityUpdated(const QString& tagName, Tag::Quality::Value ioTagQuality);
        void tagUnitUpdated(const QString& tagName, const QString& unit);
        void tagRead(const QString& tag, const ReadTagHolder& data);
        void tagRange(const QString& tag, qint64 start, qint64 end);
        void tagsImported();

        void downloadStarted(const Download& session);
        void downloadFinished(const Download& session, const QByteArray& rest_bytes);

        void socketError(QAbstractSocket::SocketError error);

        void connected();
        void disconnected();
    protected:
        void handle(const QString& line);
        void handleDownload(const QByteArray& bytes);

        std::unique_ptr<QNetworkProxy> _networkProxy;
        std::unique_ptr<ClientPrivate> _priv;
    private:
        void updateTagValue(const QString& tagName, qint64 timestamp, const QString& type, const QString& value);
        void updateTagQuality(const QString& tagName, const QString& quality);
        void updateTagUnit(const QString& tagName, const QString& unit);
        void updateTag(const QString& tagName, qint64 timestamp, const QString& type, const QString& value, const QString &unit, const QString &quality);
        void processDownload(const QByteArray &data);

        QHash<QString, QList<ReadTagHolder>> _readingTags;
        QVector<Download> _downloads;
        QVector<std::shared_ptr<UploadSession>> _uploads;
    };
}
