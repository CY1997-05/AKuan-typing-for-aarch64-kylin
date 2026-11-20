#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
#include <QString>
#include <QColor>
#include <QPoint>

class QPushButton;
class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QTimer;
class TypingEngine;

QFont uiFont(int pixelSize, bool bold = false, const QString &family = QString());

QPushButton *mkButton(QWidget *parent, const QString &text, int x, int y, int w, int h, int px = 16);
QLabel *mkLabel(QWidget *parent, const QString &text, int x, int y, int w, int h,
                int px = 15, bool bold = false, const QColor &color = QColor());
QLabel *mkRedLabel(QWidget *parent, const QString &text, int x, int y, int w, int h, int px = 19);
QLabel *mkTitleLabel(QWidget *parent, const QString &text, int x, int y, int w, int h, int px = 24);
QLineEdit *mkEdit(QWidget *parent, int x, int y, int w, int h, int px = 15);
QCheckBox *mkCheck(QWidget *parent, const QString &text, int x, int y, int w, int px = 15);
QComboBox *mkCombo(QWidget *parent, int x, int y, int w, int h, int px = 15);

void suiInfo(QWidget *parent, const QString &text, const QString &title = QString());
bool suiConfirm(QWidget *parent, const QString &text, const QString &title = QString());
bool suiAskPassword(QWidget *parent, const QString &prompt, QString *out);

// 打字区的一行：显示目标文本、已输入字符（正确绿色/错误红色）、光标与输入法预编辑
class TypeLine : public QWidget
{
    Q_OBJECT
public:
    TypeLine(QWidget *parent, TypingEngine *engine, int lineIndex);

    void setLineIndex(int i);
    int lineIndex() const { return m_line; }

signals:
    void charsTyped(const QString &s);
    void backspacePressed();
    void clicked();

protected:
    void paintEvent(QPaintEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void inputMethodEvent(QInputMethodEvent *e);
    QVariant inputMethodQuery(Qt::InputMethodQuery q) const;
    void mousePressEvent(QMouseEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);

private slots:
    void blink();

private:
    bool isActiveLine() const;
    int  charAdvance(const QChar &c) const;
    int     caretCharOffset() const;   // 光标在本行内的字符偏移
    QRect   caretRect() const;         // 光标矩形（控件局部坐标），供输入法定位候选框
    QString typedText() const;         // 本行已经敲入的文字，作为输入法的“周边文本”
    void    notifyCursorMoved();       // 光标移动后通知输入法重新取光标位置

    TypingEngine *m_engine;
    int m_line;
    QString m_preedit;
    QTimer *m_blink;
    bool m_caretOn;
};

#endif // WIDGETS_H
