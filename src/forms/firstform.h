#ifndef FIRSTFORM_H
#define FIRSTFORM_H

#include "suiform.h"

class QPushButton;

class FirstForm : public SuiForm
{
    Q_OBJECT
public:
    explicit FirstForm(QWidget *parent = 0);

private slots:
    void onOption();
    void onServerMode();
    void onPractice();
    void onNetPractice();
    void onEnterExam();
    void onReport();
    void onReportPractice();
    void onChangeTheme();
    void onAbout();
    void updateButtonsPrivate();

private:
    void updateButtons();
    void startTest(int mode);   // 0 练习 1 联网练习 2 正式考试

    QPushButton *m_btEnter;
    QPushButton *m_btNetPractice;
    QPushButton *m_btPractice;
};

#endif // FIRSTFORM_H
