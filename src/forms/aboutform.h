#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include "suiform.h"

// 关于窗口（对应原版 fmAbout 528x563）
class AboutForm : public SuiForm
{
    Q_OBJECT
public:
    explicit AboutForm(QWidget *parent = 0);
};

#endif // ABOUTFORM_H
