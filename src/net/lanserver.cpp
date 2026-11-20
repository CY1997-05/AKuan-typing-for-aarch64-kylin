#include "lanserver.h"
#include "lanprotocol.h"
#include "../config.h"
#include "../common.h"
#include "../store.h"
#include "../textgen.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QHostInfo>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>

static QString b64(const QString &s)
{
    return QString::fromLatin1(s.toUtf8().toBase64());
}

static QString unb64(const QString &s)
{
    return QString::fromUtf8(QByteArray::fromBase64(s.toLatin1()));
}

LanServer::LanServer(QObject *parent)
    : QObject(parent)
    , m_tcp(0)
    , m_udp(0)
{
}

LanServer::~LanServer()
{
    stop();
}

int LanServer::activeCount() const
{
    int n = 0;
    const QList<Candidate> cs = Store::instance()->candidates();
    for (int i = 0; i < cs.size(); ++i) {
        if (cs.at(i).state != 0)
            ++n;
    }
    return n;
}

bool LanServer::start(QString *err)
{
    if (m_tcp)
        return true;

    m_tcp = new QTcpServer(this);
    if (!m_tcp->listen(QHostAddress::AnyIPv4, HZTEST_PORT)) {
        if (err)
            *err = L("考试服务器启动失败，端口 8849 可能被占用。请尝试关闭防火墙软件或联系技术人员帮忙解决。");
        delete m_tcp;
        m_tcp = 0;
        return false;
    }
    connect(m_tcp, SIGNAL(newConnection()), this, SLOT(onNewConnection()));

    m_udp = new QUdpSocket(this);
    m_udp->bind(QHostAddress::AnyIPv4, HZTEST_PORT, QUdpSocket::ShareAddress);
    connect(m_udp, SIGNAL(readyRead()), this, SLOT(onUdpRead()));

    m_textCache.clear();
    emit logMessage(L("打字速度测试服务器开始运行..."));
    return true;
}

void LanServer::stop()
{
    for (int i = 0; i < m_clients.size(); ++i) {
        m_clients.at(i)->disconnectFromHost();
        m_clients.at(i)->deleteLater();
    }
    m_clients.clear();

    if (m_tcp) {
        m_tcp->close();
        m_tcp->deleteLater();
        m_tcp = 0;
    }
    if (m_udp) {
        m_udp->close();
        m_udp->deleteLater();
        m_udp = 0;
    }
    emit logMessage(L("管理员手动终止考试服务器的运行。"));
}

bool LanServer::isRunning() const
{
    return m_tcp && m_tcp->isListening();
}

QString LanServer::examTextFor(bool practice)
{
    AppConfig *cfg = AppConfig::instance();

    if (practice) {
        if (cfg->practiceUseFolder && !cfg->practiceFolder.isEmpty()) {
            QDir dir(cfg->practiceFolder);
            const QStringList files = dir.entryList(QStringList() << QLatin1String("*.txt"), QDir::Files);
            if (!files.isEmpty()) {
                QFile f(dir.filePath(files.at(qrand() % files.size())));
                if (f.open(QIODevice::ReadOnly)) {
                    QTextStream ts(&f);
                    ts.setCodec("GBK");
                    return ts.readAll();
                }
            }
        }
        if (cfg->practiceRandom)
            return TextGen::buildTestText(1200);
    }

    if (!cfg->testFile.isEmpty()) {
        QFile f(cfg->testFile);
        if (f.open(QIODevice::ReadOnly)) {
            QTextStream ts(&f);
            ts.setCodec("GBK");
            return ts.readAll();
        }
    }
    if (m_textCache.isEmpty())
        m_textCache = TextGen::buildTestText(1500);
    return m_textCache;
}

void LanServer::onUdpRead()
{
    while (m_udp && m_udp->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(static_cast<int>(m_udp->pendingDatagramSize()));
        QHostAddress sender;
        quint16 port = 0;
        m_udp->readDatagram(data.data(), data.size(), &sender, &port);
        if (QString::fromUtf8(data).trimmed() == QLatin1String(HZTEST_FIND)) {
            const QByteArray replyData = (QLatin1String(HZTEST_SERVER) + QLatin1Char(' ')
                                          + QHostInfo::localHostName()).toUtf8();
            m_udp->writeDatagram(replyData, sender, port);
        }
    }
}

void LanServer::onNewConnection()
{
    while (m_tcp && m_tcp->hasPendingConnections()) {
        QTcpSocket *sock = m_tcp->nextPendingConnection();
        m_clients.append(sock);
        connect(sock, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connect(sock, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
        sock->setProperty("practice", false);
        emit countChanged(activeCount());
    }
}

void LanServer::onDisconnected()
{
    QTcpSocket *sock = qobject_cast<QTcpSocket *>(sender());
    if (!sock)
        return;
    m_clients.removeAll(sock);
    sock->deleteLater();
    emit countChanged(activeCount());
}

void LanServer::reply(QTcpSocket *sock, const QString &line)
{
    sock->write(line.toUtf8() + '\n');
    sock->flush();
}

void LanServer::onReadyRead()
{
    QTcpSocket *sock = qobject_cast<QTcpSocket *>(sender());
    if (!sock)
        return;
    while (sock->canReadLine()) {
        const QString line = QString::fromUtf8(sock->readLine()).trimmed();
        if (!line.isEmpty())
            handleLine(sock, line);
    }
}

void LanServer::handleLine(QTcpSocket *sock, const QString &line)
{
    const QStringList f = line.split(QLatin1Char('\t'));
    const QString cmd = f.isEmpty() ? QString() : f.at(0);
    const QString peer = sock->peerAddress().toString();

    if (cmd == QLatin1String("QUIT")) {
        sock->disconnectFromHost();
        return;
    }

    if (cmd == QLatin1String("LOGIN")) {
        if (f.size() < 3) {
            reply(sock, QLatin1String("ERR\t") + b64(L("登录请求格式不正确！")));
            return;
        }
        const QString no = f.at(1);
        const bool practice = (f.at(2) == QLatin1String("practice"));

        if (!isValidCandidateNo(no)) {
            emit logMessage(L("[%1] 的考生申请开始考试或联网练习，但该考号格式不正确").arg(peer));
            reply(sock, QLatin1String("ERR\t") + b64(L("对不起，考号格式不正确！")));
            return;
        }

        Candidate *c = Store::instance()->find(no);
        if (!c) {
            emit logMessage(L("[%1] 的考生申请登录进行联网%2，但服务器上没有此考号。")
                            .arg(peer).arg(practice ? L("练习") : L("考试")));
            reply(sock, QLatin1String("ERR\t") + b64(L("对不起，考试服务器无此考号！请咨询考试管理人员。")));
            return;
        }
        if (c->state != 0) {
            reply(sock, QLatin1String("ERR\t") + b64(L("对不起，此考号正在考试中，不能再重复登录！")));
            return;
        }

        AppConfig *cfg = AppConfig::instance();
        if (!practice) {
            const int times = Store::instance()->examCount(no);
            if (cfg->multiMode == 0 && times >= 1) {
                reply(sock, QLatin1String("ERR\t") + b64(L("对不起，您的测试次数已经超过限制了，不能再考！")));
                return;
            }
            if (cfg->multiMode != 0 && times >= cfg->maxTimes) {
                reply(sock, QLatin1String("ERR\t") + b64(L("对不起，您的测试次数已经超过限制了，不能再考！")));
                return;
            }
        }

        Store::instance()->setState(no, 1);
        sock->setProperty("practice", practice);
        sock->setProperty("no", no);

        const QString text = examTextFor(practice);
        const int minutes = practice ? cfg->practiceMinutes : cfg->testMinutes;
        const QString resp = QLatin1String("OK\t") + b64(c->name) + QLatin1String("\t")
                + b64(c->idcard) + QLatin1String("\t") + QString::number(minutes)
                + QLatin1String("\t") + b64(text);
        reply(sock, resp);

        emit countChanged(activeCount());
        emit logMessage(L("[%1] 上有考号为[%2]的考生申请登录进行联网%3...")
                        .arg(peer).arg(no).arg(practice ? L("练习") : L("考试")));
        return;
    }

    if (cmd == QLatin1String("RESULT")) {
        if (f.size() < 12) {
            reply(sock, QLatin1String("ERR\t") + b64(L("成绩数据不完整，未作保存！")));
            emit logMessage(L("[%1] 发来的打字或联网练习成绩数据不完整，未作保存！").arg(peer));
            return;
        }
        const QString no = f.at(1);
        const QString name = unb64(f.at(2));
        const QString idcard = unb64(f.at(3));
        const int total = f.at(4).toInt();
        const int correct = f.at(5).toInt();
        const int wrong = f.at(6).toInt();
        const int remain = f.at(7).toInt();
        const int seconds = f.at(8).toInt();
        const int speed = f.at(9).toInt();
        const int accuracy = f.at(10).toInt();
        const QString file = unb64(f.at(11));
        const bool practice = sock->property("practice").toBool();

        if (seconds < 60) {
            emit logMessage(L("[%1] 收到的考号为[%2]的成绩，但练习或考试时间小于1分钟，数据未保存。")
                            .arg(peer).arg(no));
            reply(sock, QLatin1String("ERR\t") + b64(L("练习或考试时间小于1分钟，数据未保存。")));
            return;
        }

        if (practice) {
            PracticeRecord r;
            r.no = no;
            r.name = name;
            r.total = total;
            r.correct = correct;
            r.seconds = seconds;
            r.speed = speed;
            r.accuracy = accuracy;
            r.when = QDateTime::currentDateTime();
            r.file = file;
            Store::instance()->addPractice(r);
        } else {
            ExamRecord r;
            r.no = no;
            r.name = name;
            r.idcard = idcard;
            r.total = total;
            r.correct = correct;
            r.wrong = wrong;
            r.remain = remain;
            r.seconds = seconds;
            r.speed = speed;
            r.accuracy = accuracy;
            r.when = QDateTime::currentDateTime();
            r.file = file;
            Store::instance()->addRecord(r);
        }
        Store::instance()->setState(no, 0);
        reply(sock, QLatin1String("ACK"));
        emit countChanged(activeCount());
        emit logMessage(L("[%1] 收到考号为[%2]的%3成绩，已保存。")
                        .arg(peer).arg(no).arg(practice ? L("联网练习") : L("打字")));
        return;
    }

    reply(sock, QLatin1String("ERR\t") + b64(L("未知命令！")));
}
