#pragma once

#include "dataformat.h"

class AsciiFormat : public DataFormat {
public:
    QString name() const override;
    QString inputHint() const override;
    QString toText(const QByteArray &data) const override;
    bool fromText(const QString &text, QByteArray *data, QString *error) const override;
    QValidator::State validate(QString &input, int &pos) const override;
};
