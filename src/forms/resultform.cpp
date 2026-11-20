#include "resultform.h"
#include "widgets.h"
#include "common.h"

#include <QLabel>
#include <QPushButton>
#include <QPainter>


ResultForm::ResultForm(QWidget *parent, const QString &no, const QString &name, const QString &idcard,
                       int total, int correct, int wrong, int remain,
                       int seconds, int speed, int accuracy, const QString &summary)
    : SuiForm(parent)
{
    setTitleText(L("测试结果"));
    setClientSize(517, 468);

    QWidget *panel = new QWidget(body());
    panel->setGeometry(7, 52, 501, 366);
    panel->setStyleSheet(QLatin1String("QWidget { background: #ffffff; border: 1px solid #a0a0a0; }"));

    struct Row { int y; QString label; QString value; };
    QVector<Row> rows;
    {
        Row r;
        r.y = 28;  r.label = L("考    号："); r.value = no;      rows.append(r);
        r.y = 57;  r.label = L("考员姓名："); r.value = name;    rows.append(r);
        r.y = 85;  r.label = L("身份证号："); r.value = idcard;  rows.append(r);
        r.y = 114; r.label = L("总 字 数："); r.value = QString::number(total);   rows.append(r);
        r.y = 142; r.label = L("正确字数："); r.value = QString::number(correct); rows.append(r);
        r.y = 171; r.label = L("错误字数："); r.value = QString::number(wrong);   rows.append(r);
        r.y = 199; r.label = L("剩余字数："); r.value = QString::number(remain);  rows.append(r);
        r.y = 228; r.label = L("所用时间："); r.value = fmtTime(seconds);         rows.append(r);
        r.y = 256; r.label = L("正确速度："); r.value = QString::fromLatin1("%1 %2").arg(speed).arg(L("字/分钟")); rows.append(r);
        r.y = 285; r.label = L("正 确 率："); r.value = QString::fromLatin1("%1%").arg(accuracy); rows.append(r);
    }

    for (int i = 0; i < rows.size(); ++i) {
        QLabel *l = new QLabel(rows.at(i).label + rows.at(i).value, panel);
        l->setGeometry(15, rows.at(i).y, 474, 24);
        l->setFont(uiFont(21, true, L("楷体")));
        l->setStyleSheet(QLatin1String("QLabel { background: transparent; border: none; }"));
    }

    QLabel *sum = new QLabel(summary, panel);
    sum->setGeometry(16, 321, 473, 34);
    sum->setFont(uiFont(21, true, L("楷体")));
    sum->setStyleSheet(QLatin1String("QLabel { background: transparent; border: none; }"));
    sum->setWordWrap(true);

    QPushButton *ok = mkButton(body(), L("确定"), 159, 425, 218, 35);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));

    setModal(true);
}
