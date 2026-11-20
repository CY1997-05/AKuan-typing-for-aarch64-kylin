#include "selectipform.h"
#include "widgets.h"
#include "common.h"
#include "net/lanclient.h"

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QApplication>

SelectIPForm::SelectIPForm(QWidget *parent)
    : SuiForm(parent)
{
    setModal(true);
    setTitleText(L("信息提示"));
    setClientSize(409, 413);

    QLabel *label = new QLabel(
        L("正在从局域网上监听考试服务器IP地址（监听前需要服务器端进入“考试服务器模式），"
          "请等待几秒钟，考试服务器IP会自动出现在下面的列表框中。如监听到多个IP，"
          "请点击选中本次考试的正确IP（可咨询网络技术人员），然后点击“确定”按钮。"
          "如考试服务器已进入服务模式仍无法监听到，可能是服务器与客户机不在同一局域网网段所致。"),
        body());
    label->setGeometry(16, 52, 393, 179);
    label->setFont(uiFont(17, true, L("楷体")));
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_list = new QListWidget(body());
    m_list->setGeometry(19, 239, 373, 121);
    m_list->setFont(uiFont(16));

    m_btOk = mkButton(body(), L("确定"), 133, 370, 156, 33);
    connect(m_btOk, SIGNAL(clicked()), this, SLOT(onOk()));

    QTimer::singleShot(300, this, SLOT(doSearch()));
}

void SelectIPForm::doSearch()
{
    QString err;
    const QStringList ips = LanClient::searchServers(3500, &err);

    m_list->clear();
    for (int i = 0; i < ips.size(); ++i)
        m_list->addItem(ips.at(i));

    if (ips.isEmpty()) {
        m_list->addItem(L("（未监听到考试服务器）"));
    } else {
        m_list->setCurrentRow(0);
    }
}

void SelectIPForm::onOk()
{
    QListWidgetItem *it = m_list->currentItem();
    if (it && it->text().contains(QLatin1Char('.')))
        m_ip = it->text();
    accept();
}
