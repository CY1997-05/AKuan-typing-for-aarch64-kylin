#include "aboutform.h"
#include "widgets.h"
#include "common.h"

#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

AboutForm::AboutForm(QWidget *parent)
    : SuiForm(parent)
{
    setModal(true);
    setTitleText(L("关于..."));
    setClientSize(528, 563);

    QWidget *panel = new QWidget(body());
    panel->setGeometry(8, 51, 513, 467);
    panel->setStyleSheet(QLatin1String("QWidget { background: #fbfbfb; border: 1px solid #b8b8b8; }"));

    QTextEdit *memo = new QTextEdit(panel);
    memo->setGeometry(9, 32, 493, 401);
    memo->setReadOnly(true);
    memo->setFont(uiFont(14));
    memo->setStyleSheet(QLatin1String("QTextEdit { background: transparent; border: none; }"));

    QString about = resourceText(QLatin1String(":/data/about.txt"));
    if (about.trimmed().isEmpty()) {
        about = L("中文打字速度测试软件 V1.41\n\n"
                  "本软件用于中文打字速度测试与练习，可自动记录考号、姓名、考试时间、\n"
                  "所用考试文本、正确字数、错误字数、正确速度、正确率等信息。\n\n"
                  "本 Qt 版本由原 Delphi 版 HZTest V1.41 逆向还原。");
    }
    memo->setPlainText(about);

    QLabel *u1 = new QLabel(L("查看更多免费软件"), panel);
    u1->setGeometry(48, 441, 128, 18);
    u1->setStyleSheet(QLatin1String("QLabel { color: #0000cc; text-decoration: underline; background: transparent; border: none; }"));

    QLabel *u2 = new QLabel(L("给我发电子邮件"), panel);
    u2->setGeometry(217, 441, 112, 18);
    u2->setStyleSheet(QLatin1String("QLabel { color: #0000cc; text-decoration: underline; background: transparent; border: none; }"));

    QLabel *u3 = new QLabel(L("向我QQ咨询"), panel);
    u3->setGeometry(376, 441, 80, 18);
    u3->setStyleSheet(QLatin1String("QLabel { color: #0000cc; text-decoration: underline; background: transparent; border: none; }"));

    QPushButton *ok = mkButton(body(), L("返回"), 183, 524, 122, 35);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
}
