#ifndef TESTFORM_H
#define TESTFORM_H

#include "suiform.h"
#include "typing.h"

#include <QString>
#include <QVector>

class QLabel;
class QPushButton;
class QTimer;
class QWidget;
class TypeLine;

// 打字测试主界面（对应原版 fmMain，ClientSize 756x541）
class TestForm : public SuiForm
{
    Q_OBJECT
public:
    enum Mode { Practice = 0, NetPractice = 1, Exam = 2 };

    TestForm(QWidget *parent, int mode, const QString &no, const QString &name,
             const QString &idcard, const QString &text, int minutes, bool online);

signals:
    void finished();

protected:
    void paintEvent(QPaintEvent *e);
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);

private slots:
    void onStart();
    void onEnd();
    void onLoadDoc();
    void onRandomMenu();
    void onTick();
    void onChars(const QString &s);
    void onBackspace();
    void onLineClicked();

private:
    void setupUi();
    void applyText(const QString &text, const QString &fileName);
    void refreshView();
    void refreshInfo();
    void focusActiveLine();
    void endTest();
    void uploadIfNeeded(int total, int correct, int wrong, int remain,
                        int seconds, int speed, int accuracy);

    int      m_mode;
    QString  m_no;
    QString  m_name;
    QString  m_idcard;
    QString  m_fileName;
    int      m_minutes;
    bool     m_online;
    bool     m_running;
    bool     m_ended;

    TypingEngine m_engine;
    QVector<TypeLine *> m_lines;
    QWidget *m_paInfo;
    QPushButton *m_btStart;
    QPushButton *m_btEnd;
    QTimer  *m_timer;
    QLabel  *m_lbKh;
    QLabel  *m_lbXm;
    QLabel  *m_lbSfz;
    QLabel  *m_lbTime;
    QLabel  *m_lbPage;
    QLabel  *m_lbSpeed;
    QLabel  *m_lbTest;
};

#endif // TESTFORM_H
