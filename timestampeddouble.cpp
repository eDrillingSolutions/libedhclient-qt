#include "timestampeddouble.h"

#include <QDebug>
#include <QDateTime>
#include <QDataStream>

QDataStream& operator<<(QDataStream& out, const TimestampedDouble& td) {
    out << td.ts;
    out << td.value;
    return out;
}

QDataStream& operator>>(QDataStream& in, TimestampedDouble& td) {
    in >> td.ts;
    in >> td.value;
    return in;
}

QDebug operator<<(QDebug dbg, const TimestampedDouble& td) {
    QDebugStateSaver state(dbg);

    dbg.nospace();
    dbg.noquote();
    dbg << "TimestampedDouble: " << QDateTime::fromMSecsSinceEpoch(td.ts) << ": " << td.value;

    return dbg.maybeSpace();
}
