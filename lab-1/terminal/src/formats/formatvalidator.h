#pragma once

#include "dataformat.h"

#include <QValidator>

class FormatValidator : public QValidator {
public:
    using QValidator::QValidator;

    void setFormat(const DataFormat *format) { m_format = format; }

    State validate(QString &input, int &pos) const override {
        return m_format ? m_format->validate(input, pos) : Acceptable;
    }

private:
    const DataFormat *m_format = nullptr;
};
