#include "edhprotocol.h"

#include <QDebug>
#include <QMetaType>
#include <QDateTime>
#include <QMetaEnum>
#include <QRegularExpression>

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

QString eDrillingHub::Protocol::FileTransfer(const QString &filename){
    return QString("file|transfer|%1").arg(filename);
}

QString eDrillingHub::Protocol::QueryTagRange(const QString &tag) {
    return QString("db|range|%1").arg(tag);
}
