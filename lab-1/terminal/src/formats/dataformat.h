#pragma once

#include <QByteArray>
#include <QString>
#include <QValidator>

// Новый формат регистрируется в FormatRegistry::createDefault().
class DataFormat {
public:
    virtual ~DataFormat() = default;

    virtual QString name() const = 0;
    virtual QString inputHint() const = 0;
    virtual QString toText(const QByteArray &data) const = 0;
    virtual bool fromText(const QString &text, QByteArray *data, QString *error) const = 0;

    // Как QValidator::validate: вызывается на каждое изменение строки ввода.
    virtual QValidator::State validate(QString &input, int &pos) const = 0;
};
