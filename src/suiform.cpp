#include "suiform.h"
#include "theme.h"

#include <QPainter>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QFont>
#include <QApplication>

SuiForm::SuiForm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
    , m_titleBar(true)
    , m_closeBtn(true)
    , m_minBtn(false)
    , m_dragging(false)
    , m_titleH(28)
{
    setModal(true);
    setAutoFillBackground(false);

    m_body = new QWidget(this);
    m_body->setObjectName(QLatin1String("suiBody"));
    m_body->setAutoFillBackground(false);
    m_body->installEventFilter(this);
}

void SuiForm::setTitleText(const QString &t)
{
    m_title = t;
    setWindowTitle(t);
    update();
}

void SuiForm::setShowTitleBar(bool on)
{
    m_titleBar = on;
    m_titleH = on ? 28 : 0;
    update();
}

void SuiForm::setShowCloseButton(bool on)
{
    m_closeBtn = on;
    update();
}

void SuiForm::setShowMinButton(bool on)
{
    m_minBtn = on;
    update();
}

void SuiForm::setClientSize(int w, int h)
{
    setFixedSize(w, h);
    m_body->setGeometry(0, 0, w, h);
}

void SuiForm::applyTheme()
{
    setStyleSheet(Theme::instance()->styleSheet());
    update();
}

QRect SuiForm::closeRect() const
{
    return QRect(width() - 26, 5, 18, 18);
}

QRect SuiForm::minRect() const
{
    return QRect(width() - 50, 5, 18, 18);
}

void SuiForm::paintEvent(QPaintEvent *)
{
    const SuiTheme t = Theme::instance()->current();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRect outer = rect().adjusted(0, 0, -1, -1);

    // 客户区
    p.setPen(Qt::NoPen);
    p.setBrush(t.clientTop);
    p.drawRoundedRect(outer, 4, 4);

    // 渐变标题栏
    if (m_titleBar) {
        const QRect tr(outer.left(), outer.top(), outer.width(), m_titleH);
        QLinearGradient tg(0, tr.top(), 0, tr.bottom());
        tg.setColorAt(0.0, t.titleTop);
        tg.setColorAt(0.55, t.titleTop.darker(112));
        tg.setColorAt(1.0, t.titleBottom);

        p.save();
        p.setClipRect(tr);
        p.setBrush(tg);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(outer, 4, 4);
        p.restore();

        // 标题文字
        QFont f = font();
        f.setPixelSize(13);
        f.setBold(true);
        p.setFont(f);
        p.setPen(t.titleText);
        p.drawText(tr.adjusted(10, 0, -56, 0), Qt::AlignLeft | Qt::AlignVCenter, m_title);

        // 关闭按钮
        if (m_closeBtn) {
            const QRect cr = closeRect();
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0xC8, 0x3A, 0x2E));
            p.drawRoundedRect(cr, 3, 3);
            p.setPen(QPen(QColor(0xFF, 0xFF, 0xFF), 2));
            p.drawLine(cr.left() + 4, cr.top() + 4, cr.right() - 4, cr.bottom() - 4);
            p.drawLine(cr.right() - 4, cr.top() + 4, cr.left() + 4, cr.bottom() - 4);
        }
        // 最小化按钮
        if (m_minBtn) {
            const QRect mr = minRect();
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0x3A, 0x6E, 0xC8));
            p.drawRoundedRect(mr, 3, 3);
            p.setPen(QPen(QColor(0xFF, 0xFF, 0xFF), 2));
            p.drawLine(mr.left() + 4, mr.center().y() + 1, mr.right() - 4, mr.center().y() + 1);
        }
    }

    // 外框
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(t.border, 1));
    p.drawRoundedRect(outer, 4, 4);
}

void SuiForm::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (m_titleBar && m_closeBtn && closeRect().contains(e->pos())) {
            close();
            return;
        }
        if (m_titleBar && m_minBtn && minRect().contains(e->pos())) {
            showMinimized();
            return;
        }
        if (m_titleBar && e->pos().y() <= m_titleH) {
            m_dragging = true;
            m_dragOffset = e->globalPos() - frameGeometry().topLeft();
            return;
        }
    }
    QDialog::mousePressEvent(e);
}

void SuiForm::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging) {
        move(e->globalPos() - m_dragOffset);
        return;
    }
    QDialog::mouseMoveEvent(e);
}

void SuiForm::mouseReleaseEvent(QMouseEvent *e)
{
    m_dragging = false;
    QDialog::mouseReleaseEvent(e);
}

bool SuiForm::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_body && m_titleBar) {
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent *>(e);
            if (me->button() == Qt::LeftButton && me->pos().y() <= m_titleH) {
                if (m_closeBtn && closeRect().contains(me->pos())) {
                    close();
                    return true;
                }
                if (m_minBtn && minRect().contains(me->pos())) {
                    showMinimized();
                    return true;
                }
                m_dragging = true;
                m_dragOffset = me->globalPos() - frameGeometry().topLeft();
                return true;
            }
        } else if (e->type() == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent *>(e);
            if (m_dragging) {
                move(me->globalPos() - m_dragOffset);
                return true;
            }
        } else if (e->type() == QEvent::MouseButtonRelease) {
            if (m_dragging) {
                m_dragging = false;
                return true;
            }
        }
    }
    return QDialog::eventFilter(o, e);
}
