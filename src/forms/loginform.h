#ifndef LOGINFORM_H
#define LOGINFORM_H

#include "suiform.h"
#include <QString>

class QLineEdit;
class QSpinBox;
class QComboBox;
class QLabel;

// 登录窗口（对应原版 fmLogin 310x237）
class LoginForm : public SuiForm
{
    Q_OBJECT
public:
    // mode: 0 本机练习 / 1 联网练习 / 2 正式考试
    LoginForm(QWidget *parent, int mode);

    QString candidateNo() const { return m_no; }
    QString candidateName() const { return m_name; }
    QString candidateId() const { return m_idcard; }
    QString testText() const { return m_text; }
    int     testMinutes() const { return m_minutes; }

private slots:
    void onLogin();

private:
    bool doLocalLogin(QString *err);
    bool doOnlineLogin(QString *err);
    void loadInputMethods();

    int m_mode;
    QString m_no;
    QString m_name;
    QString m_idcard;
    QString m_text;
    int     m_minutes;

    QLineEdit *m_edNumber;
    QSpinBox  *m_spin;
    QComboBox *m_cbIme;
    QLabel    *m_lbTest;
};

#endif // LOGINFORM_H
