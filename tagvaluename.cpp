#include "tagvaluename.h"

#include <QDebug>

Tag::ValueName::ValueName(const QVariant& value) :
    _tag_value(value)
{}

Tag::ValueName::ValueName(const QString& name, const Tag::Value& value) :
    _name(name),
    _tag_value(value)
{}

QDebug operator<<(QDebug dbg, const Tag::ValueName &tvn) {
    QDebugStateSaver state(dbg);

    dbg.nospace();
    dbg.noquote();
    dbg << tvn.name() << "@" << tvn.value();

    return dbg.maybeSpace();
}
