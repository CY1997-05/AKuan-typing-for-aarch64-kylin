#include "candidateform.h"
#include "widgets.h"
#include "config.h"
#include "common.h"
#include "store.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QMessageBox>

CandidateForm::CandidateForm(QWidget *parent)
    : SuiForm(parent)
    , m_row(-1)
{
    setModal(true);
    setTitleText(L("考生资料录入修改"));
    setClientSize(887, 519);

    // 左侧列表
    m_table = new QTableWidget(body());
    m_table->setGeometry(19, 59, 422, 399);
    m_table->setColumnCount(3);
    QStringList heads;
    heads << L("考号") << L("姓名") << L("身份证号");
    m_table->setHorizontalHeaderLabels(heads);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setColumnWidth(0, 87);
    m_table->setColumnWidth(1, 112);
    m_table->setColumnWidth(2, 201);

    mkLabel(body(), L("考   号："), 453, 67, 69, 20);
    mkLabel(body(), L("姓   名："), 453, 95, 69, 20);
    mkLabel(body(), L("身份证号："), 453, 123, 78, 20);

    m_edId = mkEdit(body(), 524, 62, 121, 23);
    m_edId->setMaxLength(10);
    m_edName = mkEdit(body(), 524, 90, 121, 23);
    m_edName->setMaxLength(10);
    m_edSfz = mkEdit(body(), 524, 118, 153, 25);
    m_edSfz->setMaxLength(18);

    QPushButton *btAdd = mkButton(body(), L("添加考生"), 456, 154, 415, 28, 15);
    QPushButton *btDel = mkButton(body(), L("删除此考生"), 456, 188, 415, 28, 15);
    QPushButton *btDelAll = mkButton(body(), L("删除所有考生资料"), 456, 223, 415, 28, 15);
    QPushButton *btDelRec = mkButton(body(), L("清除此考生考试记录（允许重考）"), 456, 257, 415, 28, 15);
    QPushButton *btDelRecs = mkButton(body(), L("清除所有考生考试记录（含考试成绩，慎用！）"), 456, 292, 415, 28, 15);
    QPushButton *btRestore = mkButton(body(), L("恢复此考生登录标志（解决非正常退出无法登录问题）"), 456, 326, 415, 28, 15);
    QPushButton *btRestoreAll = mkButton(body(), L("恢复所有考生登录标志（解决非正常退出无法登录问题）"), 456, 360, 415, 28, 15);
    QPushButton *btLoadXls = mkButton(body(), L("从EXCEL文件批量录入资料"), 456, 395, 415, 28, 15);
    QPushButton *btSaveXls = mkButton(body(), L("导出考生资料至EXCEL文件"), 456, 429, 415, 28, 15);

    QPushButton *btGen = mkButton(body(), L("在此随机生成身份证号"), 687, 117, 184, 27, 15);
    QPushButton *btGenMulti = mkButton(body(), L("随机批量生成身份证号"), 688, 62, 184, 27, 15);

    QPushButton *ok = mkButton(body(), L("保存返回"), 391, 478, 137, 33);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));

    connect(m_table, SIGNAL(itemSelectionChanged()), this, SLOT(onRowChanged()));
    connect(btAdd, SIGNAL(clicked()), this, SLOT(onAdd()));
    connect(btDel, SIGNAL(clicked()), this, SLOT(onDelete()));
    connect(btDelAll, SIGNAL(clicked()), this, SLOT(onDeleteAll()));
    connect(btDelRec, SIGNAL(clicked()), this, SLOT(onDelRecord()));
    connect(btDelRecs, SIGNAL(clicked()), this, SLOT(onDelRecords()));
    connect(btRestore, SIGNAL(clicked()), this, SLOT(onRestoreState()));
    connect(btRestoreAll, SIGNAL(clicked()), this, SLOT(onRestoreAllState()));
    connect(btGen, SIGNAL(clicked()), this, SLOT(onGenSfz()));
    connect(btGenMulti, SIGNAL(clicked()), this, SLOT(onGenMultiSfz()));
    connect(btLoadXls, SIGNAL(clicked()), this, SLOT(onLoadXls()));
    connect(btSaveXls, SIGNAL(clicked()), this, SLOT(onSaveXls()));

    reload();
}

void CandidateForm::reload()
{
    const QList<Candidate> list = Store::instance()->candidates();
    m_table->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i) {
        m_table->setItem(i, 0, new QTableWidgetItem(list.at(i).no));
        m_table->setItem(i, 1, new QTableWidgetItem(list.at(i).name));
        m_table->setItem(i, 2, new QTableWidgetItem(list.at(i).idcard));
    }
}

void CandidateForm::onRowChanged()
{
    m_row = m_table->currentRow();
    if (m_row < 0 || m_row >= m_table->rowCount())
        return;
    m_edId->setText(m_table->item(m_row, 0) ? m_table->item(m_row, 0)->text() : QString());
    m_edName->setText(m_table->item(m_row, 1) ? m_table->item(m_row, 1)->text() : QString());
    m_edSfz->setText(m_table->item(m_row, 2) ? m_table->item(m_row, 2)->text() : QString());
}

int CandidateForm::currentNo() const
{
    if (m_row < 0 || m_row >= m_table->rowCount())
        return -1;
    return m_row;
}

void CandidateForm::onAdd()
{
    Candidate c;
    c.no = m_edId->text().trimmed();
    c.name = m_edName->text().trimmed();
    c.idcard = m_edSfz->text().trimmed();

    QString err;
    if (!Store::instance()->addCandidate(c, &err)) {
        suiInfo(this, err.isEmpty() ? L("添加失败！") : err);
        return;
    }
    reload();
    // 选中新行
    for (int i = 0; i < m_table->rowCount(); ++i) {
        if (m_table->item(i, 0) && m_table->item(i, 0)->text() == c.no) {
            m_table->selectRow(i);
            break;
        }
    }
    m_edId->clear();
    m_edName->clear();
    m_edSfz->clear();
}

void CandidateForm::onDelete()
{
    const int row = currentNo();
    if (row < 0) {
        suiInfo(this, L("请您先在左边的列框中选择要删除的考号！"));
        return;
    }
    const QString no = m_table->item(row, 0)->text();
    if (!suiConfirm(this, L("您真的要删除考号[") + no + L("]的考生资料吗？可同时删除其联网练习记录。")))
        return;
    if (!Store::instance()->removeCandidate(no, true)) {
        suiInfo(this, L("删除失败，请您先在左边的列框中选择要删除的考号！"));
        return;
    }
    m_row = -1;
    reload();
}

void CandidateForm::onDeleteAll()
{
    if (!suiConfirm(this, L("您真的要删除所有考生资料吗？此操作无法恢复！")))
        return;
    Store::instance()->clearCandidates();
    m_row = -1;
    reload();
}

void CandidateForm::onDelRecord()
{
    const int row = currentNo();
    if (row < 0) {
        suiInfo(this, L("请您先在左边的列框中选择要操作的考号！"));
        return;
    }
    const QString no = m_table->item(row, 0)->text();
    if (!suiConfirm(this, L("您真的要清除此考生考试记录吗？")))
        return;
    if (!suiConfirm(this, L("请再次确认：您真的要清除此考生考试记录吗？")))
        return;
    Store::instance()->clearRecords(no);
    suiInfo(this, L("当前考生考试数据成功清除！"));
}

void CandidateForm::onDelRecords()
{
    if (!suiConfirm(this, L("您真的要清除所有考试记录吗？")))
        return;
    if (!suiConfirm(this, L("请再次确认：您真的要清除所有考试记录吗？")))
        return;
    Store::instance()->clearRecords();
    suiInfo(this, L("当前所有考试数据成功清除！"));
}

void CandidateForm::onRestoreState()
{
    const int row = currentNo();
    if (row < 0) {
        suiInfo(this, L("请您先在左边的列框中选择要恢复正在考试标志的考号！"));
        return;
    }
    const QString no = m_table->item(row, 0)->text();
    if (!suiConfirm(this, L("请问您真的要恢复此考生的“正在考试标志”吗？")))
        return;
    Store::instance()->setState(no, 0);
    suiInfo(this, L("登录标志已恢复。"));
}

void CandidateForm::onRestoreAllState()
{
    if (!suiConfirm(this, L("请问您真的要恢复所有考生的“正在考试标志”吗？")))
        return;
    Store::instance()->resetAllStates();
    suiInfo(this, L("所有考生登录标志已恢复。"));
}

static QString randomIdCard()
{
    // 随机生成一个“看起来合理”的 18 位身份证号（仅供测试）
    QString area;
    const char *areas[] = { "110101", "310104", "440305", "510107", "330106", "420102", "610113" };
    area = QLatin1String(areas[qrand() % 7]);

    const int year = 1960 + qrand() % 45;
    const int month = 1 + qrand() % 12;
    const int day = 1 + qrand() % 28;

    QString out = area;
    out += QString::fromLatin1("%1").arg(year, 4, 10, QLatin1Char('0'));
    out += QString::fromLatin1("%1").arg(month, 2, 10, QLatin1Char('0'));
    out += QString::fromLatin1("%1").arg(day, 2, 10, QLatin1Char('0'));
    out += QString::fromLatin1("%1").arg(qrand() % 1000, 3, 10, QLatin1Char('0'));
    out += QLatin1Char(static_cast<char>('0' + qrand() % 10));

    // 校验位
    const int w[17] = { 7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2 };
    const char *check = "10X98765432";
    int sum = 0;
    for (int i = 0; i < 17; ++i)
        sum += (out.at(i).unicode() - '0') * w[i];
    out[17] = QLatin1Char(check[sum % 11]);
    return out;
}

void CandidateForm::onGenSfz()
{
    m_edSfz->setText(randomIdCard());
}

void CandidateForm::onGenMultiSfz()
{
    QString all;
    for (int i = 0; i < 100; ++i)
        all += randomIdCard() + QLatin1String("\r\n");
    m_edSfz->setText(all.trimmed());
    suiInfo(this, L("已批量生成 100 个虚拟身份证号，已复制到剪贴板，可直接粘贴到考生EXCEL文件中再导入。"));
}

void CandidateForm::onSaveXls()
{
    const QString fn = QFileDialog::getSaveFileName(this, L("导出考生资料"), L("考生资料.xls"),
                                                    L("XLS文件(*.xls)"));
    if (fn.isEmpty())
        return;
    QFile f(fn);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        suiInfo(this, L("无法写入文件！"));
        return;
    }
    QXmlStreamWriter x(&f);
    x.setAutoFormatting(true);
    x.writeStartDocument();
    x.writeStartElement(QLatin1String("Workbook"));
    x.writeAttribute(QLatin1String("xmlns"), QLatin1String("urn:schemas-microsoft-com:office:spreadsheet"));
    x.writeStartElement(QLatin1String("Worksheet"));
    x.writeAttribute(QLatin1String("Name"), L("考生资料"));
    x.writeStartElement(QLatin1String("Table"));
    // 表头
    x.writeStartElement(QLatin1String("Row"));
    {
        const char *heads[3] = { "考号", "姓名", "身份证号" };
        for (int i = 0; i < 3; ++i) {
            x.writeStartElement(QLatin1String("Cell"));
            x.writeStartElement(QLatin1String("Data"));
            x.writeAttribute(QLatin1String("Type"), QLatin1String("String"));
            x.writeCharacters(L(heads[i]));
            x.writeEndElement();
            x.writeEndElement();
        }
    }
    x.writeEndElement();

    const QList<Candidate> list = Store::instance()->candidates();
    for (int i = 0; i < list.size(); ++i) {
        x.writeStartElement(QLatin1String("Row"));
        const QString vals[3] = { list.at(i).no, list.at(i).name, list.at(i).idcard };
        for (int k = 0; k < 3; ++k) {
            x.writeStartElement(QLatin1String("Cell"));
            x.writeStartElement(QLatin1String("Data"));
            x.writeAttribute(QLatin1String("Type"), QLatin1String("String"));
            x.writeCharacters(vals[k]);
            x.writeEndElement();
            x.writeEndElement();
        }
        x.writeEndElement();
    }

    x.writeEndElement();
    x.writeEndElement();
    x.writeEndElement();
    x.writeEndDocument();
    f.close();
    suiInfo(this, L("导出成功！"));
}

void CandidateForm::onLoadXls()
{
    const QString fn = QFileDialog::getOpenFileName(this, L("导入考生资料"), QString(),
                                                    L("XLS文件(*.xls);;文本文件(*.txt);;所有文件(*.*)"));
    if (fn.isEmpty())
        return;

    QList<QStringList> rows;
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) {
        suiInfo(this, L("无法读取文件！"));
        return;
    }

    if (fn.endsWith(QLatin1String(".xls"), Qt::CaseInsensitive)) {
        QXmlStreamReader r(&f);
        QStringList cur;
        while (!r.atEnd()) {
            r.readNext();
            if (r.isStartElement() && r.name() == QLatin1String("Row")) {
                cur.clear();
            } else if (r.isStartElement() && r.name() == QLatin1String("Data")) {
                cur.append(r.readElementText());
            } else if (r.isEndElement() && r.name() == QLatin1String("Row")) {
                if (!cur.isEmpty())
                    rows.append(cur);
            }
        }
    } else {
        QTextStream ts(&f);
        ts.setCodec("GBK");
        while (!ts.atEnd()) {
            const QString line = ts.readLine();
            if (line.trimmed().isEmpty())
                continue;
            QStringList parts = line.split(QLatin1Char('\t'));
            if (parts.size() < 3)
                parts = line.split(QLatin1Char(','));
            rows.append(parts);
        }
    }
    f.close();

    if (rows.isEmpty()) {
        suiInfo(this, L("文件中没有可导入的数据！"));
        return;
    }

    // 跳过表头
    int start = 0;
    if (rows.at(0).size() >= 1 && rows.at(0).at(0).trimmed() == L("考号"))
        start = 1;

    int okCount = 0;
    QString errs;
    for (int i = start; i < rows.size(); ++i) {
        const QStringList &cols = rows.at(i);
        if (cols.size() < 3)
            continue;
        Candidate c;
        c.no = cols.at(0).trimmed();
        c.name = cols.at(1).trimmed();
        c.idcard = cols.at(2).trimmed();
        QString err;
        if (Store::instance()->addCandidate(c, &err))
            ++okCount;
        else
            errs += L("第") + QString::number(i + 1) + L("行：") + err + QLatin1String("\n");
    }
    reload();
    QString msg = QString::fromLatin1("%1 %2").arg(L("成功导入考生")).arg(okCount);
    if (!errs.isEmpty())
        msg += QLatin1String("\n") + errs;
    suiInfo(this, msg);
}
