#include <QApplication>
#include <QDateTime>
#include <QFont>
#include <QIcon>
#include <QDir>
#include <QTimer>
#include <QPushButton>
#include <QKeyEvent>
#include <QEventLoop>
#include <QStringList>
#include <QElapsedTimer>
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#ifdef Q_OS_LINUX
#include <QLibraryInfo>
#include <QStandardPaths>
#endif

#include "common.h"
#include "config.h"
#include "store.h"
#include "theme.h"
#include "widgets.h"
#include "forms/firstform.h"
#include "forms/loginform.h"
#include "forms/testform.h"
#include "forms/resultform.h"
#include "forms/optionform.h"
#include "forms/candidateform.h"
#include "forms/gentxtform.h"
#include "forms/aboutform.h"
#include "forms/reportform.h"
#include "forms/passform.h"
#include "forms/serverform.h"
#include "forms/selectipform.h"


static void pump(int ms)
{
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < ms) {
        QApplication::processEvents(QEventLoop::AllEvents, 20);
        QThread::msleep(10);
    }
}

static void shot(QWidget *w, const QString &path)
{
    w->setWindowModality(Qt::NonModal);
    pump(250);
    w->grab().save(path);
}

// 开发者用：HZTest.exe --shots <目录>  自动生成各界面截图后退出
static void runShotMode(FirstForm *first, const QString &dir)
{
    QDir().mkpath(dir);

    first->show();
    shot(first, dir + QLatin1String("/01_main.png"));
    first->hide();

    {
        OptionForm opt(0);
        shot(&opt, dir + QLatin1String("/02_options.png"));
    }

    {
        TestForm test(0, TestForm::Practice, QString::fromLatin1("222"), QString(), QString(),
                      QString(), 10, false);
        shot(&test, dir + QLatin1String("/03_test.png"));

        QList<QPushButton *> btns = test.findChildren<QPushButton *>();
        if (!btns.isEmpty())
            btns.at(0)->click();          // 开始练习
        pump(200);

        TypeLine *line = test.findChild<TypeLine *>();
        const QString sample = QString::fromUtf8("\xE6\x82\xA8\xE5\xA5\xBD\xEF\xBC\x81\xE9\x9D\x9E\xE5\xB8\xB8\xE6\x84\x9F\xE8\xB0\xA2\xE6\x82\xA8\xE4\xBD\xBF\xE7\x94\xA8\xE6\xAD\xA4\xE8\xBD\xAF\xE4\xBB\xB6\xE3\x80\x82");
        if (line) {
            for (int i = 0; i < sample.size(); ++i) {
                QKeyEvent ev(QEvent::KeyPress, 0, Qt::NoModifier, QString(sample.at(i)));
                QApplication::sendEvent(line, &ev);
            }
            QKeyEvent wrong(QEvent::KeyPress, 0, Qt::NoModifier, QString(QChar(0x9519)));
            QApplication::sendEvent(line, &wrong);
        }
        shot(&test, dir + QLatin1String("/04_test_typed.png"));
    }

    first->close();
}

#ifdef Q_OS_LINUX
// ---------------------------------------------------------------------------
// Linux 中文输入法适配（打字界面里输入法能不能用的关键）
//
// 本程序在 Linux 下用自编译的 Qt 5.9.9（/usr/local/Qt-5.9.9）。这个 Qt 的输入法
// 前端插件只有 compose / xim / ibus 三个，**没有 fcitx 前端**；而国产 Linux（麒麟等）
// 默认把 QT_IM_MODULE 设成 fcitx，于是 Qt 找不到任何可用的输入法前端，中文输入法在
// 打字界面里完全不生效（敲拼音只出英文字母）。
//
// fcitx5 自带 IBus 前端（在会话总线上提供 org.freedesktop.IBus 服务，并写出
// ~/.config/ibus/bus/* 地址文件），Qt 5.9 的 ibus 前端正好能和它通信，所以这里把
// 输入法改走 IBus：
//   1. 当前 Qt 没有 fcitx 前端、但有 ibus 前端，且本机确实有 IBus 服务时，把
//      QT_IM_MODULE 设为 ibus；
//   2. Qt 的 ibus 前端用“PATH 里有没有 ibus-daemon 这条命令”判断 IBus 是否可用。
//      只装 fcitx5、没装 ibus 的机器上它就直接放弃，所以这里把一个只用于存在性探测的
//      空壳脚本放进**本进程**的 PATH（不改动系统，也永远不会被真正执行）。
//
// 注意：走 IBus 之后，用户用的仍然是 fcitx5（候选框、Ctrl+Space / Ctrl+Shift 切换
// 键都不变），只是 Qt 与 fcitx5 之间的“通道”换成了 IBus 协议。
//
// 关闭这段适配：设环境变量 HZTEST_NO_IM_BRIDGE=1；
// 自行指定输入法前端：设 QT_IM_MODULE（如 fcitx / ibus / compose）。
// ---------------------------------------------------------------------------

static bool imInputContextPluginExists(const QString &icDir, const QString &prefix)
{
    QDir d(icDir);
    if (!d.exists())
        return false;
    const QStringList filters = QStringList() << (prefix + QLatin1Char('*'));
    return !d.entryList(filters, QDir::Files).isEmpty();
}

// Qt 的 ibus 前端也是读这个目录下的地址文件去找 IBus 服务的
static bool ibusServiceLooksReady()
{
    const QString busDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                           + QLatin1String("/ibus/bus");
    QDir d(busDir);
    if (!d.exists())
        return false;
    return !d.entryList(QDir::Files | QDir::Hidden).isEmpty();
}

// 提供给 Qt 探测用的 ibus-daemon 空壳；优先用随程序发布的那个（bin/im/ibus-daemon），
// 找不到时在用户缓存目录里生成一个。
static QString ensureIbusDaemonShim(const QString &appDir)
{
    static const char *kScript =
        "#!/bin/sh\n"
        "# HZTest 输入法桥接用空壳：Qt 5.9 的 ibus 输入法前端只检查 PATH 中是否存在\n"
        "# ibus-daemon 这条命令，以此判断 IBus 是否可用。本机实际使用的是 fcitx5\n"
        "# （由它自带的 IBus 前端提供服务），所以需要一个空壳让探测通过。\n"
        "# 这个脚本不会被真正执行，也不影响系统里的其它程序。\n"
        "exit 0\n";

    const QStringList dirs = QStringList()
        << (appDir + QLatin1String("/im"))
        << (QDir::homePath() + QLatin1String("/.cache/HZTest/im"));

    for (int i = 0; i < dirs.size(); ++i) {
        QFileInfo fi(dirs.at(i) + QLatin1String("/ibus-daemon"));
        if (fi.exists() && fi.isExecutable())
            return dirs.at(i);
    }

    const QString dir = dirs.at(1);
    if (!QDir().mkpath(dir))
        return QString();
    QFile f(dir + QLatin1String("/ibus-daemon"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return QString();
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    ts << kScript;
    ts.flush();
    f.close();
    if (!f.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
                          | QFile::ReadGroup | QFile::ExeGroup
                          | QFile::ReadOther | QFile::ExeOther))
        return QString();
    return dir;
}

static void setupLinuxInputMethod(const char *argv0)
{
    if (qEnvironmentVariableIsSet("HZTEST_NO_IM_BRIDGE"))
        return;

    const QString icDir = QLibraryInfo::location(QLibraryInfo::PluginsPath)
                          + QLatin1String("/platforminputcontexts");

    // 当前 Qt 自带 fcitx 前端（例如发行版的 Qt 5.15）→ 系统自己能处理，不插手
    if (imInputContextPluginExists(icDir, QLatin1String("libfcitx")))
        return;
    // 连 ibus 前端都没有 → 这段适配帮不上忙
    if (!imInputContextPluginExists(icDir, QLatin1String("libibus")))
        return;
    // 本机没有 IBus 服务（没装 ibus，也不是 fcitx5）→ 保持原样
    if (!ibusServiceLooksReady())
        return;

    const QString cur = QString::fromLocal8Bit(qgetenv("QT_IM_MODULE"));
    if (!cur.isEmpty()
        && cur != QLatin1String("fcitx") && cur != QLatin1String("fcitx5")
        && cur != QLatin1String("xim") && cur != QLatin1String("ibus")) {
        return;   // 用户明确指定了别的输入法前端，尊重它
    }

    QString appDir;
    if (argv0 && *argv0) {
        const QFileInfo fi(QString::fromLocal8Bit(argv0));
        appDir = fi.absolutePath();
    }
    if (appDir.isEmpty())
        appDir = QDir::currentPath();

    if (QStandardPaths::findExecutable(QLatin1String("ibus-daemon")).isEmpty()) {
        const QString shimDir = ensureIbusDaemonShim(appDir);
        if (shimDir.isEmpty())
            return;
        const QByteArray oldPath = qgetenv("PATH");
        qputenv("PATH", shimDir.toLocal8Bit() + ':' + oldPath);
    }

    qputenv("QT_IM_MODULE", "ibus");
}
#endif // Q_OS_LINUX

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    // 必须在构造 QApplication 之前完成：Qt 就是在构造时创建输入法上下文的
    setupLinuxInputMethod(argc > 0 ? argv[0] : 0);
#endif

    QApplication app(argc, argv);
    app.setApplicationName(QLatin1String("HZTest"));
    app.setOrganizationName(QLatin1String("AKuan"));

    QFont f = uiFont(15);
    app.setFont(f);

    qsrand(static_cast<uint>(QDateTime::currentDateTime().toTime_t()));

    AppConfig *cfg = AppConfig::instance();
    cfg->load();

    Store::instance()->load();

    Theme::instance()->setIndex(cfg->themeIndex);
    app.setStyleSheet(Theme::instance()->styleSheet());

    FirstForm w;

    const QStringList args = app.arguments();
    const int idx = args.indexOf(QLatin1String("--shots"));
    if (idx > 0 && idx + 1 < args.size()) {
        runShotMode(&w, args.at(idx + 1));
        return 0;
    }

    w.show();
    return app.exec();
}
