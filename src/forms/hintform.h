#ifndef HINTFORM_H
#define HINTFORM_H

#include "suiform.h"
#include <QString>

// 信息提示小窗（对应原版 fmHint 641x86）
class HintForm : public SuiForm
{
    Q_OBJECT
public:
    HintForm(QWidget *parent, const QString &text);
};

#endif // HINTFORM_H
