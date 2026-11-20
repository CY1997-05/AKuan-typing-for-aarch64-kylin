#include "common.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegExp>

QString appDir()
{
    QString d = QCoreApplication::applicationDirPath();
    if (!d.endsWith(QLatin1Char('/')))
        d += QLatin1Char('/');
    return d;
}

QString dataDir()
{
    // 单文件版：启动器会把数据目录通过环境变量指到 exe 所在目录，
    // 这样配置/考生库/成绩都跟着 exe 走，换电脑拷贝即用。
    const QString env = QString::fromLocal8Bit(qgetenv("HZTEST_DATA_DIR"));
    if (!env.isEmpty()) {
        QString d = env;
        if (!d.endsWith(QLatin1Char('/')))
            d += QLatin1Char('/');
        return d;
    }
    return appDir();
}

QString resourceText(const QString &resPath)
{
    QFile f(resPath);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    return ts.readAll();
}

QStringList resourceLines(const QString &resPath)
{
    QStringList out;
    const QStringList raw = resourceText(resPath).split(QLatin1Char('\n'));
    for (int i = 0; i < raw.size(); ++i) {
        QString s = raw.at(i);
        if (s.endsWith(QLatin1Char('\r')))
            s.chop(1);
        s = s.trimmed();
        if (!s.isEmpty())
            out.append(s);
    }
    return out;
}

QString fmtTime(int seconds)
{
    if (seconds < 0) seconds = 0;
    const int m = seconds / 60;
    const int s = seconds % 60;
    return QString::fromLatin1("%1:%2")
            .arg(m, 2, 10, QLatin1Char('0'))
            .arg(s, 2, 10, QLatin1Char('0'));
}

bool isValidCandidateNo(const QString &no)
{
    // 原版：考号由数字组成，最长 10 位
    if (no.isEmpty() || no.size() > 10)
        return false;
    for (int i = 0; i < no.size(); ++i) {
        const QChar c = no.at(i);
        if (c < QLatin1Char('0') || c > QLatin1Char('9'))
            return false;
    }
    return true;
}

bool isValidIdCard(const QString &id)
{
    // 18 位：前 17 位数字，末位数字或 X
    if (id.size() != 18)
        return false;
    for (int i = 0; i < 17; ++i) {
        const QChar c = id.at(i);
        if (c < QLatin1Char('0') || c > QLatin1Char('9'))
            return false;
    }
    const QChar last = id.at(17);
    return (last >= QLatin1Char('0') && last <= QLatin1Char('9')) || last == QLatin1Char('X')
            || last == QLatin1Char('x');
}
