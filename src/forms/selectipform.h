#ifndef SELECTIPFORM_H
#define SELECTIPFORM_H

#include "suiform.h"
#include <QString>

class QListWidget;
class QPushButton;

// 自动搜索局域网内的考试服务器（对应原版 fmSelectIP 409x413）
class SelectIPForm : public SuiForm
{
    Q_OBJECT
public:
    explicit SelectIPForm(QWidget *parent = 0);
    QString selectedIp() const { return m_ip; }

private slots:
    void doSearch();
    void onOk();

private:
    QListWidget *m_list;
    QPushButton *m_btOk;
    QString m_ip;
};

#endif // SELECTIPFORM_H
