#include "adcplot.h"

#include "adc.h"

namespace {

constexpr double kWindowSeconds = 10.0;
// Запас над 4095, чтобы кривая не прилипала к рамке.
constexpr double kAxisMaxCode = 4200.0;

} // namespace

AdcPlot::AdcPlot(QWidget *parent)
    : QCustomPlot(parent) {
    addGraph();
    graph(0)->setPen(QPen(QColor(0, 110, 200), 2));

    xAxis->setLabel(tr("Время, с"));
    yAxis->setLabel(tr("Код АЦП"));
    yAxis->setRange(0, kAxisMaxCode);
    yAxis2->setVisible(true);
    yAxis2->setLabel(tr("Напряжение, В"));
    yAxis2->setRange(0, kAxisMaxCode * Adc::kVoltsPerCode);

    m_replotTimer.setInterval(40);
    connect(&m_replotTimer, &QTimer::timeout, this, &AdcPlot::replotIfDirty);
    m_replotTimer.start();

    reset();
}

void AdcPlot::addSample(int code) {
    const double time = m_clock.elapsed() / 1000.0;
    graph(0)->addData(time, code);

    if (time > kWindowSeconds) {
        xAxis->setRange(time, kWindowSeconds, Qt::AlignRight);
        graph(0)->data()->removeBefore(time - kWindowSeconds);
    }
    m_dirty = true;
}

void AdcPlot::reset() {
    graph(0)->data()->clear();
    xAxis->setRange(0, kWindowSeconds);
    m_clock.start();
    m_dirty = false;
    replot();
}

void AdcPlot::replotIfDirty() {
    if (!m_dirty)
        return;
    m_dirty = false;
    replot();
}
