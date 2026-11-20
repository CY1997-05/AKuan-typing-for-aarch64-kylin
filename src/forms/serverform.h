#ifndef SERVERFORM_H
#define SERVERFORM_H

#include "suiform.h"
#include <QString>

class QLabel;
class QPlainTextEdit;
class LanServer;
class QTimer;

// 联网考试服务器界面（对应原版 fmServer 728x583）
class ServerForm : public SuiForm
{
    Q_OBJECT
public:
    explicit ServerForm(QWidget *parent = 0);
    ~ServerForm();

signals:
    void finished();

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void onLog(const QString &msg);
    void onCount(int n);
    void onStop();
    void onSaveLog();
    void onViewLog();
    void onTick();

private:
    void finishToReport();

    QLabel *m_lbMsg;
    QPlainTextEdit *m_log;
    LanServer *m_server;
    QTimer *m_timer;
    int m_minutes;
    bool m_stopped;
};

#endif // SERVERFORM_H
