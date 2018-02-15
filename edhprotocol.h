#ifndef EDHPROTOCOL_H
#define EDHPROTOCOL_H

#include <memory>

#include <QString>
#include <QVector>
#include <QVariant>
#include <QDateTime>
#include <QCryptographicHash>

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

    class ServerConfiguration {
        Q_GADGET
    public:

        enum class Operation {
            OP_CREATE,
            OP_READ,
            OP_UPDATE,
            OP_DELETE
        };
        QString getOperationPrefix(){return "OP_";}
        Q_ENUM(Operation)

        enum class Target {
            PRE_ACTION,
            POST_ACTION
        };
        Q_ENUM(Target)

        enum class Command {
            ENABLE,
            DISABLE
        };
        Q_ENUM(Command)
    };

    struct ReadTagHolder {
        QDateTime from, to;
        QList<QDateTime> timestamps;
        QList<QVariant> values;
    };

    struct Download {
        qint64 received = 0;
        qint64 size = 0;
        std::shared_ptr<DownloadSession> session;
        std::shared_ptr<QCryptographicHash> hashfn;
    };

    namespace Protocol {
        const QString UnsubscribeAllCommand = "unsubscribe";;
        const QString BrowseCommand = "browse";
        const QString ReadTagTemplate = "read|%1";
        const QString SubscribeTagTemplate = "subscribe|%1";

        QString ReadTag(const QString &tag);
        QString ReadTagRange(const QString &tag, QDateTime aStart, QDateTime aEnd);
        QString QueryTagRange(const QString &tag);
        QString SubscribeTag(const QString &tag);
        QString WriteTag(const QString& tagName, const QDateTime& timestamp, const QVariant& value);
        QString SwitchSession(const QString &sessionName);
        QString Configuration(ServerConfiguration::Operation operation, ServerConfiguration::Target target, ServerConfiguration::Command command, const QString &targetTag);
        QString FileTransfer(const QString &filename);
    }
};
Q_DECLARE_METATYPE(eDrillingHub::ReadTagHolder)

#endif // EDHPROTOCOL_H
