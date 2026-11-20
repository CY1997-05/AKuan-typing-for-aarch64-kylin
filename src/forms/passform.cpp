#include "passform.h"
#include "widgets.h"
#include "config.h"
#include "common.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

PassForm::PassForm(QWidget *parent, const QString &prompt)
    : SuiForm(parent)
{
    setModal(true);
    setTitleText(L("密码管理"));
    setClientSize(397, 204);

    QWidget *panel = new QWidget(body());
    panel->setGeometry(14, 53, 369, 105);
    panel->setStyleSheet(QLatin1String("QWidget { background: #ffffff; border: 1px solid #a0a0a0; }"));

    m_prompt = new QLabel(prompt.isEmpty() ? L("请输入管理员密码：") : prompt, panel);
    m_prompt->setGeometry(20, 34, 330, 26);
    m_prompt->setFont(uiFont(21, true, L("楷体")));
    m_prompt->setAlignment(Qt::AlignCenter);
    m_prompt->setStyleSheet(QLatin1String("QLabel { background: transparent; border: none; }"));

    m_edit = new QLineEdit(panel);
    m_edit->setGeometry(80, 72, 217, 22);
    m_edit->setEchoMode(QLineEdit::Password);
    m_edit->setFont(uiFont(15));

    QPushButton *ok = mkButton(body(), L("确定"), 94, 165, 89, 29);
    QPushButton *cancel = mkButton(body(), L("取消"), 222, 165, 89, 29);
    connect(ok, SIGNAL(clicked()), this, SLOT(onOk()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(onOk()));
    m_edit->setFocus();
}

QString PassForm::password() const
{
    return m_edit->text();
}

void PassForm::onOk()
{
    if (m_edit->text().isEmpty()) {
        suiInfo(this, L("请输入密码！"));
        return;
    }
    accept();
}

bool askAdminPassword(QWidget *parent, const QString &prompt)
{
    AppConfig *cfg = AppConfig::instance();
    if (!cfg->needPassword)
        return true;

    PassForm dlg(parent, prompt.isEmpty() ? L("请输入管理员密码：") : prompt);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    if (dlg.password() != cfg->adminPassword) {
        suiInfo(parent, L("对不起，密码错误！你无权进行此操作！"));
        return false;
    }
    return true;
}
