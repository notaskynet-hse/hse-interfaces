#include "numericformat.h"

#include <QObject>
#include <QRegularExpression>

NumericFormat::NumericFormat(const QString &name, int base, int digits, const QString &prefix,
                             const QString &example)
    : m_name(name)
    , m_base(base)
    , m_digits(digits)
    , m_prefix(prefix)
    , m_example(example) {}

QString NumericFormat::name() const {
    return m_name;
}

QString NumericFormat::inputHint() const {
    return QObject::tr("Байты через пробел, например: %1").arg(m_example);
}

QString NumericFormat::toText(const QByteArray &data) const {
    QString result;
    for (const char c : data) {
        const auto byte = static_cast<unsigned char>(c);
        result += QString::number(byte, m_base).rightJustified(m_digits, '0').toUpper() + ' ';
    }
    return result;
}

bool NumericFormat::fromText(const QString &text, QByteArray *data, QString *error) const {
    static const QRegularExpression separators("\\s+");
    const QStringList tokens = text.split(separators, Qt::SkipEmptyParts);

    QByteArray result;
    for (const QString &token : tokens) {
        QString digits = token;
        if (!m_prefix.isEmpty() && digits.startsWith(m_prefix, Qt::CaseInsensitive))
            digits.remove(0, m_prefix.size());

        bool ok = false;
        const uint value = digits.toUInt(&ok, m_base);
        if (!ok || digits.size() > m_digits || value > 0xFF) {
            *error = QObject::tr("«%1» - не байт в формате %2").arg(token, m_name);
            return false;
        }
        result.append(static_cast<char>(value));
    }

    *data = result;
    return true;
}

// Заполненный байт сам отделяется пробелом: "111e1e" -> "11 1e 1e".
QValidator::State NumericFormat::validate(QString &input, int &pos) const {
    QString result;
    int newPos = -1;
    QString token;
    int digitCount = 0;
    uint value = 0;

    for (int i = 0; i < input.size(); ++i) {
        if (i == pos)
            newPos = result.size();

        const QChar c = input.at(i);
        if (c.isSpace()) {
            result += ' ';
            token.clear();
            digitCount = 0;
            value = 0;
            continue;
        }

        if (m_prefix.size() == 2 && token.compare(m_prefix.left(1), Qt::CaseInsensitive) == 0 &&
            c.toLower() == m_prefix.at(1).toLower()) {
            result += c;
            token += c;
            digitCount = 0;
            value = 0;
            continue;
        }

        bool ok = false;
        const uint digit = QString(c).toUInt(&ok, m_base);
        if (!ok)
            return QValidator::Invalid;

        if (digitCount == m_digits || value * m_base + digit > 0xFF) {
            result += ' ';
            token.clear();
            digitCount = 0;
            value = 0;
        }
        result += c;
        token += c;
        ++digitCount;
        value = value * m_base + digit;
    }

    input = result;
    pos = newPos >= 0 ? newPos : result.size();
    return QValidator::Acceptable;
}
