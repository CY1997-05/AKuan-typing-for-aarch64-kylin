#ifndef RESULTFORM_H
#define RESULTFORM_H

#include "suiform.h"
#include <QString>

// 测试结果窗口（对应原版 fmResult 517x468）
class ResultForm : public SuiForm
{
    Q_OBJECT
public:
    ResultForm(QWidget *parent, const QString &no, const QString &name, const QString &idcard,
               int total, int correct, int wrong, int remain,
               int seconds, int speed, int accuracy, const QString &summary);
};

#endif // RESULTFORM_H
