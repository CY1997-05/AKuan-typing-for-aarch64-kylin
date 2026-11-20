#ifndef LANCLIENT_H
#define LANCLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>

class QTcpSocket;

// 联网考试的客户机端：登录取卷、上传成绩、搜索服务器
class LanClient : public QObject
{
    Q_OBJECT
public:
    explicit LanClient(QObject *parent = 0);

    bool login(const QString &no, bool practice,
               QString *name, QString *idcard, int *minutes, QString *text, QString *err);

    bool uploadResult(const QString &no, const QString &name, const QString &idcard,
                      int total, int correct, int wrong, int remain,
                      int seconds, int speed, int accuracy,
                      const QString &fileName, QString *err);

    // 广播搜索局域网内的考试服务器，返回 IP 列表
    static QStringList searchServers(int timeoutMs, QString *err);

private:
    bool connectToServer(QTcpSocket *sock, QString *err);
    bool readLine(QTcpSocket *sock, QString *line, int timeoutMs);
};

#endif // LANCLIENT_H
