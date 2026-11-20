#include "reportform.h"
#include "widgets.h"
#include "common.h"
#include "store.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QStringList>
#include <QDateTime>

ReportForm::ReportForm(QWidget *parent, int mode)
    : SuiForm(parent)
    , m_mode(mode)
{
    setModal(true);
    setTitleText(m_mode == 1 ? L("查看联网练习情况") : L("查看测试记录"));
    setClientSize(769, 574);

    m_table = new QTableWidget(body());
    m_table->setGeometry(12, 52, 745, 437);
    m_table->setFont(uiFont(14));
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setColumnCount(m_mode == 1 ? 9 : 12);

    m_lbId = new QLabel(L("请选择您要查看联网练习情况的考生:"), body());
    m_lbId->setGeometry(24, 502, 250, 20);

    m_combo = new QComboBox(body());
    m_combo->setGeometry(280, 498, 185, 23);
    m_combo->setFont(uiFont(15));

    m_btClearOne = mkButton(body(), L("清除此考生练习记录"), 472, 495, 121, 27, 13);
    m_btClearAll = mkButton(body(), L("清除所有考生练习记录"), 608, 495, 137, 27, 13);
    QPushButton *btOk = mkButton(body(), L("返回"), 600, 527, 153, 35);
    QPushButton *btTxt = mkButton(body(), L("输出结果到文本文件"), 23, 527, 153, 35, 15);
    QPushButton *btXls = mkButton(body(), L("输出结果到XLS文件"), 179, 527, 153, 35, 15);
    m_btExportAll = mkButton(body(), L("输出所有考生联网练习情况到XLS"), 336, 527, 257, 35, 15);

    connect(btOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btTxt, SIGNAL(clicked()), this, SLOT(onExportTxt()));
    connect(btXls, SIGNAL(clicked()), this, SLOT(onExportXls()));
    connect(m_btExportAll, SIGNAL(clicked()), this, SLOT(onExportAllPractice()));
    connect(m_btClearOne, SIGNAL(clicked()), this, SLOT(onClearOne()));
    connect(m_btClearAll, SIGNAL(clicked()), this, SLOT(onClearAll()));
    connect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(onCandidateChanged()));

    const bool practice = (m_mode == 1);
    m_lbId->setVisible(practice);
    m_combo->setVisible(practice);
    m_btClearOne->setVisible(practice);
    m_btClearAll->setVisible(practice);
    m_btExportAll->setVisible(practice);

    if (practice) {
        // 填充有练习记录的考生
        const QList<Candidate> cands = Store::instance()->candidates();
        for (int i = 0; i < cands.size(); ++i) {
            if (!Store::instance()->practiceOf(cands.at(i).no).isEmpty())
                m_combo->addItem(cands.at(i).no + QLatin1String(" ") + cands.at(i).name, cands.at(i).no);
        }
    }

    buildTable();
}

QStringList ReportForm::tableHeader() const
{
    QStringList h;
    if (m_mode == 1) {
        h << L("考号") << L("姓名") << L("练习时间") << L("应录入")
          << L("录入正") << L("练习时长") << L("正确速度") << L("正确率") << L("练习文件");
    } else {
        h << L("考号") << L("姓名") << L("身份证号") << L("应录入") << L("录入正")
          << L("录入错") << L("未打完") << L("考试时长") << L("正确速度") << L("正确率")
          << L("考试时间") << L("考试文件");
    }
    return h;
}

QList<QStringList> ReportForm::tableRows() const
{
    QList<QStringList> rows;

    if (m_mode == 1) {
        QString no = m_combo->currentData().toString();
        QList<PracticeRecord> list = no.isEmpty()
                ? Store::instance()->practiceRecords()
                : Store::instance()->practiceOf(no);
        for (int i = 0; i < list.size(); ++i) {
            const PracticeRecord &r = list.at(i);
            QStringList row;
            row << r.no << r.name
                << r.when.toString(QLatin1String("yyyy-MM-dd HH:mm:ss"))
                << QString::number(r.total) << QString::number(r.correct)
                << fmtTime(r.seconds) << QString::number(r.speed)
                << QString::fromLatin1("%1%").arg(r.accuracy) << r.file;
            rows.append(row);
        }
    } else {
        const QList<ExamRecord> list = Store::instance()->records();
        for (int i = 0; i < list.size(); ++i) {
            const ExamRecord &r = list.at(i);
            QStringList row;
            row << r.no << r.name << r.idcard
                << QString::number(r.total) << QString::number(r.correct)
                << QString::number(r.wrong) << QString::number(r.remain)
                << fmtTime(r.seconds) << QString::number(r.speed)
                << QString::fromLatin1("%1%").arg(r.accuracy)
                << r.when.toString(QLatin1String("yyyy-MM-dd HH:mm:ss")) << r.file;
            rows.append(row);
        }
    }
    return rows;
}

void ReportForm::buildTable()
{
    const QStringList h = tableHeader();
    m_table->setHorizontalHeaderLabels(h);

    const QList<QStringList> rows = tableRows();
    m_table->setRowCount(rows.size());
    for (int i = 0; i < rows.size(); ++i) {
        const QStringList &r = rows.at(i);
        for (int c = 0; c < r.size() && c < h.size(); ++c)
            m_table->setItem(i, c, new QTableWidgetItem(r.at(c)));
    }

    if (m_mode == 1) {
        const int widths[9] = { 70, 80, 140, 60, 60, 80, 80, 60, 200 };
        for (int i = 0; i < 9; ++i)
            m_table->setColumnWidth(i, widths[i]);
    } else {
        const int widths[12] = { 70, 80, 160, 60, 60, 60, 60, 80, 80, 60, 140, 220 };
        for (int i = 0; i < 12; ++i)
            m_table->setColumnWidth(i, widths[i]);
    }
}

void ReportForm::reloadTable()
{
    buildTable();
}

void ReportForm::onCandidateChanged()
{
    buildTable();
}

void ReportForm::onExportTxt()
{
    const QString fn = QFileDialog::getSaveFileName(this, L("保存测试结果"), L("打字测试结果.txt"),
                                                    L("文本文件(*.txt);;所有文件(*.*)"));
    if (fn.isEmpty())
        return;
    QFile f(fn);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        suiInfo(this, L("无法写入文件！"));
        return;
    }
    QTextStream ts(&f);
    ts.setCodec("GBK");

    ts << L("打字速度测试或练习结果输出如下：") << QLatin1String("\r\n\r\n");
    ts << tableHeader().join(QLatin1String("\t")) << QLatin1String("\r\n");
    const QList<QStringList> rows = tableRows();
    for (int i = 0; i < rows.size(); ++i)
        ts << rows.at(i).join(QLatin1String("\t")) << QLatin1String("\r\n");
    ts.flush();
    f.close();
    suiInfo(this, L("输出成功！"));
}

void ReportForm::onExportXls()
{
    const QString fn = QFileDialog::getSaveFileName(this, L("保存测试结果"), L("打字测试结果.xls"),
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
    x.writeAttribute(QLatin1String("Name"), L("测试结果"));
    x.writeStartElement(QLatin1String("Table"));

    // 表头
    x.writeStartElement(QLatin1String("Row"));
    const QStringList h = tableHeader();
    for (int i = 0; i < h.size(); ++i) {
        x.writeStartElement(QLatin1String("Cell"));
        x.writeStartElement(QLatin1String("Data"));
        x.writeAttribute(QLatin1String("Type"), QLatin1String("String"));
        x.writeCharacters(h.at(i));
        x.writeEndElement();
        x.writeEndElement();
    }
    x.writeEndElement();

    const QList<QStringList> rows = tableRows();
    for (int i = 0; i < rows.size(); ++i) {
        x.writeStartElement(QLatin1String("Row"));
        const QStringList &r = rows.at(i);
        for (int c = 0; c < r.size(); ++c) {
            x.writeStartElement(QLatin1String("Cell"));
            x.writeStartElement(QLatin1String("Data"));
            x.writeAttribute(QLatin1String("Type"), QLatin1String("String"));
            x.writeCharacters(r.at(c));
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
    suiInfo(this, L("输出成功！"));
}

void ReportForm::onExportAllPractice()
{
    QString dir = QFileDialog::getExistingDirectory(this, L("请选择输出文件夹"));
    if (dir.isEmpty())
        return;

    const QList<Candidate> cands = Store::instance()->candidates();
    int n = 0;
    for (int i = 0; i < cands.size(); ++i) {
        const QList<PracticeRecord> list = Store::instance()->practiceOf(cands.at(i).no);
        if (list.isEmpty())
            continue;
        const QString fn = dir + QLatin1Char('/') + cands.at(i).no + QLatin1String("_")
                + cands.at(i).name + L("_联网练习情况.xls");
        QFile f(fn);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            continue;
        QXmlStreamWriter x(&f);
        x.setAutoFormatting(true);
        x.writeStartDocument();
        x.writeStartElement(QLatin1String("Workbook"));
        x.writeAttribute(QLatin1String("xmlns"), QLatin1String("urn:schemas-microsoft-com:office:spreadsheet"));
        x.writeStartElement(QLatin1String("Worksheet"));
        x.writeAttribute(QLatin1String("Name"), L("联网练习情况"));
        x.writeStartElement(QLatin1String("Table"));
        for (int k = 0; k < list.size(); ++k) {
            x.writeStartElement(QLatin1String("Row"));
            QStringList r;
            r << list.at(k).no << list.at(k).name
              << list.at(k).when.toString(QLatin1String("yyyy-MM-dd HH:mm:ss"))
              << QString::number(list.at(k).total) << QString::number(list.at(k).correct)
              << QString::number(list.at(k).speed)
              << QString::fromLatin1("%1%").arg(list.at(k).accuracy)
              << list.at(k).file;
            for (int c = 0; c < r.size(); ++c) {
                x.writeStartElement(QLatin1String("Cell"));
                x.writeStartElement(QLatin1String("Data"));
                x.writeAttribute(QLatin1String("Type"), QLatin1String("String"));
                x.writeCharacters(r.at(c));
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
        ++n;
    }
    suiInfo(this, QString::fromLatin1("%1 %2").arg(L("已输出")).arg(n));
}

void ReportForm::onClearOne()
{
    const QString no = m_combo->currentData().toString();
    if (no.isEmpty()) {
        suiInfo(this, L("对不起,本机上未发现该考生的联网练习记录!"));
        return;
    }
    if (!suiConfirm(this, L("您确认要清除考生[") + no + L("]的联网练习记录吗?")))
        return;
    Store::instance()->clearPractice(no);
    suiInfo(this, L("清除成功!"));
    buildTable();
}

void ReportForm::onClearAll()
{
    if (!suiConfirm(this, L("您确认要清除所有考生的联网练习记录吗?")))
        return;
    Store::instance()->clearPractice();
    suiInfo(this, L("所有考生的联网练习记录清除完毕!"));
    buildTable();
}
