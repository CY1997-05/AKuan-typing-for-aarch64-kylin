#ifndef TYPING_H
#define TYPING_H

#include <QString>
#include <QVector>
#include <QDateTime>

// 打字评分引擎：负责逐字比对、计时、速度与正确率统计
class TypingEngine
{
public:
    TypingEngine();

    void setLineChars(int n);
    void setText(const QString &text);
    void restart();

    // 用户输入
    void inputChar(const QChar &c);
    void backspace();

    // 状态
    bool   isRunning() const { return m_running; }
    bool   isFinished() const { return m_finished; }
    bool   isStarted() const { return m_started; }
    void   finish();
    void   tick();                      // 由界面定时器驱动，用于检测超时

    const QString &text() const { return m_text; }
    int    pos() const { return m_pos; }
    int    correct() const { return m_correct; }
    int    wrong() const { return m_wrong; }
    int    total() const { return m_text.size(); }
    int    remain() const { return m_text.size() - m_pos; }
    int    elapsed() const;             // 秒
    int    speed() const;               // 正确速度 字/分钟
    int    accuracy() const;            // 正确率 %
    int    maxSeconds() const { return m_maxSeconds; }
    void   setMaxSeconds(int s) { m_maxSeconds = s; }

    int    lineChars() const { return m_lineChars; }
    int    lineCount() const;
    int    lineOf(int pos) const;
    int    pageOf(int pos) const { return lineOf(pos) / 6; }
    int    pageCount() const;
    QString lineText(int line) const;

    // 某个位置的输入结果：0 未输入 1 正确 2 错误
    int    stateAt(int pos) const;

    // 用户在该位置实际敲入的字符（用于输入条按实际输入显示）
    QChar  typedAt(int pos) const;

private:
    QString m_text;
    int     m_lineChars;
    int     m_pos;
    int     m_correct;
    int     m_wrong;
    int     m_maxSeconds;
    bool    m_started;
    bool    m_running;
    bool    m_finished;
    QDateTime m_begin;
    QDateTime m_end;
    QVector<int> m_states;   // 每字符 0/1/2
    QVector<QChar> m_typed;  // 每字符实际输入的字符
};

#endif // TYPING_H
