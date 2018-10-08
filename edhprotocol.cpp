#include "edhprotocol.h"

#include <QDebug>
#include <QMetaType>
#include <QDateTime>
#include <QMetaEnum>
#include <QRegularExpression>

#include "file_session.h"
#include "serialization.h"

QString eDrillingHub::Protocol::WriteTag(const QString& tagName, const QDateTime& timestamp, const QVariant& value) {
    auto serialized = Serialization::serialize(value);
    return QString("write|%1|%2|%3|%4").arg(
                tagName,
                QString::number(timestamp.toMSecsSinceEpoch()),
                QString::number(std::get<1>(serialized)),
                std::get<0>(serialized));
}

QString eDrillingHub::Protocol::ReadTag(const QString &tag) {
    return ReadTagTemplate.arg(tag);
}

QString eDrillingHub::Protocol::SubscribeTag(const QString &tag) {
    return SubscribeTagTemplate.arg(tag);
}

QString eDrillingHub::Protocol::SwitchSession(const QString &sessionName) {
    return QString("session|switch|%1").arg(sessionName);
}

QString eDrillingHub::Protocol::Configuration(ServerConfiguration::Operation operation, ServerConfiguration::Target target, ServerConfiguration::Command command, const QString &targetTag) {
    QStringList msg;

    msg += "config";
    QString operationString = QMetaEnum::fromType<ServerConfiguration::Operation>().valueToKey(static_cast<int>(operation));
    msg += operationString.remove(ServerConfiguration().getOperationPrefix());
    msg += QMetaEnum::fromType<ServerConfiguration::Target>().valueToKey(static_cast<int>(target));
    msg += QMetaEnum::fromType<ServerConfiguration::Command>().valueToKey(static_cast<int>(command));
    msg += targetTag;

    return msg.join("|");
}

QString eDrillingHub::Protocol::ReadTagRange(const QString &tag, QDateTime aStart, QDateTime aEnd) {
    QString request = QString("read|%1|%2|%3")
        .arg(tag)
        .arg(aStart.toMSecsSinceEpoch())
        .arg(aEnd.toMSecsSinceEpoch());
    return request;
}

QString eDrillingHub::Protocol::FileTransfer(const QString &filename) {
    return QString("file|transfer|%1").arg(filename);
}
QString eDrillingHub::Protocol::FileUploadRequest(const QString &filename, qint64 size) {
    return QString("file|upload|%1|%2").arg(filename, QString::number(size)).toUtf8();
}
QString eDrillingHub::Protocol::FileUploadTransfer(std::shared_ptr<UploadSession> session, std::function<void(const QByteArray&)> transfer_fn) {
    qint64 transferred = 0;
    QByteArray data;
    QCryptographicHash hash(QCryptographicHash::Sha3_512);

    while ((data = session->device()->read(65536)).size() > 0) {
        transferred += data.size();
        transfer_fn(data);
        hash.addData(data);
        session->progress(transferred);
    }

    return QString("file|upload|done|%1").arg(QString(hash.result().toHex())).toUtf8();
}

QString eDrillingHub::Protocol::QueryTagRange(const QString &tag) {
    return QString("db|range|%1").arg(tag);
}
