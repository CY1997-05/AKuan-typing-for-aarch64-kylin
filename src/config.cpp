#include "config.h"
#include "common.h"

#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QTextStream>

AppConfig *AppConfig::s_instance = 0;

AppConfig *AppConfig::instance()
{
    if (!s_instance)
        s_instance = new AppConfig();
    return s_instance;
}

AppConfig::AppConfig()
    : mode(ModeSingle)
    , serverIp(L("192.168.0.254"))
    , testMinutes(10)
    , multiMode(0)
    , maxTimes(3)
    , defineIme(false)
    , practiceMinutes(5)
    , practiceRandom(true)
    , practiceUseFolder(false)
    , needPassword(true)
    , adminPassword(L("888888"))
    , themeIndex(0)
{
}

QString AppConfig::fileName() const
{
    return dataDir() + L("hztest.ini");
}

void AppConfig::load()
{
    QSettings s(fileName(), QSettings::IniFormat);
    s.setIniCodec("UTF-8");

    mode            = s.value(L("Basic/Mode"), mode).toInt();
    serverIp        = s.value(L("Basic/ServerIp"), serverIp).toString();
    testMinutes     = s.value(L("Exam/Minutes"), testMinutes).toInt();
    testFile        = s.value(L("Exam/TextFile"), testFile).toString();
    multiMode       = s.value(L("Exam/MultiMode"), multiMode).toInt();
    maxTimes        = s.value(L("Exam/MaxTimes"), maxTimes).toInt();
    defineIme       = s.value(L("Exam/DefineIme"), defineIme).toBool();
    imeName         = s.value(L("Exam/ImeName"), imeName).toString();
    practiceMinutes = s.value(L("Practice/Minutes"), practiceMinutes).toInt();
    practiceRandom  = s.value(L("Practice/Random"), practiceRandom).toBool();
    practiceUseFolder = s.value(L("Practice/UseFolder"), practiceUseFolder).toBool();
    practiceFolder  = s.value(L("Practice/Folder"), practiceFolder).toString();
    needPassword    = s.value(L("Admin/NeedPassword"), needPassword).toBool();
    adminPassword   = s.value(L("Admin/Password"), adminPassword).toString();
    themeIndex      = s.value(L("Ui/Theme"), themeIndex).toInt();
    lastName        = s.value(L("Basic/LastName"), lastName).toString();

    if (testMinutes < 1)   testMinutes = 10;
    if (testMinutes > 240) testMinutes = 240;
    if (maxTimes < 1)      maxTimes = 1;
    if (maxTimes > 99)     maxTimes = 99;
    if (practiceMinutes < 1)    practiceMinutes = 5;
    if (practiceMinutes > 999)  practiceMinutes = 999;
    if (mode < 0 || mode > 2)   mode = ModeSingle;
    if (adminPassword.isEmpty()) adminPassword = L("888888");
}

void AppConfig::save()
{
    QSettings s(fileName(), QSettings::IniFormat);
    s.setIniCodec("UTF-8");

    s.setValue(L("Basic/Mode"), mode);
    s.setValue(L("Basic/ServerIp"), serverIp);
    s.setValue(L("Exam/Minutes"), testMinutes);
    s.setValue(L("Exam/TextFile"), testFile);
    s.setValue(L("Exam/MultiMode"), multiMode);
    s.setValue(L("Exam/MaxTimes"), maxTimes);
    s.setValue(L("Exam/DefineIme"), defineIme);
    s.setValue(L("Exam/ImeName"), imeName);
    s.setValue(L("Practice/Minutes"), practiceMinutes);
    s.setValue(L("Practice/Random"), practiceRandom);
    s.setValue(L("Practice/UseFolder"), practiceUseFolder);
    s.setValue(L("Practice/Folder"), practiceFolder);
    s.setValue(L("Admin/NeedPassword"), needPassword);
    s.setValue(L("Admin/Password"), adminPassword);
    s.setValue(L("Ui/Theme"), themeIndex);
    s.setValue(L("Basic/LastName"), lastName);
    s.sync();
}
