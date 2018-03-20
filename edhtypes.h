#pragma once

#include <QObject>
#include "edhmatrix.h"

#if defined EXPORT_LIBEDRILLINGHUB
    #define EXPORT_LIBEDRILLINGHUB_SPEC Q_DECL_EXPORT
#else
	#if defined LIBEDRILLINGHUB_DLL
		#define EXPORT_LIBEDRILLINGHUB_SPEC Q_DECL_IMPORT
	#else
		#define EXPORT_LIBEDRILLINGHUB_SPEC
	#endif
#endif

namespace eDrillingHub {
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
}

QDataStream& operator>>(QDataStream& s, eDrillingHub::Tag::Quality::Value& quality);
QDataStream& operator<<(QDataStream& s, eDrillingHub::Tag::Quality::Value quality);

