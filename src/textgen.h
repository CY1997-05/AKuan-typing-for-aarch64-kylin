#ifndef TEXTGEN_H
#define TEXTGEN_H

#include <QString>
#include <QStringList>

// 随机练习文本生成器（对应原版“随机产生文字”右键菜单的 11 种类型）
class TextGen
{
public:
    enum Kind {
        WubiLevel2 = 0,   // 五笔二级简码汉字
        WubiLevel3,       // 五笔三级简码汉字
        WubiLevel4,       // 五笔四级简码汉字
        Word2,            // 两字词语
        Word3,            // 三字词语
        Word4,            // 四字词语
        Word5,            // 五字以上词语
        Common,           // 任意常用文字
        Letters,          // 纯字母
        Digits,           // 纯数字
        Mixed             // 字母数字混合
    };

    // 生成大约 chars 个字符
    static QString generate(int kind, int chars);

    // 正式测试默认文本（常用文字 ≈ 1500 字）
    static QString buildTestText(int chars = 1500);

    // 供“随机测试文本文件生成器”使用
    static bool generateToFile(int kind, int chars, const QString &fileName, QString *err);

    static QString kindName(int kind);
};

#endif // TEXTGEN_H
