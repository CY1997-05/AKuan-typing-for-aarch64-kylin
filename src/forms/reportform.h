#ifndef REPORTFORM_H
#define REPORTFORM_H

#include "suiform.h"
#include <QString>

class QTableWidget;
class QComboBox;
class QLabel;
class QPushButton;

// 查看测试记录（对应原版 fmReport 769x574）
class ReportForm : public SuiForm
{
    Q_OBJECT
public:
    // mode: 0 测试记录 / 1 联网练习记录
    ReportForm(QWidget *parent, int mode);

private slots:
    void reloadTable();
    void onCandidateChanged();
    void onExportTxt();
    void onExportXls();
    void onExportAllPractice();
    void onClearOne();
    void onClearAll();

private:
    QStringList tableHeader() const;
    QList<QStringList> tableRows() const;
    void buildTable();

    int m_mode;
    QTableWidget *m_table;
    QComboBox *m_combo;
    QLabel *m_lbId;
    QPushButton *m_btClearOne;
    QPushButton *m_btClearAll;
    QPushButton *m_btExportAll;
};

#endif // REPORTFORM_H
