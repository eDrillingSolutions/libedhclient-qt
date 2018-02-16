#include "edhtypes.h"

#include <QDataStream>

QDataStream &operator>>(QDataStream &s, Tag::Quality::Value &quality) {
    int q;
    s >> q;
    quality = static_cast<Tag::Quality::Value>(q);
    return s;
}


QDataStream &operator<<(QDataStream &s, Tag::Quality::Value quality) {
    s << static_cast<int>(quality);
    return s;
}
