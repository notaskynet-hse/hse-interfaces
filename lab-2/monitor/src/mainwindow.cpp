#include "mainwindow.h"

#include "adc.h"
#include "adcplot.h"

#include <QComboBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle(tr("Монитор АЦП (SPI, MCP3204)"));
    resize(900, 550);

    m_portBox = new QComboBox;
    m_portBox->setMinimumWidth(180);
    // Путь можно ввести вручную: виртуальные порты (pty) в список не попадают.
    m_portBox->setEditable(true);
    m_portBox->setInsertPolicy(QComboBox::NoInsert);
    m_refreshButton = new QPushButton(tr("Обновить"));

    m_baudBox = new QComboBox;
    const QList<qint32> baudRates = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    for (qint32 rate : baudRates)
        m_baudBox->addItem(QString::number(rate), rate);
    m_baudBox->setCurrentIndex(m_baudBox->findData(19200));

    m_openButton = new QPushButton;

    auto *settingsLayout = new QHBoxLayout;
    settingsLayout->addWidget(new QLabel(tr("Порт:")));
    settingsLayout->addWidget(m_portBox);
    settingsLayout->addWidget(m_refreshButton);
    settingsLayout->addWidget(new QLabel(tr("Скорость:")));
    settingsLayout->addWidget(m_baudBox);
    settingsLayout->addWidget(m_openButton);
    settingsLayout->addStretch();

    m_valueLabel = new QLabel;
    m_valueLabel->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    auto *clearButton = new QPushButton(tr("Очистить"));

    auto *valueLayout = new QHBoxLayout;
    valueLayout->addWidget(m_valueLabel);
    valueLayout->addStretch();
    valueLayout->addWidget(clearButton);

    m_plot = new AdcPlot;

    auto *central = new QWidget;
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(settingsLayout);
    mainLayout->addLayout(valueLayout);
    mainLayout->addWidget(m_plot, 1);
    setCentralWidget(central);

    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(m_openButton, &QPushButton::clicked, this, &MainWindow::togglePort);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clear);
    connect(m_portBox, &QComboBox::editTextChanged, this, &MainWindow::updateControls);
    connect(&m_port, &QSerialPort::readyRead, this, [this] { m_parser.feed(m_port.readAll()); });
    connect(&m_port, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(&m_parser, &SampleParser::sampleReceived, this, &MainWindow::showSample);
    connect(&m_parser, &SampleParser::badLine, this, &MainWindow::showBadLine);
    connect(&m_parser, &SampleParser::noLineBreaks, this, &MainWindow::showNoLineBreaks);

    clear();
    refreshPorts();
    updateControls();
}

void MainWindow::openPort(const QString &name, int baudRate) {
    if (m_port.isOpen())
        togglePort();
    m_portBox->setEditText(name);
    const int index = m_baudBox->findData(baudRate);
    if (index >= 0)
        m_baudBox->setCurrentIndex(index);
    togglePort();
}

void MainWindow::refreshPorts() {
    const QString previous = selectedPortName();

    m_portBox->clear();
    const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        QString title = info.portName();
        if (!info.description().isEmpty())
            title += QString(" (%1)").arg(info.description());
        m_portBox->addItem(title, info.portName());
    }

    const int index = m_portBox->findData(previous);
    if (index >= 0)
        m_portBox->setCurrentIndex(index);
    else if (!previous.isEmpty())
        m_portBox->setEditText(previous);

    updateControls();
}

void MainWindow::togglePort() {
    if (m_port.isOpen()) {
        m_port.close();
        statusBar()->showMessage(tr("Порт закрыт"));
        updateControls();
        return;
    }

    m_port.setPortName(selectedPortName());
    m_port.setBaudRate(m_baudBox->currentData().toInt());
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);

    if (m_port.open(QIODevice::ReadOnly)) {
        m_port.clear();
        clear();
        statusBar()->showMessage(
            tr("Открыт %1, %2 бод, 8N1").arg(m_port.portName()).arg(m_port.baudRate()));
    } else {
        statusBar()->showMessage(
            tr("Не удалось открыть %1: %2").arg(m_port.portName(), m_port.errorString()));
    }
    updateControls();
}

void MainWindow::handleError(QSerialPort::SerialPortError error) {
    // ResourceError приходит, когда устройство отключили при открытом порте.
    if (error == QSerialPort::ResourceError) {
        statusBar()->showMessage(tr("Ошибка порта: %1").arg(m_port.errorString()));
        m_port.close();
        updateControls();
    }
}

void MainWindow::clear() {
    m_parser.reset();
    m_plot->reset();
    m_badLines = 0;
    m_valueLabel->setText(tr("Нет данных"));
}

void MainWindow::showSample(int code) {
    m_plot->addSample(code);
    m_valueLabel->setText(tr("Код: %1 (0x%2)   U = %3 В")
                              .arg(code, 4)
                              .arg(QString::number(code, 16).toUpper().rightJustified(4, '0'))
                              .arg(code * Adc::kVoltsPerCode, 0, 'f', 3));
}

void MainWindow::showBadLine(const QByteArray &line) {
    ++m_badLines;
    statusBar()->showMessage(tr("Нераспознанных строк: %1 (последняя: \"%2\")")
                                 .arg(m_badLines)
                                 .arg(QString::fromLatin1(line)));
}

void MainWindow::showNoLineBreaks() {
    ++m_badLines;
    statusBar()->showMessage(tr("В потоке нет переводов строки, проверьте скорость обмена"));
}

QString MainWindow::selectedPortName() const {
    const QString text = m_portBox->currentText();
    const int index = m_portBox->findText(text);
    return index >= 0 ? m_portBox->itemData(index).toString() : text.trimmed();
}

void MainWindow::updateControls() {
    const bool open = m_port.isOpen();
    m_openButton->setText(open ? tr("Закрыть") : tr("Открыть"));
    m_openButton->setEnabled(open || !selectedPortName().isEmpty());
    m_portBox->setEnabled(!open);
    m_baudBox->setEnabled(!open);
    m_refreshButton->setEnabled(!open);
}
