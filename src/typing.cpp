#include "typing.h"

TypingEngine::TypingEngine()
    : m_lineChars(25)
    , m_pos(0)
    , m_correct(0)
    , m_wrong(0)
    , m_maxSeconds(600)
    , m_started(false)
    , m_running(false)
    , m_finished(false)
{
}

void TypingEngine::setLineChars(int n)
{
    if (n > 0)
        m_lineChars = n;
}

void TypingEngine::setText(const QString &text)
{
    m_text = text;
    m_states.fill(0, m_text.size());
    m_typed.fill(QChar(), m_text.size());
    restart();
}

void TypingEngine::restart()
{
    m_pos = 0;
    m_correct = 0;
    m_wrong = 0;
    m_started = false;
    m_running = false;
    m_finished = false;
    m_states.fill(0, m_text.size());
    m_typed.fill(QChar(), m_text.size());
}

void TypingEngine::inputChar(const QChar &c)
{
    if (m_finished || m_pos >= m_text.size())
        return;

    if (!m_started) {
        m_started = true;
        m_running = true;
        m_begin = QDateTime::currentDateTime();
    }

    const bool ok = (c == m_text.at(m_pos));
    if (ok)
        ++m_correct;
    else
        ++m_wrong;

    if (m_pos < m_states.size())
        m_states[m_pos] = ok ? 1 : 2;
    if (m_pos < m_typed.size())
        m_typed[m_pos] = c;
    ++m_pos;

    if (m_pos >= m_text.size())
        finish();
}

void TypingEngine::backspace()
{
    if (m_finished || m_pos <= 0)
        return;
    --m_pos;
    if (m_pos < m_states.size()) {
        if (m_states.at(m_pos) == 1) {
            if (m_correct > 0) --m_correct;
        } else if (m_states.at(m_pos) == 2) {
            if (m_wrong > 0) --m_wrong;
        }
        m_states[m_pos] = 0;
        m_typed[m_pos] = QChar();
    }
}

void TypingEngine::finish()
{
    if (m_finished)
        return;
    m_finished = true;
    m_running = false;
    m_end = QDateTime::currentDateTime();
    if (!m_started) {
        m_started = true;
        m_begin = m_end;
    }
}

void TypingEngine::tick()
{
    if (m_running && elapsed() >= m_maxSeconds)
        finish();
}

int TypingEngine::elapsed() const
{
    if (!m_started)
        return 0;
    const QDateTime e = m_finished ? m_end : QDateTime::currentDateTime();
    const qint64 ms = m_begin.msecsTo(e);
    int sec = static_cast<int>(ms / 1000);
    if (sec < 0) sec = 0;
    return sec;
}

int TypingEngine::speed() const
{
    const int sec = elapsed();
    if (sec < 1)
        return 0;
    return static_cast<int>(m_correct * 60.0 / sec + 0.5);
}

int TypingEngine::accuracy() const
{
    const int n = m_correct + m_wrong;
    if (n <= 0)
        return 0;
    return static_cast<int>(m_correct * 100.0 / n + 0.5);
}

int TypingEngine::lineCount() const
{
    if (m_text.isEmpty())
        return 0;
    return (m_text.size() + m_lineChars - 1) / m_lineChars;
}

int TypingEngine::lineOf(int pos) const
{
    if (m_lineChars <= 0)
        return 0;
    return pos / m_lineChars;
}

int TypingEngine::pageCount() const
{
    const int lines = lineCount();
    return (lines + 5) / 6;
}

QString TypingEngine::lineText(int line) const
{
    const int begin = line * m_lineChars;
    if (begin >= m_text.size())
        return QString();
    return m_text.mid(begin, m_lineChars);
}

int TypingEngine::stateAt(int pos) const
{
    if (pos < 0 || pos >= m_states.size())
        return 0;
    return m_states.at(pos);
}

QChar TypingEngine::typedAt(int pos) const
{
    if (pos < 0 || pos >= m_typed.size())
        return QChar();
    return m_typed.at(pos);
}
