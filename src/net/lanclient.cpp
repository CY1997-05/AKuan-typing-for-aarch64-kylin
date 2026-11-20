#include "lanclient.h"
#include "lanprotocol.h"
#include "../config.h"
#include "../common.h"

#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QTime>
#include <QStringList>
#include <QNetworkInterface>

static QString b64(const QString &s)
{
    return QString::fromLatin1(s.toUtf8().toBase64());
}

static QString unb64(const QString &s)
{
    return QString::fromUtf8(QByteArray::fromBase64(s.toLatin1()));
}

LanClient::LanClient(QObject *parent)
    : QObject(parent)
{
}

bool LanClient::connectToServer(QTcpSocket *sock, QString *err)
{
    AppConfig *cfg = AppConfig::instance();
    QString ip = cfg->serverIp;
    if (ip.isEmpty()) {
        if (err) *err = L("尚未设置考试服务器IP，请先在“测试设置”中设置。");
        return false;
    }
    sock->connectToHost(ip, HZTEST_PORT);
    if (!sock->waitForConnected(3000)) {
        if (err)
            *err = L("向考试服务器连接出错，无法取得考试信息。请检查网络情况及服务器设置。");
        return false;
    }
    return true;
}

bool LanClient::readLine(QTcpSocket *sock, QString *line, int timeoutMs)
{
    QTime t;
    t.start();
    while (t.elapsed() < timeoutMs) {
        if (sock->canReadLine()) {
            const QByteArray raw = sock->readLine();
            QString s = QString::fromUtf8(raw);
            s = s.trimmed();
            *line = s;
            return true;
        }
        if (sock->state() != QAbstractSocket::ConnectedState && sock->bytesAvailable() == 0)
            return false;
        sock->waitForReadyRead(200);
    }
    return false;
}

bool LanClient::login(const QString &no, bool practice,
                      QString *name, QString *idcard, int *minutes, QString *text, QString *err)
{
    QTcpSocket sock;
    if (!connectToServer(&sock, err))
        return false;

    QString req = QLatin1String("LOGIN\t") + no + QLatin1String("\t")
            + (practice ? QLatin1String("practice") : QLatin1String("exam")) + QLatin1Char('\n');
    sock.write(req.toUtf8());
    sock.flush();
    sock.waitForBytesWritten(2000);

    QString line;
    if (!readLine(&sock, &line, 5000)) {
        if (err) *err = L("读取考试服务器响应超时，请检查网络。");
        return false;
    }
    const QStringList parts = line.split(QLatin1Char('\t'));
    if (parts.isEmpty()) {
        if (err) *err = L("考试服务器返回数据格式不正确！");
        return false;
    }
    if (parts.at(0) == QLatin1String("ERR")) {
        if (err)
            *err = (parts.size() > 1) ? unb64(parts.at(1)) : L("服务器拒绝了登录请求！");
        return false;
    }
    if (parts.at(0) != QLatin1String("OK") || parts.size() < 5) {
        if (err) *err = L("考试服务器返回数据不完整！");
        return false;
    }
    if (name) *name = unb64(parts.at(1));
    if (idcard) *idcard = unb64(parts.at(2));
    if (minutes) *minutes = parts.at(3).toInt();
    if (text) *text = unb64(parts.at(4));

    sock.write("QUIT\n");
    sock.flush();
    sock.waitForBytesWritten(500);
    sock.disconnectFromHost();
    return true;
}

bool LanClient::uploadResult(const QString &no, const QString &name, const QString &idcard,
                             int total, int correct, int wrong, int remain,
                             int seconds, int speed, int accuracy,
                             const QString &fileName, QString *err)
{
    QTcpSocket sock;
    if (!connectToServer(&sock, err))
        return false;

    QString req = QLatin1String("RESULT\t") + no
            + QLatin1String("\t") + b64(name)
            + QLatin1String("\t") + b64(idcard)
            + QLatin1String("\t") + QString::number(total)
            + QLatin1String("\t") + QString::number(correct)
            + QLatin1String("\t") + QString::number(wrong)
            + QLatin1String("\t") + QString::number(remain)
            + QLatin1String("\t") + QString::number(seconds)
            + QLatin1String("\t") + QString::number(speed)
            + QLatin1String("\t") + QString::number(accuracy)
            + QLatin1String("\t") + b64(fileName)
            + QLatin1Char('\n');
    sock.write(req.toUtf8());
    sock.flush();
    sock.waitForBytesWritten(3000);

    QString line;
    if (!readLine(&sock, &line, 5000)) {
        if (err) *err = L("向考试服务器上传考试成绩出错！");
        return false;
    }
    if (line.startsWith(QLatin1String("ACK"))) {
        sock.write("QUIT\n");
        sock.flush();
        sock.disconnectFromHost();
        return true;
    }
    if (err) {
        const QStringList parts = line.split(QLatin1Char('\t'));
        *err = (parts.size() > 1) ? unb64(parts.at(1)) : L("向考试服务器上传考试成绩出错！");
    }
    return false;
}

QStringList LanClient::searchServers(int timeoutMs, QString *err)
{
    QStringList found;
    QUdpSocket udp;
    if (!udp.bind(QHostAddress(QHostAddress::AnyIPv4), 0)) {
        if (err) *err = L("无法绑定本地 UDP 端口，搜索失败。");
        return found;
    }

    const QByteArray msg(HZTEST_FIND);
    udp.writeDatagram(msg, QHostAddress::Broadcast, HZTEST_PORT);

    QTime t;
    t.start();
    while (t.elapsed() < timeoutMs) {
        if (!udp.hasPendingDatagrams()) {
            udp.waitForReadyRead(100);
            continue;
        }
        while (udp.hasPendingDatagrams()) {
            QByteArray data;
            data.resize(static_cast<int>(udp.pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort = 0;
            udp.readDatagram(data.data(), data.size(), &sender, &senderPort);
            const QString s = QString::fromUtf8(data).trimmed();
            if (s.startsWith(QLatin1String(HZTEST_SERVER))) {
                const QString ip = sender.toString();
                if (!found.contains(ip))
                    found.append(ip);
            }
        }
    }
    return found;
}
