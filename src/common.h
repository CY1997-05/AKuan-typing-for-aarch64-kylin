#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QStringList>

// 所有中文界面文本统一用 L("...") 包一层：
// 源码以 UTF-8(BOM) 保存，MSVC 会按执行字符集(GBK)编译，
// 因此这里用 fromLocal8Bit 还原，避免 Qt5 默认按 UTF-8 解释导致的乱码。
inline QString L(const char *s) { return QString::fromLocal8Bit(s); }

// 程序所在目录（含结尾斜杠）
QString appDir();

// 可写数据目录（默认就是程序目录）
QString dataDir();

// 读取 :/data/xxx.txt 之类的 UTF-8 资源
QString resourceText(const QString &resPath);
QStringList resourceLines(const QString &resPath);

// 秒 -> mm:ss
QString fmtTime(int seconds);

// 身份证号 / 考号 校验
bool isValidCandidateNo(const QString &no);
bool isValidIdCard(const QString &id);

#endif // COMMON_H
