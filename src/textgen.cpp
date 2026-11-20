#include "textgen.h"
#include "common.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QChar>

static QVector<QChar> charPool(const QString &res)
{
    QVector<QChar> pool;
    const QStringList lines = resourceLines(res);
    for (int i = 0; i < lines.size(); ++i) {
        const QString &s = lines.at(i);
        for (int k = 0; k < s.size(); ++k) {
            const QChar c = s.at(k);
            if (!c.isSpace())
                pool.append(c);
        }
    }
    return pool;
}

static QStringList wordPool(const QString &res)
{
    QStringList words;
    const QStringList lines = resourceLines(res);
    for (int i = 0; i < lines.size(); ++i) {
        // 原版词库每行是一长串词，按词长切分由调用方决定；
        // 这里把整行当作候选串，取词时按 wordLen 截取。
        words.append(lines.at(i));
    }
    return words;
}

static int rnd(int n)
{
    if (n <= 0)
        return 0;
    return qrand() % n;
}

static QString pickFromChars(const QVector<QChar> &pool, int chars)
{
    QString out;
    if (pool.isEmpty())
        return out;
    out.reserve(chars);
    for (int i = 0; i < chars; ++i)
        out.append(pool.at(rnd(pool.size())));
    return out;
}

static QString pickWords(const QStringList &lines, int wordLen, int chars)
{
    QString out;
    if (lines.isEmpty())
        return out;
    while (out.size() < chars) {
        const QString &line = lines.at(rnd(lines.size()));
        if (line.size() < wordLen)
            continue;
        const int maxStart = line.size() - wordLen;
        const int start = rnd(maxStart + 1);
        out.append(line.mid(start, wordLen));
        // 词语之间偶尔加一个空格，便于阅读
        if (rnd(10) == 0)
            out.append(QLatin1Char(' '));
    }
    return out.left(chars);
}

QString TextGen::generate(int kind, int chars)
{
    if (chars < 10)
        chars = 10;

    switch (kind) {
    case WubiLevel2:
        return pickFromChars(charPool(QLatin1String(":/data/wubi_level2.txt")), chars);
    case WubiLevel3:
        return pickFromChars(charPool(QLatin1String(":/data/wubi_level3.txt")), chars);
    case WubiLevel4:
        return pickFromChars(charPool(QLatin1String(":/data/wubi_level4.txt")), chars);
    case Word2:
        return pickWords(wordPool(QLatin1String(":/data/word2.txt")), 2, chars);
    case Word3:
        return pickWords(wordPool(QLatin1String(":/data/word3.txt")), 3, chars);
    case Word4:
        return pickWords(wordPool(QLatin1String(":/data/word4.txt")), 4, chars);
    case Word5:
        return pickWords(wordPool(QLatin1String(":/data/word5.txt")), 5, chars);
    case Common:
        return pickFromChars(charPool(QLatin1String(":/data/common.txt")), chars);
    case Letters: {
        QString out;
        const char *abc = "abcdefghijklmnopqrstuvwxyz";
        for (int i = 0; i < chars; ++i)
            out.append(QLatin1Char(abc[rnd(26)]));
        return out;
    }
    case Digits: {
        QString out;
        for (int i = 0; i < chars; ++i)
            out.append(QLatin1Char(static_cast<char>('0' + rnd(10))));
        return out;
    }
    case Mixed: {
        const QString pool = QLatin1String("abcdefghijklmnopqrstuvwxyz0123456789");
        QString out;
        for (int i = 0; i < chars; ++i)
            out.append(pool.at(rnd(pool.size())));
        return out;
    }
    default:
        break;
    }
    return QString();
}

QString TextGen::buildTestText(int chars)
{
    // 原版默认使用“任意常用文字”+少量词语，保证文本可读
    const int head = chars * 7 / 10;
    QString text = generate(Common, head);
    text += generate(Word2, chars - head);
    return text.left(chars);
}

bool TextGen::generateToFile(int kind, int chars, const QString &fileName, QString *err)
{
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (err) *err = L("无法写入文件，请检查路径和权限！");
        return false;
    }
    QTextStream ts(&f);
    ts.setCodec("UTF-8");
    const QString text = generate(kind, chars);
    // 每 40 字换一行，方便打印与阅读
    int i = 0;
    while (i < text.size()) {
        ts << text.mid(i, 40) << QLatin1String("\r\n");
        i += 40;
    }
    ts.flush();
    return true;
}

QString TextGen::kindName(int kind)
{
    switch (kind) {
    case WubiLevel2: return L("随机产生五笔字型二级简码汉字");
    case WubiLevel3: return L("随机产生五笔字型三级简码汉字");
    case WubiLevel4: return L("随机产生五笔字型四级简码汉字");
    case Word2:      return L("随机产生两字词语练习");
    case Word3:      return L("随机产生三字词语练习");
    case Word4:      return L("随机产生四字词语练习");
    case Word5:      return L("随机产生五字以上词语练习");
    case Common:     return L("随机产生任意常用文字练习");
    case Letters:    return L("随机产生纯字母练习");
    case Digits:     return L("随机产生纯数字练习");
    case Mixed:      return L("随机产生字母和数字混合文字练习");
    default: break;
    }
    return QString();
}
