#include "serverform.h"
#include "widgets.h"
#include "common.h"
#include "config.h"
#include "reportform.h"
#include "net/lanserver.h"

#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>

ServerForm::ServerForm(QWidget *parent)
    : SuiForm(parent)
    , m_minutes(0)
    , m_stopped(false)
{
    setModal(false);
    setTitleText(L("信息提示"));
    setClientSize(728, 583);

    m_lbMsg = new QLabel(L("打字考试服务器正在运行中，目前共有[0]位考生正在测试。"), body());
    m_lbMsg->setGeometry(72, 52, 640, 27);
    m_lbMsg->setFont(uiFont(20, true, L("楷体")));

    QPushButton *btStop = mkButton(body(), L("中止考试，查看结果"), 73, 79, 167, 33);
    QPushButton *btSave = mkButton(body(), L("另存下列考试日志..."), 257, 79, 167, 33);
    QPushButton *btView = mkButton(body(), L("查看所有考试日志"), 441, 79, 167, 33);

    m_log = new QPlainTextEdit(body());
    m_log->setGeometry(16, 125, 697, 409);
    m_log->setReadOnly(true);
    m_log->setFont(uiFont(14));

    QLabel *note = new QLabel(L("注意：如联网考试不正常，请尝试暂时关闭服务端和客户端的软件防火墙。具体问题可咨询本单位网络技术人员。"), body());
    note->setGeometry(16, 540, 697, 42);
    note->setFont(uiFont(20, true, L("楷体")));
    QPalette pal = note->palette();
    pal.setColor(QPalette::WindowText, QColor(0xD0, 0, 0));
    note->setPalette(pal);
    note->setWordWrap(true);

    connect(btStop, SIGNAL(clicked()), this, SLOT(onStop()));
    connect(btSave, SIGNAL(clicked()), this, SLOT(onSaveLog()));
    connect(btView, SIGNAL(clicked()), this, SLOT(onViewLog()));

    m_server = new LanServer(this);
    connect(m_server, SIGNAL(logMessage(QString)), this, SLOT(onLog(QString)));
    connect(m_server, SIGNAL(countChanged(int)), this, SLOT(onCount(int)));

    QString err;
    if (!m_server->start(&err)) {
        suiInfo(this, err);
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTick()));
    m_timer->start();
}

ServerForm::~ServerForm()
{
    if (m_server && m_server->isRunning())
        m_server->stop();
}

void ServerForm::onLog(const QString &msg)
{
    const QString line = QLatin1String("[")
            + QDateTime::currentDateTime().toString(QLatin1String("yyyy-MM-dd HH:mm:ss"))
            + QLatin1String("] ") + msg;
    m_log->appendPlainText(line);

    // 追加写入 server.log
    QFile f(dataDir() + QLatin1String("server.log"));
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream ts(&f);
        ts.setCodec("GBK");
        ts << line << QLatin1String("\r\n");
        ts.flush();
    }
}

void ServerForm::onCount(int n)
{
    m_lbMsg->setText(QString::fromLatin1("%1[%2]%3")
                     .arg(L("打字考试服务器正在运行中，目前共有"))
                     .arg(n)
                     .arg(L("位考生正在测试。")));
}

void ServerForm::onTick()
{
    ++m_minutes;
    Q_UNUSED(m_minutes);
}

void ServerForm::onStop()
{
    if (!suiConfirm(this, L("中止考试后，各客户机将无法进行联网练习和联网测试，您真的要中止考试吗？"),
                    L("信息提示")))
        return;
    if (m_server->clientCount() > 0) {
        if (!suiConfirm(this, L("对不起，目前还有考生正在测试，现在中止考试将造成部分考生成绩丢失，确定要中止考试吗？"),
                        L("信息提示")))
            return;
    }
    finishToReport();
}

void ServerForm::finishToReport()
{
    if (m_stopped)
        return;
    m_stopped = true;
    m_timer->stop();
    m_server->stop();
    ReportForm dlg(this, 0);
    dlg.exec();
    emit finished();
    close();
}

void ServerForm::onSaveLog()
{
    const QString fn = QFileDialog::getSaveFileName(this, L("打字测试服务器日志另存为..."),
                                                    L("打字测试服务器运行日志.txt"),
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
    ts << m_log->toPlainText();
    ts.flush();
    f.close();
    if (suiConfirm(this, L("打字测试服务器日志保存成功，是否立即打开查看？")))
        QDesktopServices::openUrl(QUrl::fromLocalFile(fn));
}

void ServerForm::onViewLog()
{
    const QString path = dataDir() + QLatin1String("server.log");
    if (!QFile::exists(path)) {
        suiInfo(this, L("历次考试服务器运行日志保存在主程序目录，文件名叫server.log，目前还没有日志。"));
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void ServerForm::closeEvent(QCloseEvent *e)
{
    if (!m_stopped) {
        if (!suiConfirm(this, L("中止考试后，各客户机将无法进行联网练习和联网测试，您真的要中止考试吗？"))) {
            e->ignore();
            return;
        }
        m_stopped = true;
        if (m_server)
            m_server->stop();
        emit finished();
    }
    e->accept();
}
