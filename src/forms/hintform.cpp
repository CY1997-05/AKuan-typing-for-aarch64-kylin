#include "hintform.h"
#include "widgets.h"
#include "common.h"

#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>

HintForm::HintForm(QWidget *parent, const QString &text)
    : SuiForm(parent)
{
    setModal(false);
    setTitleText(L("信息提示"));
    setClientSize(641, 86);

    QLabel *label = new QLabel(text, body());
    label->setGeometry(22, 53, 601, 27);
    label->setFont(uiFont(20, true, L("楷体")));
    label->setAlignment(Qt::AlignCenter);

    // 屏幕居中
    const QRect scr = QApplication::desktop()->availableGeometry();
    move(scr.center().x() - width() / 2, scr.center().y() - height() / 2);
}
