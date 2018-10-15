#ifndef EDHPROTOCOL_H
#define EDHPROTOCOL_H

#include <memory>

#include <QString>
#include <QVector>
#include <QVariant>
#include <QDateTime>
#include <QCryptographicHash>

#include "edhtypes.h"

namespace eDrillingHub {
    class DownloadSession;
    class UploadSession;

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
#if (QT_VERSION >= QT_VERSION_CHECK(5,9,0))
        const auto hashing_algorithm = QCryptographicHash::Keccak_512;
#else
        const auto hashing_algorithm = QCryptographicHash::Sha3_512;
#endif

        const QString UnsubscribeAllCommand = "unsubscribe";;
        const QString BrowseCommand = "browse";
        const QString ReadTagTemplate = "read|%1";
        const QString SubscribeTagTemplate = "subscribe|%1";

        QString EXPORT_LIBEDRILLINGHUB_SPEC ReadTag(const QString &tag);
        QString EXPORT_LIBEDRILLINGHUB_SPEC ReadTagRange(const QString &tag, QDateTime aStart, QDateTime aEnd);
        QString EXPORT_LIBEDRILLINGHUB_SPEC QueryTagRange(const QString &tag);
        QString EXPORT_LIBEDRILLINGHUB_SPEC SubscribeTag(const QString &tag);
        QString EXPORT_LIBEDRILLINGHUB_SPEC WriteTag(const QString& tagName, const QDateTime& timestamp, const QVariant& value);
        QString EXPORT_LIBEDRILLINGHUB_SPEC SwitchSession(const QString &sessionName);
        QString EXPORT_LIBEDRILLINGHUB_SPEC Configuration(ServerConfiguration::Operation operation, ServerConfiguration::Target target, ServerConfiguration::Command command, const QString &targetTag);
        QString EXPORT_LIBEDRILLINGHUB_SPEC FileTransfer(const QString &filename);
        QString EXPORT_LIBEDRILLINGHUB_SPEC FileUploadRequest(const QString &filename, qint64 size);
        QString EXPORT_LIBEDRILLINGHUB_SPEC FileUploadTransfer(std::shared_ptr<UploadSession> session, std::function<void(const QByteArray&)> transfer_fn);
    }
};
Q_DECLARE_METATYPE(eDrillingHub::ReadTagHolder)

#endif // EDHPROTOCOL_H
