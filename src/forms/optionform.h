#ifndef OPTIONFORM_H
#define OPTIONFORM_H

#include "suiform.h"
#include <QString>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QGroupBox;
class QPushButton;
class QRadioButton;

// 测试设置（对应原版 fmOption 889x604）
class OptionForm : public SuiForm
{
    Q_OBJECT
public:
    explicit OptionForm(QWidget *parent = 0);

private slots:
    void onSave();
    void onModeChanged();
    void onCandidates();
    void onGenTestFile();
    void onAutoConvert();
    void onLoadText();
    void onModifyPass();
    void onSearchServer();
    void onNetPracFolder();
    void onDefineImeToggled(bool on);
    void onSpecFolderToggled(bool on);
    void onMultiModeChanged();

private:
    void loadFromConfig();
    void saveToConfig();
    void updateEnabled();

    QCheckBox *m_cbSingle;
    QCheckBox *m_cbClient;
    QCheckBox *m_cbServer;

    QGroupBox *m_paClient;
    QGroupBox *m_paSingleServer;
    QGroupBox *m_paNetPractice;
    QGroupBox *m_paPass;

    QLineEdit *m_edServerIP;
    QLineEdit *m_edTestFile;
    QLineEdit *m_edNetPracFolder;

    QSpinBox  *m_seTestMinutes;
    QSpinBox  *m_seMultiTest;
    QSpinBox  *m_sePracMinutes;

    QRadioButton *m_rbOnce;
    QRadioButton *m_rbLast;
    QRadioButton *m_rbBest;

    QCheckBox *m_cbDefineIME;
    QCheckBox *m_cbRandom;
    QCheckBox *m_cbSpecFolder;
    QCheckBox *m_cbNeedPass;

    QComboBox *m_cbIME;
    QPushButton *m_btNetPracFolder;
    QPushButton *m_btSearchServer;
};

#endif // OPTIONFORM_H
