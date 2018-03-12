#ifndef EDHMATRIX_H
#define EDHMATRIX_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QDataStream>

namespace eDrillingHub {
template <typename T>
class Matrix : public QVector<T> {
public:
    Matrix();
    Matrix(const Matrix<T> &other);
    Matrix(const QVector<T> &other, qint32 row, qint32 column);
    Matrix& operator= (const Matrix<T> &other);
    bool operator ==(const Matrix<T>& other) const;

    T& operator()(qint32 row, qint32 column);
    const T& operator()(qint32 row, qint32 column) const;
    void size(qint32 rows, qint32 columns);

    qint32 rows() const;
    qint32 columns() const;

    void appendRows(qint32 rows);
    void appendColumns(qint32 columns, T value);

private:
    qint32 _rows, _columns;
};

template <typename T>
Matrix<T>::Matrix() {
    this->_rows = 0;
    this->_columns = 0;
}

template <typename T>
Matrix<T>::Matrix(const Matrix<T> &other) : QVector<T>(other) {
    this->_rows = other._rows;
    this->_columns = other._columns;
}

template <typename T>
Matrix<T>::Matrix(const QVector<T> &other, qint32 rows, qint32 columns) : QVector<T>(other) {
    this->_rows = rows;
    this->_columns = columns;
}

template <typename T>
Matrix<T>& Matrix<T>::operator= (const Matrix<T> &other) {
    QVector<T>::operator =(other);

    this->_rows = other._rows;
    this->_columns = other._columns;

    return *this;
}

template <typename T>
bool Matrix<T>::operator ==(const Matrix<T> &other) const {
    if (_rows != other._rows || _columns != other._columns ) {
        return false;
    }

    return QVector<T>::operator==(other);
}

template <typename T>
void Matrix<T>::size(qint32 rows, qint32 columns) {
    this->_rows = rows;
    this->_columns = columns;

    QVector<T>::resize(rows * columns);
}

template <typename T>
qint32 Matrix<T>::rows() const {
    return _rows;
}

template <typename T>
qint32 Matrix<T>::columns() const {
    return _columns;
}

template <typename T>
void Matrix<T>::appendRows(qint32 rows) {
    if (_rows >= rows) {
        return;
    }

    _rows = rows;
    quint32 size = _columns * _rows;
    QVector<T>::resize(size);
}

template <typename T>
void Matrix<T>::appendColumns(qint32 columns, T value) {
    if (_columns >= columns) {
        return;
    }

    int i;
    QVector<T>::reserve(columns * _rows);
    quint32 extraColumns = columns - _columns;
    for (i = _rows; i > 0; i--) {
        QVector<T>::insert(i * _columns, extraColumns, value);
    }

    _columns = columns;
}

template <typename T>
T& Matrix<T>::operator()(qint32 row, qint32 column) {
    return QVector<T>::operator [](row * _columns + column);
}
template <typename T>
const T& Matrix<T>::operator()(qint32 row, qint32 column) const {
    return QVector<T>::operator [](row * _columns + column);
}

template<typename T>
QDataStream& operator>>(QDataStream& s, Matrix<T>& v) {
    quint32 columns, rows;

    s >> columns;
    s >> rows;

    v.size(rows, columns);
    for (quint32 column = 0; column < columns; column++) {
        for (quint32 row = 0; row < rows; row++) {
            T t;
            s >> t;
            v(row, column) = t;
        }
    }

    return s;
}

template<typename T>
QDataStream& operator<<(QDataStream& s, const Matrix<T>& v) {
    quint32 columns = v.columns();
    quint32 rows = v.rows();

    s << quint32(columns);
    s << quint32(rows);

    for (quint32 column = 0; column < columns; column++) {
        for (quint32 row = 0; row < rows; row++) {
            s << v(row, column);
        }
    }

    return s;
}
}

Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE(eDrillingHub::Matrix)

#endif // EDHMATRIX_H
