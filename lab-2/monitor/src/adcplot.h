#pragma once

#include "qcustomplot.h"

#include <QElapsedTimer>
#include <QTimer>

// Код АЦП за последние несколько секунд. Время отсчитывается от reset().
class AdcPlot : public QCustomPlot {
public:
    explicit AdcPlot(QWidget *parent = nullptr);

    void addSample(int code);
    void reset();

private:
    void replotIfDirty();

    QElapsedTimer m_clock;
    // Перерисовка по таймеру, а не на каждый отсчёт.
    QTimer m_replotTimer;
    bool m_dirty = false;
};
