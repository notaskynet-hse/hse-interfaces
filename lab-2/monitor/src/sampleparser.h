#pragma once

#include <QByteArray>
#include <QObject>

// Разбирает поток от прошивки ("2048\r\n2050\r\n...") на коды АЦП.
class SampleParser : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;

    void feed(const QByteArray &data);
    void reset();

signals:
    void sampleReceived(int code);
    void badLine(const QByteArray &line);
    // Слишком долго нет перевода строки - скорее всего, не та скорость обмена.
    void noLineBreaks();

private:
    QByteArray m_buffer;
    // Порт мог открыться посреди строки: "48" вместо "2048".
    bool m_skipLine = true;
};
