#pragma once

#include <QObject>
#include "edhmatrix.h"

#if defined EXPORT_LIBEDRILLINGHUB
    #define EXPORT_LIBEDRILLINGHUB_SPEC Q_DECL_EXPORT
#else
    #define EXPORT_LIBEDRILLINGHUB_SPEC Q_DECL_IMPORT
#endif

namespace Tag {
    class EXPORT_LIBEDRILLINGHUB_SPEC Quality {
        Q_GADGET
    public:
        enum class Value {
            GOOD, BAD, LAST_GOOD, DEFAULT
        };
        Q_ENUM(Value)
    };
}

QDataStream& operator>>(QDataStream& s, Tag::Quality::Value& quality);
QDataStream& operator<<(QDataStream& s, Tag::Quality::Value quality);

