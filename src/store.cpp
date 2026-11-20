#include "store.h"
#include "common.h"

#include <QFile>
#include <QTextStream>
#include <QByteArray>

static const char kHeader[] = "HZTEST-DATA-1";

// 简单异或 + 十六进制，效果等同原版“配置文件加密保存，防止人为篡改”
static QString encodeLine(const QString &plain)
{
    QByteArray ba = plain.toUtf8();
    for (int i = 0; i < ba.size(); ++i)
        ba[i] = static_cast<char>(ba.at(i) ^ (0x5A + (i % 7)));
    return QString::fromLatin1(ba.toHex());
}

static QString decodeLine(const QString &enc)
{
    QByteArray ba = QByteArray::fromHex(enc.toLatin1());
    for (int i = 0; i < ba.size(); ++i)
        ba[i] = static_cast<char>(ba.at(i) ^ (0x5A + (i % 7)));
    return QString::fromUtf8(ba);
}

static QStringList readEncoded(const QString &path)
{
    QStringList out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return out;
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    const QStringList lines = ts.readAll().split(QLatin1Char('\n'));
    for (int i = 0; i < lines.size(); ++i) {
        const QString s = lines.at(i).trimmed();
        if (s.isEmpty() || s == QLatin1String(kHeader))
            continue;
        out.append(decodeLine(s));
    }
    return out;
}

static bool writeEncoded(const QString &path, const QStringList &plainLines)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    ts << QLatin1String(kHeader) << QLatin1Char('\n');
    for (int i = 0; i < plainLines.size(); ++i)
        ts << encodeLine(plainLines.at(i)) << QLatin1Char('\n');
    ts.flush();
    return true;
}

Store *Store::s_instance = 0;

Store *Store::instance()
{
    if (!s_instance)
        s_instance = new Store();
    return s_instance;
}

Store::Store()
{
}

QString Store::candidatesFile() const { return dataDir() + L("candidates.dat"); }
QString Store::recordsFile()  const { return dataDir() + L("records.dat"); }
QString Store::practiceFile() const { return dataDir() + L("practice.dat"); }

void Store::load()
{
    m_candidates.clear();
    const QStringList cl = readEncoded(candidatesFile());
    for (int i = 0; i < cl.size(); ++i) {
        const QStringList f = cl.at(i).split(QLatin1Char('\t'));
        if (f.size() < 3)
            continue;
        Candidate c;
        c.no = f.at(0);
        c.name = f.at(1);
        c.idcard = f.at(2);
        c.times = (f.size() > 3) ? f.at(3).toInt() : 0;
        c.state = (f.size() > 4) ? f.at(4).toInt() : 0;
        m_candidates.append(c);
    }

    m_records.clear();
    const QStringList rl = readEncoded(recordsFile());
    for (int i = 0; i < rl.size(); ++i) {
        const QStringList f = rl.at(i).split(QLatin1Char('\t'));
        if (f.size() < 12)
            continue;
        ExamRecord r;
        r.no = f.at(0);
        r.name = f.at(1);
        r.idcard = f.at(2);
        r.total = f.at(3).toInt();
        r.correct = f.at(4).toInt();
        r.wrong = f.at(5).toInt();
        r.remain = f.at(6).toInt();
        r.seconds = f.at(7).toInt();
        r.speed = f.at(8).toInt();
        r.accuracy = f.at(9).toInt();
        r.when = QDateTime::fromString(f.at(10), QLatin1String("yyyy-MM-dd HH:mm:ss"));
        r.file = f.at(11);
        m_records.append(r);
    }

    m_practice.clear();
    const QStringList pl = readEncoded(practiceFile());
    for (int i = 0; i < pl.size(); ++i) {
        const QStringList f = pl.at(i).split(QLatin1Char('\t'));
        if (f.size() < 8)
            continue;
        PracticeRecord r;
        r.no = f.at(0);
        r.name = f.at(1);
        r.total = f.at(2).toInt();
        r.correct = f.at(3).toInt();
        r.seconds = f.at(4).toInt();
        r.speed = f.at(5).toInt();
        r.accuracy = f.at(6).toInt();
        r.when = QDateTime::fromString(f.at(7), QLatin1String("yyyy-MM-dd HH:mm:ss"));
        r.file = (f.size() > 8) ? f.at(8) : QString();
        m_practice.append(r);
    }
}

void Store::save()
{
    QStringList cl;
    for (int i = 0; i < m_candidates.size(); ++i) {
        const Candidate &c = m_candidates.at(i);
        cl.append(QString::fromLatin1("%1\t%2\t%3\t%4\t%5")
                  .arg(c.no).arg(c.name).arg(c.idcard)
                  .arg(c.times).arg(c.state));
    }
    writeEncoded(candidatesFile(), cl);

    QStringList rl;
    for (int i = 0; i < m_records.size(); ++i) {
        const ExamRecord &r = m_records.at(i);
        rl.append(QString::fromLatin1("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\t%11\t%12")
                  .arg(r.no).arg(r.name).arg(r.idcard)
                  .arg(r.total).arg(r.correct).arg(r.wrong).arg(r.remain)
                  .arg(r.seconds).arg(r.speed).arg(r.accuracy)
                  .arg(r.when.toString(QLatin1String("yyyy-MM-dd HH:mm:ss")))
                  .arg(r.file));
    }
    writeEncoded(recordsFile(), rl);

    QStringList pl;
    for (int i = 0; i < m_practice.size(); ++i) {
        const PracticeRecord &r = m_practice.at(i);
        pl.append(QString::fromLatin1("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9")
                  .arg(r.no).arg(r.name).arg(r.total).arg(r.correct)
                  .arg(r.seconds).arg(r.speed).arg(r.accuracy)
                  .arg(r.when.toString(QLatin1String("yyyy-MM-dd HH:mm:ss")))
                  .arg(r.file));
    }
    writeEncoded(practiceFile(), pl);
}

Candidate *Store::find(const QString &no)
{
    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).no == no)
            return &m_candidates[i];
    }
    return 0;
}

bool Store::addCandidate(const Candidate &c, QString *err)
{
    if (c.no.isEmpty() || c.name.isEmpty() || c.idcard.isEmpty()) {
        if (err) *err = L("对不起，考号、姓名和身份证号均不能为空！");
        return false;
    }
    if (!isValidCandidateNo(c.no)) {
        if (err) *err = L("对不起，考号格式不正确！");
        return false;
    }
    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).no == c.no) {
            if (err) *err = L("对不起，考号不能重复！");
            return false;
        }
    }
    if (!isValidIdCard(c.idcard)) {
        if (err) *err = L("对不起，身份证号格式不正确，请仔细检查您的输入！");
        return false;
    }
    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).idcard == c.idcard) {
            if (err) *err = L("对不起，身份证号不能重复！");
            return false;
        }
    }
    m_candidates.append(c);
    save();
    return true;
}

bool Store::updateCandidate(const Candidate &c)
{
    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).no == c.no) {
            m_candidates[i] = c;
            save();
            return true;
        }
    }
    return false;
}

bool Store::removeCandidate(const QString &no, bool alsoRecords)
{
    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).no == no) {
            m_candidates.removeAt(i);
            if (alsoRecords)
                clearRecords(no);
            save();
            return true;
        }
    }
    return false;
}

void Store::clearCandidates()
{
    m_candidates.clear();
    save();
}

QList<ExamRecord> Store::recordsOf(const QString &no) const
{
    QList<ExamRecord> out;
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records.at(i).no == no)
            out.append(m_records.at(i));
    }
    return out;
}

void Store::addRecord(const ExamRecord &r)
{
    m_records.append(r);
    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).no == r.no) {
            m_candidates[i].times += 1;
            m_candidates[i].state = 0;
            break;
        }
    }
    save();
}

void Store::clearRecords()
{
    m_records.clear();
    for (int i = 0; i < m_candidates.size(); ++i)
        m_candidates[i].times = 0;
    save();
}

void Store::clearRecords(const QString &no)
{
    for (int i = m_records.size() - 1; i >= 0; --i) {
        if (m_records.at(i).no == no)
            m_records.removeAt(i);
    }
    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).no == no) {
            m_candidates[i].times = 0;
            m_candidates[i].state = 0;
        }
    }
    save();
}

int Store::examCount(const QString &no) const
{
    int n = 0;
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records.at(i).no == no)
            ++n;
    }
    return n;
}

ExamRecord Store::bestRecord(const QString &no) const
{
    ExamRecord best;
    bool has = false;
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records.at(i).no != no)
            continue;
        if (!has || m_records.at(i).speed > best.speed) {
            best = m_records.at(i);
            has = true;
        }
    }
    return best;
}

QList<PracticeRecord> Store::practiceOf(const QString &no) const
{
    QList<PracticeRecord> out;
    for (int i = 0; i < m_practice.size(); ++i) {
        if (m_practice.at(i).no == no)
            out.append(m_practice.at(i));
    }
    return out;
}

void Store::addPractice(const PracticeRecord &r)
{
    m_practice.append(r);
    save();
}

void Store::clearPractice()
{
    m_practice.clear();
    save();
}

void Store::clearPractice(const QString &no)
{
    for (int i = m_practice.size() - 1; i >= 0; --i) {
        if (m_practice.at(i).no == no)
            m_practice.removeAt(i);
    }
    save();
}

void Store::setState(const QString &no, int state)
{
    for (int i = 0; i < m_candidates.size(); ++i) {
        if (m_candidates.at(i).no == no) {
            m_candidates[i].state = state;
            break;
        }
    }
    save();
}

void Store::resetAllStates()
{
    for (int i = 0; i < m_candidates.size(); ++i)
        m_candidates[i].state = 0;
    save();
}
