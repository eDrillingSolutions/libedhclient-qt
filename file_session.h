#pragma once

#include <QObject>

class QFile;
class QIODevice;

namespace eDrillingHub {
    class DownloadSession : public QObject {
        Q_OBJECT
    public:
        enum class FailReason {
            Hash,
            Server,
            Unknown
        };
        Q_ENUM(FailReason)
    signals:
        void download(const QString& file);
        void progress(QByteArray bytes, qint64 transferred, qint64 size);
        void success();
        void fail(FailReason reason, const QString& description);
    };

    class UploadSession : public QObject
    {
        Q_OBJECT
    public:
        enum class FailReason {
            Hash,
            Server,
            Unknown
        };
        Q_ENUM(FailReason)

        explicit UploadSession(QObject *parent = 0);
        void upload(const QString& filename, QFile& file);

        const QString& filename() const {return filename_;}
        qint64 size() const {return size_;}
        QIODevice* device() {return dev_;}
    signals:
        void server_request();
        void server_ready();
        void progress(qint64 transferred);
        void success();
        void fail(UploadSession::FailReason reason, const QString& description);

    public slots:

    private:
        QString filename_;
        qint64 size_;
        QIODevice* dev_;
    };
}
