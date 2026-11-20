#ifndef GENTXTFORM_H
#define GENTXTFORM_H

#include "suiform.h"
#include <QString>

class QSpinBox;
class QCheckBox;
class QPlainTextEdit;
class QPushButton;

// 随机测试文本文件生成器（对应原版 fmGenTxt 699x610）
class GenTxtForm : public SuiForm
{
    Q_OBJECT
public:
    explicit GenTxtForm(QWidget *parent = 0);
    QString savedFile() const { return m_saved; }

private slots:
    void onGen();
    void onSave();
    void onSelectAll();
    void onSelectNone();

private:
    QString buildText();

    QSpinBox *m_seCount;
    QCheckBox *m_cbDigit;
    QCheckBox *m_cbLetter;
    QCheckBox *m_cbSignal;
    QCheckBox *m_cbSingle;
    QCheckBox *m_cbTwo;
    QCheckBox *m_cbThree;
    QCheckBox *m_cbFour;
    QCheckBox *m_cbFive;
    QPlainTextEdit *m_preview;
    QPushButton *m_btSave;
    QString m_saved;
};

#endif // GENTXTFORM_H
