#include "serialization.h"

#include <QDebug>
#include <QDateTime>

#include <QMetaEnum>

#include "tagvaluename.h"
#include "timestampeddouble.h"

static QMetaEnum _qualityEnum() {
    static QMetaEnum _enum = QMetaEnum::fromType<Tag::Quality::Value>();
    return _enum;
}

const QRegularExpression Serialization::HashListSplitter("(?<!\\\\)#", QRegularExpression::OptimizeOnFirstUsageOption);
const QRegularExpression Serialization::CommandSplitter("(?<!\\\\)\\|", QRegularExpression::OptimizeOnFirstUsageOption);

static void edhMatrixConvert(QVector<bool>& vector, const QStringList& array) {
    for (const QString& str : array) {
        vector.append(str.toLower() == "true");
    }
}

static void edhMatrixConvert(QVector<double>& vector, const QStringList& array) {
    for (const QString& str : array) {
        vector.append(str.toDouble());
    }
}

static void edhMatrixConvert(QVector<int>& vector, const QStringList& array) {
    for (const QString& str : array) {
        vector.append(str.toInt());
    }
}

static void edhMatrixConvert(QVector<qint64>& vector, const QStringList& array) {
    for (const QString& str : array) {
        vector.append(str.toLongLong());
    }
}

static QString deserializeStringValue(const QString &str) {
    return QString(str).replace("\\|", "|").replace("\\r\\n", "\r\n").replace("\\\\", "\\");
}

static void edhMatrixConvert(QVector<QString>& vector, const QStringList& array) {
    for (const QString& str : array) {
        vector.append(deserializeStringValue(str));
    }
}

static void edhMatrixConvert(QVector<QDateTime>& vector, const QStringList& array) {
    for (const QString& str : array) {
        vector.append(QDateTime::fromMSecsSinceEpoch(str.toLongLong(), Qt::UTC));
    }
}

template <typename T>
QString Serialization::edhMatrixToQString(const Matrix<T>& matrix) {
    QString value;

    qint32 rows = matrix.rows();
    qint32 columns = matrix.columns();
    if (rows > 0 && columns > 0) {
        value.append(QString("EDHMatrix#%1#%2#%3#").arg(rows).arg(columns).arg(qMetaTypeId<T>()));

        for (qint32 i = 0; i < rows; i++) {
            for (qint32 j = 0; j < columns; j++) {
                value.append(serializeScalar(matrix(i, j)).replace('#', "\\#"));
                value.append('#');
            }
        }
        value.remove(value.length() - 1, 1);
    } else {
        value.append(QString("EDHMatrix#0#0#%1#").arg(qMetaTypeId<T>()));
    }

    return value;
}

template <template<typename> class container, typename type>
QString Serialization::iterableToQString(const container<type> &ct) {
    int size = ct.size();
    QString value = QString("Vector#%1#%2#").arg(size).arg(qMetaTypeId<type>());

    switch (size) {
    case 0:
        break;
    case 1:
        value.append(serializeScalar(*ct.begin()).replace('#', "\\#"));
        break;
    default:
        auto last = std::end(ct) - 1;
        std::for_each(std::begin(ct), last, [&] (const type& v) {
            value.append(serializeScalar(v).replace('#', "\\#"));
            value.append('#');
        });
        value.append(serializeScalar(*last).replace('#', "\\#"));
    }

    return value;
}

static QString qMetaTypeToJSONType(int qmetatypeId) {
    switch (qmetatypeId) {
    case QMetaType::Double:
        return "Double";
    case QMetaType::Int:
        return "Int";
    case QMetaType::LongLong:
        return "Int64";
    case QMetaType::Bool:
        return "Bool";
    case QMetaType::QString:
        return "String";
    case QMetaType::QDateTime:
        return "DateTime";
    default:
        qWarning() << Q_FUNC_INFO << "qMetaType" << qmetatypeId << "is unhandled";
        return "";
    }
}

std::tuple<QString, QMetaType::Type> Serialization::serialize(const QVariant& variant) {
    QString value;
    QMetaType::Type metatype = static_cast<QMetaType::Type>(variant.type());
    switch (metatype) {
    case QMetaType::User: {
            // QMetaTypeId is not known at compile-time, so we can't use switch-statements
            int userType = variant.userType();

            if (userType == qMetaTypeId<Matrix<bool>>()) {
                value = edhMatrixToQString(variant.value<Matrix<bool>>());
            } else if (userType == qMetaTypeId<Matrix<int>>()) {
                value = edhMatrixToQString(variant.value<Matrix<int>>());
            } else if (userType == qMetaTypeId<Matrix<qint64>>()) {
                value = edhMatrixToQString(variant.value<Matrix<qint64>>());
            } else if (userType == qMetaTypeId<Matrix<double>>()) {
                value = edhMatrixToQString(variant.value<Matrix<double>>());
            } else if (userType == qMetaTypeId<Matrix<QString>>()) {
                value = edhMatrixToQString(variant.value<Matrix<QString>>());
            } else if (userType == qMetaTypeId<Matrix<QDateTime>>()) {
                value = edhMatrixToQString(variant.value<Matrix<QDateTime>>());
            } else if (userType == qMetaTypeId<QVector<bool>>()) {
                value = iterableToQString(variant.value<QVector<bool>>());
            } else if (userType == qMetaTypeId<QVector<int>>()) {
                value = iterableToQString(variant.value<QVector<int>>());
            } else if (userType == qMetaTypeId<QVector<qint64>>()) {
                value = iterableToQString(variant.value<QVector<qint64>>());
            } else if (userType == qMetaTypeId<QVector<double>>()) {
                value = iterableToQString(variant.value<QVector<double>>());
            } else if (userType == qMetaTypeId<QVector<QString>>()) {
                value = iterableToQString(variant.value<QVector<QString>>());
            } else if (userType == qMetaTypeId<QVector<QDateTime>>()) {
                value = iterableToQString(variant.value<QVector<QDateTime>>());
            } else if (userType == qMetaTypeId<QSet<QString>>()) {
                value = iterableToQString(variant.value<QSet<QString>>());
            } else if (userType == qMetaTypeId<TimestampedDoubles>()) {
                value = QString("TimestampedDoubles");
            } else {
                qWarning() << "Unknown userType, can't serialize type" << variant.typeName();
            }
        }

        break;
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Double:
    case QMetaType::QString:
    case QMetaType::QDateTime:
        value = serializeScalar(variant);
        break;
    case QMetaType::QStringList: {
            value = iterableToQString(static_cast<QList<QString>>(variant.toStringList()));
            metatype = QMetaType::User;
        }
        break;
    case QMetaType::UnknownType:
        break;
    default:
        qWarning() << variant.typeName() << "has no serializer";
        metatype = QMetaType::UnknownType;
        break;
    }

    return std::make_tuple(value, metatype);
}

QString Serialization::serializeTagQuality(Tag::Quality::Value quality) {
    return _qualityEnum().valueToKey(static_cast<int>(quality));
}

QString Serialization::toString(const Tag::Value& tv) {
    QString value;
    QMetaType::Type metatype;
    std::tie(value, metatype) = serialize(tv.value());

    return QString("%1|%2|%3|%4|%5").
            arg(QString::number(tv.timestamp()), QString::number(metatype), value, tv.unit(), serializeTagQuality(tv.quality()));
}

QString Serialization::toString(const Tag::ValueName& tvn) {
    return QString("%1|%2").
            arg(tvn.name(), toString(tvn.tag_value()));
}

QMetaType::Type Serialization::edhMatrixType(const QStringList &list, bool &ok) {
    if (list.size() < 4) {
        ok = false;
        return QMetaType::UnknownType;
    }

    QMetaType::Type elementType = static_cast<QMetaType::Type>(list[3].toInt());
    ok = true;
    return elementType;
}

QMetaType::Type Serialization::qVectorType(const QStringList &list, bool &ok) {
    if (list.size() < 3) {
        ok = false;
        return QMetaType::UnknownType;
    }

    QMetaType::Type elementType = static_cast<QMetaType::Type>(list[2].toInt());
    ok = true;
    return elementType;
}

template <typename T>
Matrix<T> Serialization::qStringToEDHMatrix(QStringList &list, bool &ok) {
    if (list.size() < 4) {
        qWarning() << "Dropping faulty EDHMatrix";
        ok = false;
        return Matrix<T>();
    }

    int rows = list[1].toInt();
    int columns  = list[2].toInt();
    int size = rows * columns;

    if (size == 0) {
        ok = true;
        return Matrix<T>();
    }
    if ((list.size() - 4) != size) {
        qWarning() << Q_FUNC_INFO << "Size does not match" << list.size() << size;
        qWarning() << list;
        ok = false;
        return Matrix<T>();
    }
    list.removeFirst();
    list.removeFirst();
    list.removeFirst();
    list.removeFirst();

    QVector<T> vector;
    vector.reserve(rows * columns);
    edhMatrixConvert(vector, list);

    ok = true;
    return Matrix<T>(vector, rows, columns);
}

template <typename T>
QVector<T> Serialization::qStringToQVector(QStringList &list, bool &ok) {
    if (list.size() < 3) {
        qWarning() << "Dropping faulty QVector";
        ok = false;
        return Matrix<T>();
    }

    int length = list[1].toInt();

    if (length == 0) {
        ok = true;
        return QVector<T>();
    }
    if ((list.size() - 3) != length) {
        qWarning() << Q_FUNC_INFO << "Size does not match" << list.size() << length;
        qWarning() << list;
        ok = false;
        return QVector<T>();
    }
    list.removeFirst();
    list.removeFirst();
    list.removeFirst();

    QVector<T> vector;
    vector.reserve(length);
    edhMatrixConvert(vector, list);

    ok = true;
    return vector;
}

QVariant Serialization::deserializeTagValue(QMetaType::Type type, const QString &string) {
    QVariant variantValue;
    switch (type) {
    case QMetaType::User: {
            int userTypeIdx = string.indexOf('#');
            if (userTypeIdx < 0) {
                qWarning() << Q_FUNC_INFO << "UserType info not found";
                break;
            }

            QStringRef userType = string.leftRef(userTypeIdx);
            if (userType == QStringLiteral("EDHMatrix")) {
                bool ok;
                QStringList list = deserializeHashList(string);
                QMetaType::Type type = edhMatrixType(list, ok);
                if (ok) {
                    switch (type) {
                    case QMetaType::Bool: {
                        Matrix<bool> matrix = qStringToEDHMatrix<bool>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(matrix);
                        }
                    }
                        break;
                    case QMetaType::Int: {
                        Matrix<int> matrix = qStringToEDHMatrix<int>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(matrix);
                        }
                    }
                        break;
                    case QMetaType::LongLong: {
                        Matrix<qint64> matrix = qStringToEDHMatrix<qint64>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(matrix);
                        }
                    }
                        break;
                    case QMetaType::Double: {
                        Matrix<double> matrix = qStringToEDHMatrix<double>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(matrix);
                        }
                    }
                        break;
                    case QMetaType::QString: {
                        Matrix<QString> matrix = qStringToEDHMatrix<QString>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(matrix);
                        }
                    }
                        break;
                    case QMetaType::QDateTime: {
                        Matrix<QDateTime> matrix = qStringToEDHMatrix<QDateTime>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(matrix);
                        }
                    }
                        break;
                    default:
                        qWarning() << "Unsupported EDHMatrix type" << type;
                        break;
                    }
                }
            } else if (userType == QStringLiteral("Vector")) {
                bool ok;
                QStringList list = deserializeHashList(string);
                QMetaType::Type type = qVectorType(list, ok);
                if (ok) {
                    switch (type) {
                    case QMetaType::Bool: {
                        QVector<bool> vector = qStringToQVector<bool>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(vector);
                        }
                    }
                        break;
                    case QMetaType::Int: {
                        QVector<int> vector = qStringToQVector<int>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(vector);
                        }
                    }
                        break;
                    case QMetaType::LongLong: {
                        QVector<qint64> vector = qStringToQVector<qint64>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(vector);
                        }
                    }
                        break;
                    case QMetaType::Double: {
                        QVector<double> vector = qStringToQVector<double>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(vector);
                        }
                    }
                        break;
                    case QMetaType::QString: {
                        QVector<QString> vector = qStringToQVector<QString>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(vector);
                        }
                    }
                        break;
                    case QMetaType::QDateTime: {
                        QVector<QDateTime> vector = qStringToQVector<QDateTime>(list, ok);
                        if (ok) {
                            variantValue = QVariant::fromValue(vector);
                        }
                    }
                        break;
                    default:
                        qWarning() << "Unsupported QVector type" << type;
                        break;
                    }
                }
            }
        }
        break;
    default:
        variantValue = deserializeScalarValue(type, string);
        break;
    }

    return variantValue;
}

QString Serialization::serializeScalar(const QVariant &value) {
    QMetaType::Type metatype = static_cast<QMetaType::Type>(value.type());
    switch (metatype) {
    case QMetaType::QDateTime:
    {
        auto dt = value.toDateTime();
        if (dt.isValid()) {
            return QString::number(value.toDateTime().toMSecsSinceEpoch());
        } else {
            return QStringLiteral("");
        }
    }
    default:
        return value.toString().replace('\\', "\\\\").replace("\r\n", "\\r\\n").replace('|', "\\|");
    }
}

/*
 * To decode the regular expression, check
 * http://www.regular-expressions.info/lookaround.html#lookbehind
 */
QStringList Serialization::deserializeHashList(const QString& rawstring) {
    QStringList array = rawstring.split(HashListSplitter);
    array.replaceInStrings("\\#", "#");
    return array;
}

QVariant Serialization::deserializeScalarValue(QMetaType::Type type, const QString& rv) {
    switch (type) {
    case QMetaType::Double:
        return QVariant(rv.toDouble());
    case QMetaType::Int:
        return QVariant(rv.toInt());
    case QMetaType::LongLong:
        return QVariant(rv.toLongLong());
    case QMetaType::QString:
        return QVariant(deserializeStringValue(rv));
    case QMetaType::Bool:
        return QVariant(rv.toLower() == QString("true"));
    case QMetaType::QDateTime:
        return QVariant(QDateTime::fromMSecsSinceEpoch(rv.toLongLong(), Qt::UTC));
    default:
        qWarning() << Q_FUNC_INFO << "Unknown qVariant Type Conversion" << type;
        return QVariant();
    }
}
