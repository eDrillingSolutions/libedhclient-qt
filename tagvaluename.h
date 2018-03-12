#pragma once

#include "tagvalue.h"

namespace eDrillingHub {
namespace Tag {
    class ValueName {
    public:
        ValueName() = default;
        ValueName(const QVariant& value);
        ValueName(const QString& name, const Tag::Value& value);

        const QString& name() const { return _name; }
        const Tag::Value& tag_value() const { return _tag_value; }

        qint64 timestamp() const { return _tag_value.timestamp(); }
        const QVariant& value() const { return _tag_value.value(); }
        const QString& unit() const { return _tag_value.unit(); }
        Tag::Quality::Value quality() const { return _tag_value.quality(); }

        void timestamp(qint64 timestamp) { _tag_value.timestamp(timestamp); }
        void value(const QVariant& value) { _tag_value.value(value); }
        void unit(const QString& unit) { _tag_value.unit(unit); }
        void quality(Tag::Quality::Value quality) { _tag_value.quality(quality); }
    private:
        QString _name;
        Tag::Value _tag_value;
    };
}
}
Q_DECLARE_METATYPE(eDrillingHub::Tag::ValueName)

using TagUpdatedValueSlot = std::function<void (const eDrillingHub::Tag::ValueName& tv)>;

QDebug operator<<(QDebug dbg, const eDrillingHub::Tag::ValueName &tvn);
