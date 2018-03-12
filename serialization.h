#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "edhtypes.h"

#include <QRegularExpression>

namespace eDrillingHub {
namespace Tag {
    class Value;
    class ValueName;
}

class Serialization {
public:
    static QString serializeTagQuality(Tag::Quality::Value quality);

    static QString toString(const Tag::Value& tv);
    static QString toString(const Tag::ValueName& tvn);

    static std::tuple<QString, QMetaType::Type> serialize(const QVariant& value);
    static QString serializeScalar(const QVariant& value);

    /**
     * @brief deserializeTagValue - deserilize any valid serialized string, including matrix and vector
     * @param type
     * @param string
     * @return converted value
     */
    static QVariant deserializeTagValue(QMetaType::Type type, const QString &string);
    /**
     * @brief deserializeScalarValue - deserialize single-type values (ie: not matrix and vectors)
     * @param type
     * @param rv
     * @return converted value
     */
    static QVariant deserializeScalarValue(QMetaType::Type type, const QString& rv);

    static const QRegularExpression HashListSplitter;
    static const QRegularExpression CommandSplitter;
private:
    static QStringList deserializeHashList(const QString& rawstring);

    template <typename T>
    static Matrix<T> qStringToEDHMatrix(QStringList& list, bool& ok);

    template <typename T>
    static QVector<T> qStringToQVector(QStringList& list, bool& ok);

    template <typename T>
    static QString edhMatrixToQString(const Matrix<T>& matrix);
    template <typename T>
    static QJsonObject edhMatrixToQJsonObject(const Matrix<T>& matrix);

    template <template<typename> class container, typename type>
    static QString iterableToQString(const container<type> &ct);
    template <typename type, template<typename> class container>
    static QJsonObject iterableToQJsonObject(const container<type> &ct);

    static QMetaType::Type edhMatrixType(const QStringList& list, bool& ok);
    static QMetaType::Type qVectorType(const QStringList& list, bool& ok);
};
}

#endif
