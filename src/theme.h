#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QString>

// 配色方案（对应原版“换个界面，换换心情”）
struct SuiTheme
{
    QString name;
    QColor clientTop;      // 窗体背景
    QColor clientBottom;
    QColor border;         // 边框/控件描边
    QColor titleTop;       // 标题栏渐变（上）
    QColor titleBottom;    // 标题栏渐变（下）
    QColor titleText;
    QColor buttonTop;      // 按钮渐变
    QColor buttonBottom;
    QColor buttonBorder;
    QColor text;
    QColor accent;         // 红色强调字（楷体）
    QColor fieldBg;        // 输入框背景
    QColor fieldText;
    QColor articleBg;      // 打字区背景（浅灰）
    QColor articleText;    // 目标文字（黑）
    QColor inputBg;        // 每行输入条背景（原版为 0,128,192）
    QColor inputText;      // 已输入文字（白）
    QColor inputWrong;     // 错误文字
};

class Theme
{
public:
    static Theme *instance();

    int count() const;
    int index() const;
    void setIndex(int i);
    void next();

    SuiTheme current() const;
    QString styleSheet() const;

private:
    Theme();
    static Theme *s_instance;
    int m_index;
};

#endif // THEME_H
