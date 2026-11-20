#include "gentxtform.h"
#include "widgets.h"
#include "common.h"
#include "textgen.h"

#include <QSpinBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QVector>

GenTxtForm::GenTxtForm(QWidget *parent)
    : SuiForm(parent)
{
    setModal(true);
    setTitleText(L("随机测试文本文件生成器"));
    setClientSize(699, 610);

    QGroupBox *opt = new QGroupBox(L("随机生成选项"), body());
    opt->setGeometry(16, 54, 665, 155);
    opt->setFont(uiFont(14));

    mkLabel(opt, L("生成汉字数量："), 15, 34, 110, 20);
    m_seCount = new QSpinBox(opt);
    m_seCount->setGeometry(130, 30, 71, 25);
    m_seCount->setRange(50, 20000);
    m_seCount->setValue(1000);
    mkLabel(opt, L("个"), 210, 34, 30, 20);

    m_cbDigit = mkCheck(opt, L("包含全角数字"), 16, 62, 113);
    m_cbLetter = mkCheck(opt, L("包含全角字母"), 136, 62, 113);
    m_cbSignal = mkCheck(opt, L("包含全角标点和符号"), 264, 62, 158);
    m_cbSingle = mkCheck(opt, L("包含随机生成的单个汉字"), 440, 61, 200);
    m_cbTwo = mkCheck(opt, L("包含两字词语"), 16, 94, 113);
    m_cbThree = mkCheck(opt, L("包含三字词语"), 136, 94, 113);
    m_cbFour = mkCheck(opt, L("包含四字词语"), 264, 94, 113);
    m_cbFive = mkCheck(opt, L("包含五字以上词语"), 440, 93, 160);

    QPushButton *btAll = mkButton(opt, L("以上全部选中"), 170, 124, 156, 23, 15);
    QPushButton *btNone = mkButton(opt, L("以上全部不选"), 353, 124, 156, 23, 15);

    QPushButton *btGen = mkButton(body(), L("点此立即按以上选项生成"), 162, 216, 383, 31);

    QGroupBox *paPreview = new QGroupBox(L("生成文本预览"), body());
    paPreview->setGeometry(16, 254, 665, 297);
    paPreview->setFont(uiFont(14));

    m_preview = new QPlainTextEdit(paPreview);
    m_preview->setGeometry(8, 27, 649, 262);
    m_preview->setReadOnly(true);
    m_preview->setFont(uiFont(15));

    QPushButton *btSave = mkButton(body(), L("保存为文件..."), 130, 566, 151, 31);
    QPushButton *btReturn = mkButton(body(), L("返回"), 432, 566, 145, 31);
    m_btSave = btSave;

    connect(btGen, SIGNAL(clicked()), this, SLOT(onGen()));
    connect(btSave, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(btReturn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btAll, SIGNAL(clicked()), this, SLOT(onSelectAll()));
    connect(btNone, SIGNAL(clicked()), this, SLOT(onSelectNone()));

    // 默认选中“随机单个汉字”
    m_cbSingle->setChecked(true);
}

void GenTxtForm::onSelectAll()
{
    m_cbDigit->setChecked(true);
    m_cbLetter->setChecked(true);
    m_cbSignal->setChecked(true);
    m_cbSingle->setChecked(true);
    m_cbTwo->setChecked(true);
    m_cbThree->setChecked(true);
    m_cbFour->setChecked(true);
    m_cbFive->setChecked(true);
}

void GenTxtForm::onSelectNone()
{
    m_cbDigit->setChecked(false);
    m_cbLetter->setChecked(false);
    m_cbSignal->setChecked(false);
    m_cbSingle->setChecked(false);
    m_cbTwo->setChecked(false);
    m_cbThree->setChecked(false);
    m_cbFour->setChecked(false);
    m_cbFive->setChecked(false);
}

static QString fullWidthDigits(int n)
{
    QString s;
    for (int i = 0; i < n; ++i)
        s.append(QChar(0xFF10 + qrand() % 10));
    return s;
}

static QString fullWidthLetters(int n)
{
    QString s;
    for (int i = 0; i < n; ++i) {
        if (qrand() % 2 == 0)
            s.append(QChar(0xFF21 + qrand() % 26));
        else
            s.append(QChar(0xFF41 + qrand() % 26));
    }
    return s;
}

static QString fullWidthSignals(int n)
{
    const QString pool = L("，。！？；：、“”（）《》…—");
    QString s;
    for (int i = 0; i < n; ++i)
        s.append(pool.at(qrand() % pool.size()));
    return s;
}

QString GenTxtForm::buildText()
{
    const int target = m_seCount->value();

    QVector<int> kinds;
    if (m_cbSingle->isChecked()) kinds.append(TextGen::Common);
    if (m_cbTwo->isChecked())    kinds.append(TextGen::Word2);
    if (m_cbThree->isChecked())  kinds.append(TextGen::Word3);
    if (m_cbFour->isChecked())   kinds.append(TextGen::Word4);
    if (m_cbFive->isChecked())   kinds.append(TextGen::Word5);

    const bool digit = m_cbDigit->isChecked();
    const bool letter = m_cbLetter->isChecked();
    const bool signal = m_cbSignal->isChecked();

    if (kinds.isEmpty() && !digit && !letter && !signal)
        return TextGen::generate(TextGen::Common, target);

    QString out;
    int guard = 0;
    while (out.size() < target && guard++ < target * 4) {
        const int pick = qrand() % (kinds.size() + (digit ? 1 : 0) + (letter ? 1 : 0) + (signal ? 1 : 0));
        const int chunk = 4 + qrand() % 9;
        if (pick < kinds.size()) {
            out += TextGen::generate(kinds.at(pick), chunk);
        } else if (digit && pick == kinds.size()) {
            out += fullWidthDigits(chunk);
        } else if (letter && pick == kinds.size() + (digit ? 1 : 0)) {
            out += fullWidthLetters(chunk);
        } else if (signal) {
            out += fullWidthSignals(chunk);
        }
    }
    return out.left(target);
}

void GenTxtForm::onGen()
{
    const QString text = buildText();
    // 每 40 字换行
    QString formatted;
    for (int i = 0; i < text.size(); i += 40)
        formatted += text.mid(i, 40) + QLatin1Char('\n');
    m_preview->setPlainText(formatted);
}

void GenTxtForm::onSave()
{
    if (m_preview->toPlainText().trimmed().isEmpty()) {
        suiInfo(this, L("请先生成文本内容！"));
        return;
    }
    const QString fn = QFileDialog::getSaveFileName(this, L("保存为文件"), L("随机测试文本.txt"),
                                                    L("文本文件(*.txt);;所有文件(*.*)"));
    if (fn.isEmpty())
        return;
    QFile f(fn);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        suiInfo(this, L("无法写入文件！"));
        return;
    }
    QTextStream ts(&f);
    ts.setCodec("GBK");
    ts << m_preview->toPlainText();
    ts.flush();
    f.close();
    m_saved = fn;
    accept();
}
