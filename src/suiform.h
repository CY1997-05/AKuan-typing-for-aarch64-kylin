#ifndef SUIFORM_H
#define SUIFORM_H

#include <QDialog>
#include <QPoint>
#include <QString>

// 仿原版皮肤窗口：无系统边框 + 自绘渐变标题栏（可拖动、最小化、关闭）。
// 客户区坐标与原版 DFM 完全一致（标题栏画在客户区顶部 0~28px）。
class SuiForm : public QDialog
{
    Q_OBJECT
public:
    explicit SuiForm(QWidget *parent = 0);

    void setTitleText(const QString &t);
    QString titleText() const { return m_title; }

    void setShowTitleBar(bool on);
    void setShowCloseButton(bool on);
    void setShowMinButton(bool on);

    // 客户区尺寸（含自绘标题栏），与 DFM 的 ClientWidth/ClientHeight 一致
    void setClientSize(int w, int h);

    QWidget *body() const { return m_body; }

    void applyTheme();

signals:
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool eventFilter(QObject *o, QEvent *e);

private:
    QRect closeRect() const;
    QRect minRect() const;

    QString m_title;
    bool    m_titleBar;
    bool    m_closeBtn;
    bool    m_minBtn;
    bool    m_dragging;
    QPoint  m_dragOffset;
    QWidget *m_body;
    int     m_titleH;
};

#endif // SUIFORM_H
