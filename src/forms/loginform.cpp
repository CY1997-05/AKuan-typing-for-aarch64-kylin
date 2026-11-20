#include "loginform.h"
#include "widgets.h"
#include "config.h"
#include "common.h"
#include "store.h"
#include "textgen.h"
#include "hintform.h"
#include "net/lanclient.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QStringList>
#include <QMessageBox>
#include <QApplication>

LoginForm::LoginForm(QWidget *parent, int mode)
    : SuiForm(parent)
    , m_mode(mode)
    , m_minutes(10)
{
    setModal(true);
    setTitleText(L("登录"));
    setClientSize(310, 237);

    // 标题
    QLabel *t = new QLabel(L("中文打字速度测试软件"), body());
    t->setGeometry(38, 47, 250, 28);
    t->setFont(uiFont(24, true, L("楷体")));
    t->setAlignment(Qt::AlignCenter);

    // 左侧竖排模式说明
    QString modeText;
    if (m_mode == 2)
        modeText = L("正式考试模式");
    else if (m_mode == 1)
        modeText = L("联网练习模式");
    else
        modeText = L("本地练习模式");

    m_lbTest = new QLabel(body());
    m_lbTest->setGeometry(11, 74, 25, 119);
    QString vertical;
    for (int i = 0; i < modeText.size(); ++i) {
        vertical.append(modeText.at(i));
        if (i + 1 < modeText.size())
            vertical.append(QLatin1Char('\n'));
    }
    m_lbTest->setText(vertical);
    m_lbTest->setFont(uiFont(19, true, L("楷体")));
    QPalette pal = m_lbTest->palette();
    pal.setColor(QPalette::WindowText, QColor(0xD0, 0, 0));
    m_lbTest->setPalette(pal);
    m_lbTest->setAlignment(Qt::AlignCenter);

    mkLabel(body(), L("请输入您的考号："), 54, 80, 120, 20);

    m_edNumber = mkEdit(body(), 183, 75, 89, 23, 16);
    m_edNumber->setMaxLength(10);
    AppConfig *cfg = AppConfig::instance();
    m_edNumber->setText(cfg->lastName);

    mkLabel(body(), L("测试时长："), 55, 110, 60, 20);
    m_spin = new QSpinBox(body());
    m_spin->setGeometry(146, 106, 68, 26);
    m_spin->setRange(1, 240);
    m_spin->setValue(cfg->testMinutes);
    m_spin->setReadOnly(true);
    mkLabel(body(), L("分钟"), 220, 110, 40, 20);

    QGroupBox *grp = new QGroupBox(L("请选择您要使用的输入法"), body());
    grp->setGeometry(55, 137, 225, 58);
    grp->setFont(uiFont(15));
    m_cbIme = new QComboBox(grp);
    m_cbIme->setGeometry(16, 22, 193, 23);
    m_cbIme->setFont(uiFont(15));
    loadInputMethods();
    if (!cfg->imeName.isEmpty()) {
        int idx = m_cbIme->findText(cfg->imeName);
        if (idx >= 0)
            m_cbIme->setCurrentIndex(idx);
    }

    QPushButton *btLog = mkButton(body(), L("登录"), 55, 198, 105, 29);
    QPushButton *btReturn = mkButton(body(), L("返回"), 175, 198, 105, 29);
    connect(btLog, SIGNAL(clicked()), this, SLOT(onLogin()));
    connect(btReturn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_edNumber, SIGNAL(returnPressed()), this, SLOT(onLogin()));

    m_edNumber->setFocus();
}

void LoginForm::loadInputMethods()
{
    m_cbIme->addItem(L("默认输入法"));
    // 从注册表枚举系统已安装的键盘布局/输入法
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
        m_cbIme->addItem(names.at(i));
}

void LoginForm::onLogin()
{
    QString err;
    const bool ok = (m_mode == 1 || (m_mode == 2 && AppConfig::instance()->mode == AppConfig::ModeClient))
            ? doOnlineLogin(&err) : doLocalLogin(&err);
    if (!ok) {
        suiInfo(this, err.isEmpty() ? L("登录失败！") : err);
        return;
    }
    accept();
}

bool LoginForm::doLocalLogin(QString *err)
{
    AppConfig *cfg = AppConfig::instance();
    const QString no = m_edNumber->text().trimmed();
    if (no.isEmpty()) {
        *err = L("请输入您的考号！");
        return false;
    }
    if (!isValidCandidateNo(no)) {
        *err = L("对不起，考号格式不正确！");
        return false;
    }

    // 限定输入法
    if (cfg->defineIme && !cfg->imeName.isEmpty()) {
        if (m_cbIme->currentText() != cfg->imeName) {
            *err = L("对不起，本次考试限定了输入法，请选择：") + cfg->imeName;
            return false;
        }
    }

    m_no = no;
    m_minutes = cfg->testMinutes;

    if (m_mode == 2) {
        // 正式考试：必须在考生库中
        Candidate *c = Store::instance()->find(no);
        if (!c) {
            *err = L("对不起，无此考号！请检查您的输入是否准确！");
            return false;
        }
        if (c->state != 0) {
            *err = L("对不起，此考号正在考试中，不能再重复登录！如果您确认您的考号从未登录过，请联系考试管理人员在考试服务器上修改您的考试状态。");
            return false;
        }
        const int times = Store::instance()->examCount(no);
        if (cfg->multiMode == 0 && times >= 1) {
            *err = L("对不起，您的测试次数已经超过限制了，不能再考！如确需再考，需请管理员单独对您的考号进行设置。");
            return false;
        }
        if (cfg->multiMode != 0 && times >= cfg->maxTimes) {
            *err = L("对不起，您的测试次数已经超过限制了，不能再考！如确需再考，需请管理员单独对您的考号进行设置。");
            return false;
        }
        m_name = c->name;
        m_idcard = c->idcard;
        Store::instance()->setState(no, 1);

        if (!cfg->testFile.isEmpty()) {
            QFile f(cfg->testFile);
            if (f.open(QIODevice::ReadOnly)) {
                QTextStream ts(&f);
                ts.setCodec("GBK");
                m_text = ts.readAll();
            }
        }
        if (m_text.trimmed().isEmpty())
            m_text = TextGen::buildTestText(1500);
        return true;
    }

    // 本机练习：不强制在考生库中
    Candidate *c = Store::instance()->find(no);
    m_name = c ? c->name : QString();
    m_idcard = c ? c->idcard : QString();
    m_text = resourceText(QLatin1String(":/data/practice.txt"));
    m_text.replace(QLatin1Char('\n'), QString());
    m_text.replace(QLatin1Char('\r'), QString());
    if (m_text.isEmpty())
        m_text = TextGen::buildTestText(600);
    return true;
}

bool LoginForm::doOnlineLogin(QString *err)
{
    AppConfig *cfg = AppConfig::instance();
    const QString no = m_edNumber->text().trimmed();
    if (!isValidCandidateNo(no)) {
        *err = L("对不起，考号格式不正确！");
        return false;
    }

    HintForm hint(0, L("正在从考试服务器读取联网测试或联网练习资料，请稍候......"));
    hint.show();
    QApplication::processEvents();

    LanClient client(this);
    QString name, idcard, text;
    int minutes = 0;
    const bool practice = (m_mode == 1);
    if (!client.login(no, practice, &name, &idcard, &minutes, &text, err)) {
        hint.close();
        return false;
    }
    hint.close();

    m_no = no;
    m_name = name;
    m_idcard = idcard;
    m_minutes = minutes > 0 ? minutes : cfg->testMinutes;
    m_text = text;
    return true;
}
