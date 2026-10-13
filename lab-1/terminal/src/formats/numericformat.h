#pragma once

#include "dataformat.h"

class NumericFormat : public DataFormat {
public:
    // digits - цифр на байт, prefix - необязательный префикс при вводе ("0x").
    NumericFormat(const QString &name, int base, int digits, const QString &prefix,
                  const QString &example);

    QString name() const override;
    QString inputHint() const override;
    QString toText(const QByteArray &data) const override;
    bool fromText(const QString &text, QByteArray *data, QString *error) const override;
    QValidator::State validate(QString &input, int &pos) const override;

private:
    QString m_name;
    int m_base;
    int m_digits;
    QString m_prefix;
    QString m_example;
};
