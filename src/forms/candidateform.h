#ifndef CANDIDATEFORM_H
#define CANDIDATEFORM_H

#include "suiform.h"
#include <QString>

class QTableWidget;
class QLineEdit;
class QPushButton;
class QLabel;

// 考生资料录入/修改（对应原版 fmCandidates 887x519）
class CandidateForm : public SuiForm
{
    Q_OBJECT
public:
    explicit CandidateForm(QWidget *parent = 0);

private slots:
    void reload();
    void onRowChanged();
    void onAdd();
    void onDelete();
    void onDeleteAll();
    void onDelRecord();
    void onDelRecords();
    void onRestoreState();
    void onRestoreAllState();
    void onGenSfz();
    void onGenMultiSfz();
    void onLoadXls();
    void onSaveXls();

private:
    int  currentNo() const;
    QStringList rowValues() const;

    QTableWidget *m_table;
    QLineEdit *m_edId;
    QLineEdit *m_edName;
    QLineEdit *m_edSfz;
    int m_row;
};

#endif // CANDIDATEFORM_H
