#ifndef STORE_H
#define STORE_H

#include <QString>
#include <QList>
#include <QDateTime>

// 考生资料
struct Candidate
{
    QString no;        // 考号
    QString name;      // 姓名
    QString idcard;    // 身份证号
    int     times;     // 已考次数
    int     state;     // 0 正常 / 1 已登录 / 2 正在考试

    Candidate() : times(0), state(0) {}
};

// 考试（测试）成绩记录
struct ExamRecord
{
    QString no;
    QString name;
    QString idcard;
    int     total;     // 应录入
    int     correct;   // 录入正
    int     wrong;     // 录入错
    int     remain;    // 未打完
    int     seconds;   // 考试时长(秒)
    int     speed;     // 正确速度 (字/分钟)
    int     accuracy;  // 正确率 %
    QDateTime when;
    QString file;      // 考试文件

    ExamRecord() : total(0), correct(0), wrong(0), remain(0),
        seconds(0), speed(0), accuracy(0) {}
};

// 联网练习记录
struct PracticeRecord
{
    QString no;
    QString name;
    int     total;
    int     correct;
    int     seconds;
    int     speed;
    int     accuracy;
    QDateTime when;
    QString file;

    PracticeRecord() : total(0), correct(0), seconds(0), speed(0), accuracy(0) {}
};

class Store
{
public:
    static Store *instance();

    void load();
    void save();

    // 考生
    QList<Candidate> candidates() const { return m_candidates; }
    Candidate *find(const QString &no);
    bool addCandidate(const Candidate &c, QString *err);
    bool updateCandidate(const Candidate &c);
    bool removeCandidate(const QString &no, bool alsoRecords);
    void clearCandidates();

    // 考试记录
    QList<ExamRecord> records() const { return m_records; }
    QList<ExamRecord> recordsOf(const QString &no) const;
    void addRecord(const ExamRecord &r);
    void clearRecords();
    void clearRecords(const QString &no);
    int  examCount(const QString &no) const;
    ExamRecord bestRecord(const QString &no) const;

    // 练习记录
    QList<PracticeRecord> practiceRecords() const { return m_practice; }
    QList<PracticeRecord> practiceOf(const QString &no) const;
    void addPractice(const PracticeRecord &r);
    void clearPractice();
    void clearPractice(const QString &no);

    // 登录标志
    void setState(const QString &no, int state);
    void resetAllStates();

private:
    Store();
    static Store *s_instance;
    QList<Candidate> m_candidates;
    QList<ExamRecord> m_records;
    QList<PracticeRecord> m_practice;

    QString candidatesFile() const;
    QString recordsFile() const;
    QString practiceFile() const;
};

#endif // STORE_H
