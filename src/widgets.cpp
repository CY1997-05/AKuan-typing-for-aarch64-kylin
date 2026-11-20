#include "widgets.h"
#include "typing.h"
#include "theme.h"
#include "common.h"

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QFont>
#include <QFontMetrics>
#include <QMessageBox>
#include <QInputDialog>
#include <QPainter>
#include <QKeyEvent>
#include <QInputMethodEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QApplication>
#include <QGuiApplication>
#include <QInputMethod>

QFont uiFont(int pixelSize, bool bold, const QString &family)
{
    QFont f;
    if (family.isEmpty())
        f.setFamily(L("宋体"));
    else
        f.setFamily(family);
    f.setPixelSize(pixelSize);
    f.setBold(bold);
    return f;
}

QPushButton *mkButton(QWidget *parent, const QString &text, int x, int y, int w, int h, int px)
{
    QPushButton *b = new QPushButton(text, parent);
    b->setGeometry(x, y, w, h);
    b->setFont(uiFont(px));
    b->setCursor(Qt::PointingHandCursor);
    b->setFocusPolicy(Qt::NoFocus);
    return b;
}

QLabel *mkLabel(QWidget *parent, const QString &text, int x, int y, int w, int h,
                int px, bool bold, const QColor &color)
{
    QLabel *l = new QLabel(text, parent);
    l->setGeometry(x, y, w, h);
    l->setFont(uiFont(px, bold));
    if (color.isValid()) {
        QPalette pal = l->palette();
        pal.setColor(QPalette::WindowText, color);
        l->setPalette(pal);
    }
    l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    return l;
}

QLabel *mkRedLabel(QWidget *parent, const QString &text, int x, int y, int w, int h, int px)
{
    QLabel *l = new QLabel(text, parent);
    l->setGeometry(x, y, w, h);
    l->setFont(uiFont(px, true, L("楷体")));
    QPalette pal = l->palette();
    pal.setColor(QPalette::WindowText, QColor(0xFF, 0x00, 0x00));
    l->setPalette(pal);
    l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    return l;
}

QLabel *mkTitleLabel(QWidget *parent, const QString &text, int x, int y, int w, int h, int px)
{
    QLabel *l = new QLabel(text, parent);
    l->setGeometry(x, y, w, h);
    l->setFont(uiFont(px, true, L("楷体")));
    l->setAlignment(Qt::AlignCenter);
    return l;
}

QLineEdit *mkEdit(QWidget *parent, int x, int y, int w, int h, int px)
{
    QLineEdit *e = new QLineEdit(parent);
    e->setGeometry(x, y, w, h);
    e->setFont(uiFont(px));
    return e;
}

QCheckBox *mkCheck(QWidget *parent, const QString &text, int x, int y, int w, int px)
{
    QCheckBox *c = new QCheckBox(text, parent);
    c->setGeometry(x, y, w, 22);
    c->setFont(uiFont(px));
    return c;
}

QComboBox *mkCombo(QWidget *parent, int x, int y, int w, int h, int px)
{
    QComboBox *c = new QComboBox(parent);
    c->setGeometry(x, y, w, h);
    c->setFont(uiFont(px));
    return c;
}

void suiInfo(QWidget *parent, const QString &text, const QString &title)
{
    QMessageBox box(parent);
    box.setWindowTitle(title.isEmpty() ? L("信息提示") : title);
    box.setText(text);
    box.setIcon(QMessageBox::Information);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

bool suiConfirm(QWidget *parent, const QString &text, const QString &title)
{
    QMessageBox box(parent);
    box.setWindowTitle(title.isEmpty() ? L("信息提示") : title);
    box.setText(text);
    box.setIcon(QMessageBox::Question);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::Yes);
    return box.exec() == QMessageBox::Yes;
}

bool suiAskPassword(QWidget *parent, const QString &prompt, QString *out)
{
    bool ok = false;
    const QString text = QInputDialog::getText(parent, L("密码管理"), prompt,
                                               QLineEdit::Password, QString(), &ok);
    if (!ok)
        return false;
    if (out)
        *out = text;
    return true;
}

// ---------------------------------------------------------------- TypeLine
// 原版每行是一个 TEdit（bsNone、底色 0,128,192、白字），这里照此绘制：
// 输入条上只显示“已经输入的字”，目标文字由 TestForm 画在输入条上方。

TypeLine::TypeLine(QWidget *parent, TypingEngine *engine, int lineIndex)
    : QWidget(parent)
    , m_engine(engine)
    , m_line(lineIndex)
    , m_caretOn(true)
{
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::IBeamCursor);

    m_blink = new QTimer(this);
    m_blink->setInterval(500);
    connect(m_blink, SIGNAL(timeout()), this, SLOT(blink()));
    m_blink->start();
}

void TypeLine::setLineIndex(int i)
{
    m_line = i;
    m_preedit.clear();
    notifyCursorMoved();
    update();
}

void TypeLine::blink()
{
    m_caretOn = !m_caretOn;
    if (isActiveLine() && hasFocus())
        update();
}

bool TypeLine::isActiveLine() const
{
    if (!m_engine)
        return false;
    if (m_engine->pos() >= m_engine->total())
        return false;
    return m_engine->lineOf(m_engine->pos()) == m_line;
}

int TypeLine::charAdvance(const QChar &c) const
{
    const QFontMetrics fm(uiFont(29));
    if (c.unicode() < 128)
        return fm.width(c);
    return fm.width(QChar(0x56FD));
}

// 光标在本行内的字符偏移（0 = 行首）
int TypeLine::caretCharOffset() const
{
    if (!m_engine)
        return 0;
    const int chars = m_engine->lineChars();
    int off = m_engine->pos() - m_line * chars;
    if (off < 0)
        off = 0;
    if (off > chars)
        off = chars;
    return off;
}

// 光标矩形（控件局部坐标）：输入法据此把候选框摆到光标下面，
// 否则要么不显示候选框，要么把它摆到窗口左上角。
QRect TypeLine::caretRect() const
{
    const int cw = charAdvance(QChar(0x56FD));
    return QRect(2 + caretCharOffset() * cw, 0, 2, height());
}

// 本行已经敲入的文字（输入法的“周边文本”，部分输入法会用它做联想/纠错）
QString TypeLine::typedText() const
{
    if (!m_engine)
        return QString();
    const int chars = m_engine->lineChars();
    const int begin = m_line * chars;
    QString s;
    for (int i = 0; i < chars; ++i) {
        const QChar c = m_engine->typedAt(begin + i);
        if (c.isNull())
            break;
        s.append(c);
    }
    return s;
}

// 光标移动后主动通知输入法重新取光标位置（QLineEdit 这类控件内部会做这件事）。
// 不通知的话，Linux 下输入法的候选框会跟着鼠标跑、或者停在窗口左上角，
// 而不是跟着打字光标走。
void TypeLine::notifyCursorMoved()
{
    QInputMethod *im = QGuiApplication::inputMethod();
    if (im)
        im->update(Qt::ImCursorRectangle);
}

void TypeLine::paintEvent(QPaintEvent *)
{
    const SuiTheme t = Theme::instance()->current();
    QPainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // 输入条底色（与原版 TEdit 的 Color = 0x00C08000 一致）
    p.fillRect(rect(), t.inputBg);

    p.setFont(uiFont(29));
    const QFontMetrics fm(p.font());
    const int y = (height() + fm.ascent() - fm.descent()) / 2;

    const QString text = m_engine ? m_engine->lineText(m_line) : QString();
    const int base = m_line * (m_engine ? m_engine->lineChars() : 0);
    const int cw = charAdvance(QChar(0x56FD));

    int x = 2;
    for (int i = 0; i < text.size(); ++i) {
        const int idx = base + i;
        const int st = m_engine ? m_engine->stateAt(idx) : 0;
        const QChar typed = m_engine ? m_engine->typedAt(idx) : QChar();
        if (st != 0 && !typed.isNull()) {
            p.setPen(st == 1 ? t.inputText : t.inputWrong);
            p.drawText(x, y, QString(typed));
        }
        if (m_engine && idx == m_engine->pos() && isActiveLine() && m_caretOn && hasFocus()) {
            p.setPen(QPen(QColor(0xFF, 0xFF, 0x00), 2));
            p.drawLine(x, 3, x, height() - 3);
        }
        x += cw;
    }

    // 输入法预编辑串（未上屏的拼音/候选）：画在当前光标处，而不是行尾
    if (isActiveLine() && !m_preedit.isEmpty()) {
        const int px = 2 + caretCharOffset() * cw;
        p.setPen(QColor(0xFF, 0xF0, 0x80));
        p.drawText(px, y, m_preedit);
        const int w = fm.width(m_preedit);
        p.drawLine(px, height() - 4, px + w, height() - 4);
    }
}

void TypeLine::keyPressEvent(QKeyEvent *e)
{
    if (!isActiveLine()) {
        QWidget::keyPressEvent(e);
        return;
    }
    if (e->key() == Qt::Key_Backspace) {
        emit backspacePressed();
        notifyCursorMoved();
        return;
    }
    // 回车 / Tab 不是打字内容：ignore() 之后 Qt 会把它往父窗口传，
    // 打字界面用这个特性实现“按回车键开始练习并计时”。
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter || e->key() == Qt::Key_Tab) {
        e->ignore();
        return;
    }
    const QString txt = e->text();
    if (!txt.isEmpty()) {
        bool printable = true;
        for (int i = 0; i < txt.size(); ++i) {
            const QChar c = txt.at(i);
            if (c.isNull() || c.unicode() < 32 || c.unicode() == 127) {
                printable = false;
                break;
            }
        }
        if (printable) {
            emit charsTyped(txt);
            notifyCursorMoved();
            return;
        }
    }
    QWidget::keyPressEvent(e);
}

void TypeLine::inputMethodEvent(QInputMethodEvent *e)
{
    if (!isActiveLine()) {
        e->ignore();
        return;
    }
    // 输入法上屏（选词完毕）→ 当作输入了这些字
    if (!e->commitString().isEmpty()) {
        emit charsTyped(e->commitString());
        notifyCursorMoved();
    }
    // 未上屏的拼音/预编辑串，由 paintEvent 画出来
    m_preedit = e->preeditString();
    update();
    e->accept();
}

// 输入法要问的几件事：能不能用、光标在哪、字体多大、周围有什么字。
// Linux 下（ibus / fcitx 前端）如果不回答 ImCursorRectangle，候选框会乱跑或者干脆
// 不出现；这也是自绘控件能正常用中文输入法的前提。
QVariant TypeLine::inputMethodQuery(Qt::InputMethodQuery q) const
{
    switch (q) {
    case Qt::ImEnabled:
        return true;
    case Qt::ImHints:
        return static_cast<int>(Qt::ImhNone);
    case Qt::ImFont:
        return uiFont(29);
    case Qt::ImCursorRectangle:
        return caretRect();
    case Qt::ImCursorPosition:
    case Qt::ImAnchorPosition:
        return typedText().size();
    case Qt::ImSurroundingText:
        return typedText();
    case Qt::ImCurrentSelection:
        return QString();
    default:
        return QWidget::inputMethodQuery(q);
    }
}

void TypeLine::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    emit clicked();
}

void TypeLine::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    notifyCursorMoved();
    update();
}

void TypeLine::focusOutEvent(QFocusEvent *e)
{
    m_preedit.clear();
    QWidget::focusOutEvent(e);
    update();
}
