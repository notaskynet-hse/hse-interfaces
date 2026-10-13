#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>

enum class Direction { Rx, Tx };

// Всё, что пришло за один readyRead, или одна отправленная строка.
struct DataChunk {
    Direction direction;
    QByteArray data;
};

class DataHistory : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;

    void append(Direction direction, const QByteArray &data);
    void clear();

    const QList<DataChunk> &chunks() const;

signals:
    void appended(const DataChunk &chunk);
    void cleared();

private:
    QList<DataChunk> m_chunks;
};
