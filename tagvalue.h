#pragma once

#include <QVariant>

namespace eDrillingHub {
namespace Tag {
    class Value {
        Q_GADGET
    public:
        Value() = default;
        Value(const QVariant& value);
        Value(qint64 timestamp, const QVariant& value);

        bool operator==(const Value& other) const {
            return m_unit == other.m_unit &&
                   m_timestamp == other.m_timestamp &&
                   m_value == other.m_value &&
                   m_quality == other.m_quality;
        }

        qint64 timestamp() const { return m_timestamp; }
        void timestamp(qint64 timestamp) { m_timestamp = timestamp; }

        const QVariant& value() const { return m_value; }
        void value(const QVariant& value) { m_value = value; }

        Tag::Quality::Value quality() const { return m_quality; }
        void quality(Tag::Quality::Value quality) { m_quality = quality; }

        const QString& unit() const { return m_unit; }
        void unit(const QString& unit) { m_unit = unit; }

    private:
        qint64 m_timestamp = 0;
        QString m_unit;
        QVariant m_value;
        Tag::Quality::Value m_quality = Tag::Quality::Value::GOOD;

        friend QDataStream& operator>>(QDataStream& s, Value& tv);
        friend QDataStream& operator<<(QDataStream& s, const Value& tv);
    };
}
}
Q_DECLARE_METATYPE(eDrillingHub::Tag::Value)

// QDataStream& operator>>(QDataStream& s, Tag::Value& tv);
// QDataStream& operator<<(QDataStream& s, const Tag::Value& tv);

QDebug operator<<(QDebug dbg, const eDrillingHub::Tag::Value &tv);
