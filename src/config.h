#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

// 对应原版的“测试设置”(fmOption) 全部选项。
// 原版把配置加密保存在程序目录，这里采用同样的思路：
// 简单异或 + 十六进制，避免人工直接修改。
class AppConfig
{
public:
    enum Mode { ModeSingle = 0, ModeClient = 1, ModeServer = 2 };

    static AppConfig *instance();

    void load();
    void save();

    // 运行模式
    int  mode;
    QString serverIp;

    // 单机考试选项
    int  testMinutes;      // 1..240
    QString testFile;      // 指定测试文本文件
    int  multiMode;        // 0 只允许一次 / 1 取最后一次 / 2 取最好成绩
    int  maxTimes;         // 1..99
    bool defineIme;        // 限定输入法
    QString imeName;

    // 联网练习扩展选项
    int  practiceMinutes;  // 1..999
    bool practiceRandom;   // 电脑随机生成联网练习文本
    bool practiceUseFolder; // 指定联网练习文章存放文件夹
    QString practiceFolder;

    // 密码选项
    bool needPassword;
    QString adminPassword;

    // 界面
    int  themeIndex;

    // 上次登录的考号
    QString lastName;

private:
    AppConfig();
    static AppConfig *s_instance;
    QString fileName() const;
};

#endif // CONFIG_H
