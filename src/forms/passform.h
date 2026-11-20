#ifndef PASSFORM_H
#define PASSFORM_H

#include "suiform.h"
#include <QString>

class QLineEdit;
class QLabel;

// 密码输入窗口（对应原版 fmPass 397x204）
class PassForm : public SuiForm
{
    Q_OBJECT
public:
    PassForm(QWidget *parent, const QString &prompt);
    QString password() const;
private slots:
    void onOk();
private:
    QLineEdit *m_edit;
    QLabel *m_prompt;
};

// 管理员权限检查：返回 true 表示有权继续
bool askAdminPassword(QWidget *parent, const QString &prompt = QString());

#endif // PASSFORM_H
