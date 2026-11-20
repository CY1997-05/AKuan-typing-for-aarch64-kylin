#ifndef LANSERVER_H
#define LANSERVER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QByteArray>

class QTcpServer;
class QTcpSocket;
class QUdpSocket;

// 联网考试的考试服务器端
class LanServer : public QObject
{
    Q_OBJECT
public:
    explicit LanServer(QObject *parent = 0);
    ~LanServer();

    bool start(QString *err);
    void stop();
    bool isRunning() const;
    int  clientCount() const { return m_clients.size(); }

signals:
    void logMessage(const QString &msg);
    void countChanged(int n);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void onUdpRead();

private:
    void handleLine(QTcpSocket *sock, const QString &line);
    void reply(QTcpSocket *sock, const QString &line);
    QString examTextFor(bool practice);
    int activeCount() const;

    QTcpServer *m_tcp;
    QUdpSocket *m_udp;
    QList<QTcpSocket *> m_clients;
    QString m_textCache;
};

#endif // LANSERVER_H
