#include "tagvalue.h"

#include <QDebug>
#include <QDataStream>

Tag::Value::Value(const QVariant& value) :
    m_value(value)
{}

Tag::Value::Value(qint64 timestamp, const QVariant &value) :
    m_timestamp(timestamp),
    m_value(value)
{}

/*
 * It is a DELIBERATE design not to store the tag_name
 *
 * Storing the tag_name nearly doubles the space required
 * to persist the tag - which in any case gets persisted as a key
 */
namespace Tag {
    QDataStream& operator>>(QDataStream& s, Tag::Value &tv) {
        s >> tv.m_timestamp;
        s >> tv.m_value;
        s >> tv.m_unit;
        s >> tv.m_quality;
        return s;
    }

    QDataStream& operator<<(QDataStream& s, const Tag::Value &tv) {
        s << tv.m_timestamp;
        s << tv.m_value;
        s << tv.m_unit;
        s << tv.m_quality;
        return s;
    }
}

inline QDebug operator<<(QDebug dbg, const Tag::Value &tv) {
    QDebugStateSaver state(dbg);

    dbg.nospace();
    dbg.noquote();
    dbg << "Tag::Value: " << tv.timestamp() << ": " << tv.value().toString();

    return dbg.maybeSpace();
}
