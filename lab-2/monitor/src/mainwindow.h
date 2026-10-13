#pragma once

#include "sampleparser.h"

#include <QMainWindow>
#include <QSerialPort>

class AdcPlot;
class QComboBox;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void openPort(const QString &name, int baudRate);

private slots:
    void refreshPorts();
    void togglePort();
    void handleError(QSerialPort::SerialPortError error);
    void clear();
    void showSample(int code);
    void showBadLine(const QByteArray &line);
    void showNoLineBreaks();

private:
    QString selectedPortName() const;
    void updateControls();

    QSerialPort m_port;
    SampleParser m_parser;
    int m_badLines = 0;

    QComboBox *m_portBox;
    QComboBox *m_baudBox;
    QPushButton *m_refreshButton;
    QPushButton *m_openButton;
    QLabel *m_valueLabel;
    AdcPlot *m_plot;
};
