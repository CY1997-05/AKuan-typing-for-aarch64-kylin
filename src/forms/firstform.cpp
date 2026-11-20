#include "firstform.h"
#include "widgets.h"
#include "theme.h"
#include "config.h"
#include "common.h"
#include "store.h"
#include "optionform.h"
#include "loginform.h"
#include "reportform.h"
#include "aboutform.h"
#include "serverform.h"
#include "passform.h"
#include "testform.h"

#include <QPushButton>
#include <QLabel>
#include <QApplication>


FirstForm::FirstForm(QWidget *parent)
    : SuiForm(parent)
{
    setTitleText(L("阿宽软件系列之中文打字速度测试软件V1.41"));
    setClientSize(483, 541);
    setShowMinButton(true);

    // 标题
    QLabel *title = new QLabel(L("中文打字速度测试软件 V1.41(网络版)"), body());
    title->setGeometry(30, 50, 429, 28);
    title->setFont(uiFont(24, true, L("楷体")));
    title->setAlignment(Qt::AlignCenter);

    m_btPractice = mkButton(body(), L("进入本机练习"), 124, 184, 225, 39);
    m_btNetPractice = mkButton(body(), L("进入联网练习"), 124, 233, 225, 39);
    m_btEnter = mkButton(body(), L("进入正式测试"), 124, 282, 225, 39);

    QPushButton *btOption = mkButton(body(), L("测试设置"), 124, 85, 225, 39);
    QPushButton *btServer = mkButton(body(), L("本机进入考试服务器模式"), 124, 134, 225, 39);
    QPushButton *btReport = mkButton(body(), L("查看本机保存的测试结果"), 124, 331, 225, 39);
    QPushButton *btReportPrac = mkButton(body(), L("查看本机保存的联网练习结果"), 124, 381, 225, 39);
    QPushButton *btTheme = mkButton(body(), L("换个界面，换换心情"), 124, 430, 225, 39);
    QPushButton *btAbout = mkButton(body(), L("关于此软件..."), 124, 479, 225, 39);

    btOption->setToolTip(L("正式考试或联网练习前必须先进此进行设置"));
    btServer->setToolTip(L("进入前须在测试设置里将本机的服务器选项设置完毕"));
    m_btPractice->setToolTip(L("仅在本机进行练习，不联网，练习情况也不上传"));
    m_btNetPractice->setToolTip(L("联网练习模式下由考试服务器自动随机选择文章进行练习，并能够把每次的练习情况都自动上传到考试服务器以了解其练习提高情况"));
    m_btEnter->setToolTip(L("进入正式考试环境，请认真对待！"));
    btReport->setToolTip(L("在联网测试的时候，成绩会自动上传至服务器，请确认是在考试服务器上点击此按钮。"));
    btReportPrac->setToolTip(L("在联网练习的时候，练习成绩会自动上传至服务器，请确认是在考试服务器上点此按钮。"));
    btTheme->setToolTip(L("换个窗口界面"));
    btAbout->setToolTip(L("有此软件有关的一些事项。"));

    connect(btOption, SIGNAL(clicked()), this, SLOT(onOption()));
    connect(btServer, SIGNAL(clicked()), this, SLOT(onServerMode()));
    connect(m_btPractice, SIGNAL(clicked()), this, SLOT(onPractice()));
    connect(m_btNetPractice, SIGNAL(clicked()), this, SLOT(onNetPractice()));
    connect(m_btEnter, SIGNAL(clicked()), this, SLOT(onEnterExam()));
    connect(btReport, SIGNAL(clicked()), this, SLOT(onReport()));
    connect(btReportPrac, SIGNAL(clicked()), this, SLOT(onReportPractice()));
    connect(btTheme, SIGNAL(clicked()), this, SLOT(onChangeTheme()));
    connect(btAbout, SIGNAL(clicked()), this, SLOT(onAbout()));

    updateButtons();
}

void FirstForm::updateButtons()
{
    AppConfig *cfg = AppConfig::instance();

    // 正式测试需要先指定测试文本，或在服务器模式下由服务器下发
    const bool examReady = !cfg->testFile.isEmpty() || cfg->mode == AppConfig::ModeClient;
    m_btEnter->setEnabled(examReady);
    m_btNetPractice->setEnabled(cfg->mode == AppConfig::ModeClient);
}

void FirstForm::onOption()
{
    OptionForm dlg(this);
    dlg.exec();
    updateButtons();
}

void FirstForm::onServerMode()
{
    AppConfig *cfg = AppConfig::instance();
    if (cfg->mode != AppConfig::ModeServer) {
        suiInfo(this, L("对不起，请先在“测试设置”里选择“将本机作为考试服务器”并将所有考试选项设置好再点此按钮。"),
                L("信息提示"));
        OptionForm dlg(this);
        dlg.exec();
        if (AppConfig::instance()->mode != AppConfig::ModeServer) {
    updateButtons();
            return;
        }
    }
    if (!suiConfirm(this, L("您已经选择了本机作为考试服务器，是否立即进入考试服务器状态？"), L("信息提示")))
        return;

    ServerForm *f = new ServerForm(this);
    f->setAttribute(Qt::WA_DeleteOnClose);
    f->show();
    hide();
    connect(f, SIGNAL(finished()), this, SLOT(show()));
}

void FirstForm::onPractice()
{
    startTest(0);
}

void FirstForm::onNetPractice()
{
    if (AppConfig::instance()->mode != AppConfig::ModeClient) {
        suiInfo(this, L("对不起，请先在“测试设置”里选择“本机是联网考试的客户机之一”，并设置好考试服务器IP。"));
        return;
    }
    startTest(1);
}

void FirstForm::onEnterExam()
{
    startTest(2);
}

void FirstForm::startTest(int mode)
{
    AppConfig *cfg = AppConfig::instance();

    if (mode == 2 && cfg->testFile.isEmpty() && cfg->mode != AppConfig::ModeClient) {
        suiInfo(this, L("对不起，请先在“测试设置”里设置好考试文本文件再点此按钮。"));
        return;
    }

    LoginForm dlg(this, mode);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString no = dlg.candidateNo();
    const QString name = dlg.candidateName();
    const QString id = dlg.candidateId();
    const QString text = dlg.testText();
    const int minutes = dlg.testMinutes();

    cfg->lastName = no;
    cfg->save();

    const bool online = (mode == 1) || (mode == 2 && cfg->mode == AppConfig::ModeClient);
    TestForm *tf = new TestForm(0, mode, no, name, id, text, minutes, online);
    tf->setAttribute(Qt::WA_DeleteOnClose);
    connect(tf, SIGNAL(finished()), this, SLOT(show()));
    connect(tf, SIGNAL(finished()), this, SLOT(updateButtonsPrivate()));
    hide();
    tf->show();
    tf->raise();
    tf->activateWindow();
}

void FirstForm::updateButtonsPrivate()
{
    updateButtons();
}

void FirstForm::onReport()
{
    if (!askAdminPassword(this))
        return;
    ReportForm dlg(this, 0);
    dlg.exec();
}

void FirstForm::onReportPractice()
{
    if (!askAdminPassword(this))
        return;
    ReportForm dlg(this, 1);
    dlg.exec();
}

void FirstForm::onChangeTheme()
{
    Theme *t = Theme::instance();
    t->next();
    AppConfig::instance()->themeIndex = t->index();
    AppConfig::instance()->save();

    applyTheme();
    // 让所有顶层窗口一起换肤
    QWidgetList all = QApplication::topLevelWidgets();
    for (int i = 0; i < all.size(); ++i) {
        SuiForm *f = qobject_cast<SuiForm *>(all.at(i));
        if (f)
            f->applyTheme();
        else
            all.at(i)->setStyleSheet(t->styleSheet());
    }
    update();
}

void FirstForm::onAbout()
{
    AboutForm dlg(this);
    dlg.exec();
}
