#include "optionform.h"
#include "widgets.h"
#include "config.h"
#include "common.h"
#include "candidateform.h"
#include "gentxtform.h"
#include "selectipform.h"
#include "passform.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QStringList>
#include <QDir>


OptionForm::OptionForm(QWidget *parent)
    : SuiForm(parent)
{
    setModal(true);
    setTitleText(L("设置"));
    setClientSize(889, 604);

    // 模式选择
    m_cbClient = mkCheck(body(), L("本机是联网考试的客户机之一，所有选项均由考试服务器设置。"), 6, 52, 470);
    m_cbSingle = mkCheck(body(), L("本机用于单机考试，根据下列选项进行测试。"), 6, 143, 340);
    m_cbServer = mkCheck(body(), L("本机用于作为联网考试的服务器，所有考试机按下列选项进行测试。"), 6, 168, 500);

    // 联网考试设置
    m_paClient = new QGroupBox(L("联网考试设置选项"), body());
    m_paClient->setGeometry(6, 76, 465, 62);
    m_paClient->setFont(uiFont(14));
    mkLabel(m_paClient, L("考试服务器IP:"), 11, 30, 99, 20);
    m_edServerIP = mkEdit(m_paClient, 116, 28, 137, 22);
    m_edServerIP->setMaxLength(15);
    m_btSearchServer = mkButton(m_paClient, L("立即自动搜索考试服务器"), 259, 26, 190, 25, 15);

    // 密码选项
    m_paPass = new QGroupBox(L("密码选项"), body());
    m_paPass->setGeometry(486, 76, 393, 112);
    m_paPass->setFont(uiFont(14));
    QPushButton *btPass = mkButton(m_paPass, L("修改管理员密码"), 15, 32, 330, 31);
    m_cbNeedPass = mkCheck(m_paPass, L("查看本机保存的测试和练习记录需要输入管理员密码。"), 14, 72, 375);

    // 单机考试选项
    m_paSingleServer = new QGroupBox(L("单机考试选项"), body());
    m_paSingleServer->setGeometry(6, 200, 873, 345);
    m_paSingleServer->setFont(uiFont(14));

    QPushButton *btCand = mkButton(m_paSingleServer, L("录入、修改考生资料"), 15, 30, 450, 31);
    mkLabel(m_paSingleServer, L("指定测试文本文件:"), 15, 76, 160, 20);
    m_edTestFile = mkEdit(m_paSingleServer, 16, 96, 385, 22);
    m_edTestFile->setReadOnly(true);
    QPushButton *btLoadText = mkButton(m_paSingleServer, L("选择"), 412, 94, 52, 25, 15);
    QPushButton *btGen = mkButton(m_paSingleServer, L("点此自动生成随机内容的测试文本文件"), 16, 126, 450, 31);
    QPushButton *btConvert = mkButton(m_paSingleServer, L("点此工具可将指定文本文件内的半角字符和数字转换为全角"), 16, 166, 450, 31);

    mkLabel(m_paSingleServer, L("限定测试最长时间："), 15, 224, 130, 20);
    m_seTestMinutes = new QSpinBox(m_paSingleServer);
    m_seTestMinutes->setGeometry(178, 220, 71, 25);
    m_seTestMinutes->setRange(1, 240);
    mkLabel(m_paSingleServer, L("分钟"), 258, 224, 40, 20);

    // 多次考试设置
    QGroupBox *sgMulti = new QGroupBox(L("同一考号多次考试设置"), m_paSingleServer);
    sgMulti->setGeometry(480, 24, 377, 96);
    sgMulti->setFont(uiFont(14));
    m_rbOnce = new QRadioButton(L("每人仅允许测试一次"), sgMulti);
    m_rbOnce->setGeometry(16, 26, 340, 20);
    m_rbLast = new QRadioButton(L("每人允许测试多次，取最后一次成绩"), sgMulti);
    m_rbLast->setGeometry(16, 48, 340, 20);
    m_rbBest = new QRadioButton(L("每人允许测试多次，取其最好成绩"), sgMulti);
    m_rbBest->setGeometry(16, 70, 340, 20);
    QButtonGroup *bg = new QButtonGroup(this);
    bg->addButton(m_rbOnce, 0);
    bg->addButton(m_rbLast, 1);
    bg->addButton(m_rbBest, 2);

    mkLabel(m_paSingleServer, L("限定每人最多测试次数为："), 481, 141, 210, 20);
    m_seMultiTest = new QSpinBox(m_paSingleServer);
    m_seMultiTest->setGeometry(690, 136, 71, 25);
    m_seMultiTest->setRange(1, 99);
    mkLabel(m_paSingleServer, L("次"), 770, 141, 30, 20);

    // 输入法
    m_cbDefineIME = mkCheck(m_paSingleServer, L("限定测试所用的输入法(选定后考试期间时将无法切换输入法）"), 16, 250, 450);
    QGroupBox *grIme = new QGroupBox(L("请选择输入法"), m_paSingleServer);
    grIme->setGeometry(15, 275, 450, 56);
    grIme->setFont(uiFont(14));
    m_cbIME = mkCombo(grIme, 16, 22, 420, 22);
    {
        m_cbIME->addItem(L("默认输入法"));
        QSettings reg(QLatin1String("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts"),
                      QSettings::NativeFormat);
        const QStringList groups = reg.childGroups();
        QStringList names;
        for (int i = 0; i < groups.size(); ++i) {
            reg.beginGroup(groups.at(i));
            const QString text = reg.value(QLatin1String("Layout Text")).toString();
            reg.endGroup();
            if (!text.isEmpty() && !names.contains(text))
                names.append(text);
        }
        names.sort();
        for (int i = 0; i < names.size(); ++i)
            m_cbIME->addItem(names.at(i));
    }

    // 联网练习扩展选项
    m_paNetPractice = new QGroupBox(L("联网练习扩展选项"), m_paSingleServer);
    m_paNetPractice->setGeometry(480, 130, 377, 153);
    m_paNetPractice->setFont(uiFont(14));
    mkLabel(m_paNetPractice, L("限定联网练习最长时间："), 15, 34, 180, 20);
    m_sePracMinutes = new QSpinBox(m_paNetPractice);
    m_sePracMinutes->setGeometry(202, 30, 71, 25);
    m_sePracMinutes->setRange(1, 999);
    mkLabel(m_paNetPractice, L("分钟"), 282, 34, 40, 20);
    m_cbRandom = mkCheck(m_paNetPractice, L("电脑随机生成联网练习文本"), 16, 62, 300);
    m_cbSpecFolder = mkCheck(m_paNetPractice, L("指定联网练习文章存放文件夹"), 16, 90, 300);
    m_edNetPracFolder = mkEdit(m_paNetPractice, 16, 120, 289, 22);
    m_edNetPracFolder->setReadOnly(true);
    m_btNetPracFolder = mkButton(m_paNetPractice, L("选择"), 317, 118, 52, 25, 15);

    // 底部按钮
    QPushButton *btOK = mkButton(body(), L("保存并返回"), 248, 559, 124, 31);
    QPushButton *btCancel = mkButton(body(), L("取消"), 544, 559, 116, 31);

    connect(btOK, SIGNAL(clicked()), this, SLOT(onSave()));
    connect(btCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_cbSingle, SIGNAL(clicked()), this, SLOT(onModeChanged()));
    connect(m_cbClient, SIGNAL(clicked()), this, SLOT(onModeChanged()));
    connect(m_cbServer, SIGNAL(clicked()), this, SLOT(onModeChanged()));
    connect(btCand, SIGNAL(clicked()), this, SLOT(onCandidates()));
    connect(btGen, SIGNAL(clicked()), this, SLOT(onGenTestFile()));
    connect(btConvert, SIGNAL(clicked()), this, SLOT(onAutoConvert()));
    connect(btLoadText, SIGNAL(clicked()), this, SLOT(onLoadText()));
    connect(btPass, SIGNAL(clicked()), this, SLOT(onModifyPass()));
    connect(m_btSearchServer, SIGNAL(clicked()), this, SLOT(onSearchServer()));
    connect(m_btNetPracFolder, SIGNAL(clicked()), this, SLOT(onNetPracFolder()));
    connect(m_cbDefineIME, SIGNAL(toggled(bool)), this, SLOT(onDefineImeToggled(bool)));
    connect(m_cbSpecFolder, SIGNAL(toggled(bool)), this, SLOT(onSpecFolderToggled(bool)));
    connect(m_rbOnce, SIGNAL(clicked()), this, SLOT(onMultiModeChanged()));
    connect(m_rbLast, SIGNAL(clicked()), this, SLOT(onMultiModeChanged()));
    connect(m_rbBest, SIGNAL(clicked()), this, SLOT(onMultiModeChanged()));

    loadFromConfig();
}

void OptionForm::loadFromConfig()
{
    AppConfig *cfg = AppConfig::instance();

    m_cbSingle->setChecked(cfg->mode == AppConfig::ModeSingle);
    m_cbClient->setChecked(cfg->mode == AppConfig::ModeClient);
    m_cbServer->setChecked(cfg->mode == AppConfig::ModeServer);

    m_edServerIP->setText(cfg->serverIp);
    m_edTestFile->setText(cfg->testFile);
    m_seTestMinutes->setValue(cfg->testMinutes);
    m_seMultiTest->setValue(cfg->maxTimes);
    m_sePracMinutes->setValue(cfg->practiceMinutes);
    m_edNetPracFolder->setText(cfg->practiceFolder);
    m_cbRandom->setChecked(cfg->practiceRandom);
    m_cbSpecFolder->setChecked(cfg->practiceUseFolder);
    m_cbNeedPass->setChecked(cfg->needPassword);
    m_cbDefineIME->setChecked(cfg->defineIme);

    if (cfg->multiMode == 1)
        m_rbLast->setChecked(true);
    else if (cfg->multiMode == 2)
        m_rbBest->setChecked(true);
    else
        m_rbOnce->setChecked(true);

    const int idx = m_cbIME->findText(cfg->imeName);
    if (idx >= 0)
        m_cbIME->setCurrentIndex(idx);

    updateEnabled();
    onModeChanged();
}

void OptionForm::updateEnabled()
{
    m_seMultiTest->setEnabled(!m_rbOnce->isChecked());
    m_cbIME->setEnabled(m_cbDefineIME->isChecked());
    m_edNetPracFolder->setEnabled(m_cbSpecFolder->isChecked());
    m_btNetPracFolder->setEnabled(m_cbSpecFolder->isChecked());
}

void OptionForm::onModeChanged()
{
    // 三个复选框像单选按钮一样互斥
    QCheckBox *senderBox = qobject_cast<QCheckBox *>(sender());
    if (senderBox && senderBox->isChecked()) {
        if (senderBox != m_cbSingle) m_cbSingle->setChecked(false);
        if (senderBox != m_cbClient) m_cbClient->setChecked(false);
        if (senderBox != m_cbServer) m_cbServer->setChecked(false);
    }
    if (!m_cbSingle->isChecked() && !m_cbClient->isChecked() && !m_cbServer->isChecked())
        m_cbSingle->setChecked(true);

    const bool client = m_cbClient->isChecked();
    m_paClient->setVisible(client);
    m_paNetPractice->setVisible(client);
    m_paSingleServer->setVisible(true);
}

void OptionForm::onCandidates()
{
    CandidateForm dlg(this);
    dlg.exec();
}

void OptionForm::onGenTestFile()
{
    GenTxtForm dlg(this);
    if (dlg.exec() == QDialog::Accepted && !dlg.savedFile().isEmpty())
        m_edTestFile->setText(dlg.savedFile());
}

void OptionForm::onAutoConvert()
{
    const QString fn = QFileDialog::getOpenFileName(this, L("请选择要转换的文本文件"), QString(),
                                                    L("文本文件(*.txt);;所有文件(*.*)"));
    if (fn.isEmpty())
        return;
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) {
        suiInfo(this, L("无法读取该文件！"));
        return;
    }
    QTextStream ts(&f);
    ts.setCodec("GBK");
    QString text = ts.readAll();
    f.close();

    QString out;
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        const ushort u = c.unicode();
        if (u >= 0x21 && u <= 0x7E)
            out.append(QChar(u + 0xFEE0));
        else
            out.append(c);
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        suiInfo(this, L("无法写入该文件！"));
        return;
    }
    QTextStream os(&f);
    os.setCodec("GBK");
    os << out;
    os.flush();
    f.close();

    suiInfo(this, L("转换完成！半角字符和数字已转换为全角。"));
}

void OptionForm::onLoadText()
{
    const QString fn = QFileDialog::getOpenFileName(this, L("请选择测试文本文件"), QString(),
                                                    L("文本文件(*.txt);;所有文件(*.*)"));
    if (!fn.isEmpty())
        m_edTestFile->setText(fn);
}

void OptionForm::onSearchServer()
{
    SelectIPForm dlg(this);
    if (dlg.exec() == QDialog::Accepted && !dlg.selectedIp().isEmpty())
        m_edServerIP->setText(dlg.selectedIp());
}

void OptionForm::onNetPracFolder()
{
    const QString dir = QFileDialog::getExistingDirectory(this, L("请选择您存放联网练习文章(.TXT文件)的文件夹："),
                                                          m_edNetPracFolder->text());
    if (!dir.isEmpty())
        m_edNetPracFolder->setText(dir);
}

void OptionForm::onDefineImeToggled(bool on)
{
    m_cbIME->setEnabled(on);
}

void OptionForm::onSpecFolderToggled(bool on)
{
    m_edNetPracFolder->setEnabled(on);
    m_btNetPracFolder->setEnabled(on);
}

void OptionForm::onMultiModeChanged()
{
    updateEnabled();
}

void OptionForm::onModifyPass()
{
    AppConfig *cfg = AppConfig::instance();

    PassForm oldDlg(this, L("请输入旧密码："));
    if (oldDlg.exec() != QDialog::Accepted)
        return;
    if (oldDlg.password() != cfg->adminPassword) {
        suiInfo(this, L("对不起，原密码错误！你无权修改管理员密码！"));
        return;
    }

    PassForm newDlg(this, L("请输入新密码："));
    if (newDlg.exec() != QDialog::Accepted)
        return;

    PassForm againDlg(this, L("请再输一次新密码："));
    if (againDlg.exec() != QDialog::Accepted)
        return;

    if (newDlg.password() != againDlg.password()) {
        suiInfo(this, L("两次密码输入不一致，密码修改失败！"));
        return;
    }
    if (newDlg.password().isEmpty()) {
        suiInfo(this, L("新密码不能为空！"));
        return;
    }
    cfg->adminPassword = newDlg.password();
    cfg->save();
    suiInfo(this, L("管理员密码设置成功！"));
}

void OptionForm::saveToConfig()
{
    AppConfig *cfg = AppConfig::instance();

    if (m_cbClient->isChecked())
        cfg->mode = AppConfig::ModeClient;
    else if (m_cbServer->isChecked())
        cfg->mode = AppConfig::ModeServer;
    else
        cfg->mode = AppConfig::ModeSingle;

    cfg->serverIp = m_edServerIP->text().trimmed();
    cfg->testFile = m_edTestFile->text();
    cfg->testMinutes = m_seTestMinutes->value();
    cfg->maxTimes = m_seMultiTest->value();
    cfg->practiceMinutes = m_sePracMinutes->value();
    cfg->practiceRandom = m_cbRandom->isChecked();
    cfg->practiceUseFolder = m_cbSpecFolder->isChecked();
    cfg->practiceFolder = m_edNetPracFolder->text();
    cfg->needPassword = m_cbNeedPass->isChecked();
    cfg->defineIme = m_cbDefineIME->isChecked();
    cfg->imeName = m_cbDefineIME->isChecked() ? m_cbIME->currentText() : QString();

    if (m_rbLast->isChecked())
        cfg->multiMode = 1;
    else if (m_rbBest->isChecked())
        cfg->multiMode = 2;
    else
        cfg->multiMode = 0;

    cfg->save();
}

void OptionForm::onSave()
{
    if (!m_cbSingle->isChecked() && !m_cbClient->isChecked() && !m_cbServer->isChecked()) {
        suiInfo(this, L("对不起！您必须从上述模式中选择一种考试模式。"));
        return;
    }
    if (m_cbClient->isChecked()) {
        const QString ip = m_edServerIP->text().trimmed();
        const QStringList parts = ip.split(QLatin1Char('.'));
        bool ok = (parts.size() == 4);
        if (ok) {
            for (int i = 0; i < 4; ++i) {
                bool n = false;
                const int v = parts.at(i).toInt(&n);
                if (!n || v < 0 || v > 255) { ok = false; break; }
            }
        }
        if (!ok) {
            suiInfo(this, L("对不起！您的服务器IP输入格式不正确！请检查。"));
            return;
        }
    }
    if (!m_edTestFile->text().isEmpty() && m_cbServer->isChecked()) {
        // 服务器模式必须能读到考试文本
        QFile f(m_edTestFile->text());
        if (!f.open(QIODevice::ReadOnly)) {
            suiInfo(this, L("考试模式设置错误，请检查设置文件是否存在、可读或是否被人为修改过！"));
            return;
        }
    }

    saveToConfig();
    accept();
}
