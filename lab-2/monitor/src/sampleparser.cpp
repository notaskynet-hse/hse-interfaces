#include "sampleparser.h"

#include "adc.h"

namespace {

// Самая длинная строка от платы - "4095\r\n".
constexpr int kMaxLineLength = 16;

} // namespace

void SampleParser::feed(const QByteArray &data) {
    m_buffer += data;

    qsizetype end;
    while ((end = m_buffer.indexOf('\n')) >= 0) {
        const QByteArray line = m_buffer.left(end).trimmed();
        m_buffer.remove(0, end + 1);

        if (m_skipLine) {
            m_skipLine = false;
            continue;
        }
        if (line.isEmpty())
            continue;

        bool ok = false;
        const int code = line.toInt(&ok);
        if (ok && code >= 0 && code <= Adc::kMaxCode)
            emit sampleReceived(code);
        else
            emit badLine(line.left(kMaxLineLength));
    }

    if (m_buffer.size() > kMaxLineLength) {
        reset();
        emit noLineBreaks();
    }
}

void SampleParser::reset() {
    m_buffer.clear();
    m_skipLine = true;
}
