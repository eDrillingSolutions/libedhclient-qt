#pragma once

#include <QVector>
#include <QtGlobal>
#include <QMetaType>

struct TimestampedDouble {
    qint64 ts;
    double value;

    bool operator==(const TimestampedDouble& other) const {
        return (ts == other.ts) && (value == other.value);
    }
};
using TimestampedDoubles = QVector<TimestampedDouble>;
QDataStream &operator<<(QDataStream &out, const TimestampedDouble &td);
QDataStream &operator>>(QDataStream &in, TimestampedDouble &td);
Q_DECLARE_METATYPE(TimestampedDouble)
Q_DECLARE_METATYPE(TimestampedDoubles)

QDebug operator<<(QDebug dbg, const TimestampedDouble &tvn);
