#include "testform.h"
#include "widgets.h"
#include "theme.h"
#include "config.h"
#include "common.h"
#include "store.h"
#include "textgen.h"
#include "resultform.h"
#include "net/lanclient.h"

#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QMenu>
#include <QPainter>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>
#include <QDateTime>
#include <QCursor>
#include <QKeyEvent>
#include <QShowEvent>


TestForm::TestForm(QWidget *parent, int mode, const QString &no, const QString &name,
                   const QString &idcard, const QString &text, int minutes, bool online)
    : SuiForm(parent)
    , m_mode(mode)
    , m_no(no)
    , m_name(name)
    , m_idcard(idcard)
    , m_minutes(minutes)
    , m_online(online)
    , m_running(false)
    , m_ended(false)
{
    setModal(false);
    setShowTitleBar(false);
    setClientSize(756, 541);
    setWindowTitle(L("中文打字速度测试软件"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    m_engine.setMaxSeconds(minutes * 60);

    setupUi();

    QString t = text;
    if (t.isEmpty()) {
        if (m_mode == TestForm::Exam) {
            AppConfig *cfg = AppConfig::instance();
            if (!cfg->testFile.isEmpty()) {
                QFile f(cfg->testFile);
                if (f.open(QIODevice::ReadOnly)) {
                    QTextStream ts(&f);
                    ts.setCodec("GBK");
                    t = ts.readAll();
                    m_fileName = cfg->testFile;
                }
            }
            if (t.trimmed().isEmpty()) {
                t = TextGen::buildTestText(1500);
                m_fileName = L("随机生成的测试文本");
            }
        } else {
            t = resourceText(QLatin1String(":/data/practice.txt"));
            t.replace(QLatin1Char('\n'), QString());
            t.replace(QLatin1Char('\r'), QString());
            m_fileName = L("练习范文");
        }
    } else if (m_fileName.isEmpty()) {
        m_fileName = L("服务器下发的测试文本");
    }

    applyText(t, m_fileName);

    m_timer = new QTimer(this);
    m_timer->setInterval(200);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTick()));
}

void TestForm::setupUi()
{
    // 打字区：6 行，每行 = 上方目标文字 + 下方蓝色输入条
    for (int i = 0; i < 6; ++i) {
        TypeLine *line = new TypeLine(body(), &m_engine, i);
        line->setGeometry(16, 40 + 67 * i, 725, 30);
        connect(line, SIGNAL(charsTyped(QString)), this, SLOT(onChars(QString)));
        connect(line, SIGNAL(backspacePressed()), this, SLOT(onBackspace()));
        connect(line, SIGNAL(clicked()), this, SLOT(onLineClicked()));
        m_lines.append(line);
    }

    // 信息面板
    m_paInfo = new QWidget(body());
    m_paInfo->setGeometry(8, 409, 741, 125);
    m_paInfo->setAutoFillBackground(false);

    m_btStart = mkButton(m_paInfo, L("开始练习"), 42, 4, 137, 26);
    QPushButton *btLoad = mkButton(m_paInfo, L("重选练习文章"), 42, 34, 137, 26);
    QPushButton *btRandom = mkButton(m_paInfo, L("随机产生文字"), 42, 64, 137, 26);
    m_btEnd = mkButton(m_paInfo, L("结束练习"), 42, 94, 137, 26);
    m_btEnd->setEnabled(false);

    mkRedLabel(m_paInfo, L("当前考号："), 192, 2, 106, 19);
    m_lbKh = mkRedLabel(m_paInfo, m_no.isEmpty() ? L("000") : m_no, 298, 0, 394, 20);

    mkRedLabel(m_paInfo, L("考生姓名："), 192, 22, 106, 19);
    m_lbXm = mkRedLabel(m_paInfo, m_name, 298, 20, 382, 19);

    mkRedLabel(m_paInfo, L("身份证号："), 192, 42, 106, 19);
    m_lbSfz = mkRedLabel(m_paInfo, m_idcard, 298, 40, 382, 19);

    mkRedLabel(m_paInfo, L("已用时间："), 192, 61, 106, 19);
    m_lbTime = mkRedLabel(m_paInfo, L("00:00"), 298, 59, 81, 26);

    mkRedLabel(m_paInfo, L("打字信息："), 192, 81, 106, 19);
    m_lbPage = mkRedLabel(m_paInfo, L("第 1/1 页,第 0/0 字"), 298, 81, 400, 19);

    mkRedLabel(m_paInfo, L("当前速度："), 192, 101, 106, 19);
    m_lbSpeed = mkRedLabel(m_paInfo, L("0字/分钟"), 298, 102, 200, 19);

    QString envText;
    if (m_mode == TestForm::Exam)
        envText = L("正式考试环境");
    else if (m_mode == TestForm::NetPractice)
        envText = L("联网练习环境");
    else
        envText = L("本地练习环境");
    m_lbTest = mkRedLabel(m_paInfo, envText, 12, 5, 25, 113, 17);
    m_lbTest->setWordWrap(true);
    m_lbTest->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    // 提示框
    QWidget *paHint = new QWidget(m_paInfo);
    paHint->setGeometry(533, 64, 201, 54);
    paHint->setStyleSheet(QLatin1String("QWidget { background: #FFFFFF; border: 1px solid #808080; }"));
    QLabel *lbHint = new QLabel(L("输入法切换键[Ctrl+Shift]\n全半角切换键[Shift+空格]"), paHint);
    lbHint->setGeometry(4, 4, 193, 46);
    QFont hf;
    hf.setFamily(L("黑体"));
    hf.setPixelSize(13);
    lbHint->setFont(hf);
    QPalette hp = lbHint->palette();
    hp.setColor(QPalette::WindowText, QColor(0xFF, 0x00, 0x00));
    lbHint->setPalette(hp);
    lbHint->setStyleSheet(QLatin1String("QLabel { background: transparent; border: none; }"));
    lbHint->setWordWrap(true);

    // 这四个按钮不要抢回车键：QDialog 默认会把回车送给 autoDefault 按钮，
    // 会让“打字时按回车”变成弹文件对话框。回车键统一由 keyPressEvent 处理。
    m_btStart->setAutoDefault(false);
    btLoad->setAutoDefault(false);
    btRandom->setAutoDefault(false);
    m_btEnd->setAutoDefault(false);

    connect(m_btStart, SIGNAL(clicked()), this, SLOT(onStart()));
    connect(m_btEnd, SIGNAL(clicked()), this, SLOT(onEnd()));
    connect(btLoad, SIGNAL(clicked()), this, SLOT(onLoadDoc()));
    connect(btRandom, SIGNAL(clicked()), this, SLOT(onRandomMenu()));
}

void TestForm::applyText(const QString &text, const QString &fileName)
{
    QString t = text;
    t.replace(QLatin1Char('\r'), QString());
    t.replace(QLatin1Char('\n'), QString());
    t.replace(QChar(0x3000), QLatin1Char(' '));
    if (t.isEmpty())
        t = TextGen::buildTestText(600);

    m_fileName = fileName;
    m_engine.setText(t);
    refreshView();
    refreshInfo();
}

void TestForm::paintEvent(QPaintEvent *e)
{
    SuiForm::paintEvent(e);

    const SuiTheme t = Theme::instance()->current();
    QPainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // 打字区背景
    const QRect article(9, 8, 739, 401);
    p.fillRect(article, t.articleBg);
    p.setPen(QPen(QColor(0x80, 0x80, 0x80), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(article.adjusted(0, 0, -1, -1));

    // 每行目标文字（画在蓝色输入条上方，与原版 FormPaint 一致）
    p.setFont(uiFont(29));
    p.setPen(t.articleText);
    for (int i = 0; i < m_lines.size(); ++i) {
        const QString line = m_engine.lineText(m_lines.at(i)->lineIndex());
        if (line.isEmpty())
            continue;
        p.drawText(QRect(16, 8 + 67 * i, 725, 30), Qt::AlignLeft | Qt::AlignVCenter, line);
    }

    // 输入条下沿的立体分隔线
    for (int i = 0; i < 6; ++i) {
        p.setPen(QColor(0xA0, 0xA0, 0xA0));
        p.drawLine(9, 71 + 67 * i, 747, 71 + 67 * i);
        p.setPen(QColor(0xFF, 0xFF, 0xFF));
        p.drawLine(9, 72 + 67 * i, 747, 72 + 67 * i);
    }

    // 信息面板
    const QRect info(8, 409, 741, 125);
    p.fillRect(info, t.clientBottom);
    p.setPen(QPen(QColor(0x80, 0x80, 0x80), 1));
    p.drawRect(info.adjusted(0, 0, -1, -1));

    // 外框
    p.setPen(QPen(QColor(0x80, 0x80, 0x80), 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}

void TestForm::refreshView()
{
    const int page = m_engine.pageOf(m_engine.pos());
    for (int i = 0; i < m_lines.size(); ++i) {
        m_lines[i]->setLineIndex(page * 6 + i);
        m_lines[i]->update();
    }
    update();
}

void TestForm::refreshInfo()
{
    m_lbTime->setText(fmtTime(m_engine.elapsed()));
    m_lbPage->setText(QString::fromLatin1("%1 %2/%3 %4,%5 %6/%7 %8")
                      .arg(L("第")).arg(m_engine.pageOf(m_engine.pos()) + 1)
                      .arg(m_engine.pageCount())
                      .arg(L("页"))
                      .arg(L("第")).arg(m_engine.pos()).arg(m_engine.total())
                      .arg(L("字")));
    m_lbSpeed->setText(QString::fromLatin1("%1%2").arg(m_engine.speed()).arg(L("字/分钟")));
}

void TestForm::focusActiveLine()
{
    const int line = m_engine.lineOf(m_engine.pos());
    const int idx = line % 6;
    if (idx >= 0 && idx < m_lines.size()) {
        m_lines[idx]->setFocus(Qt::OtherFocusReason);
        m_lines[idx]->update();
    }
}

void TestForm::showEvent(QShowEvent *e)
{
    SuiForm::showEvent(e);
    // 一进入打字界面就把焦点放到第一条输入条上：
    // 这样输入法立刻就能用，而且不用先点鼠标，直接按回车就能开始。
    if (!m_running && !m_ended)
        focusActiveLine();
}

void TestForm::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        // 回车键 = 开始练习并计时（未开始时）；练习中按回车则什么都不做，
        // 只把事件吃掉，避免触发对话框的默认按钮。
        if (!m_running && !m_ended)
            onStart();
        e->accept();
        return;
    }
    SuiForm::keyPressEvent(e);
}

void TestForm::onStart()
{
    if (m_running)
        return;
    m_engine.restart();
    m_engine.setMaxSeconds(m_minutes * 60);
    m_running = true;
    m_ended = false;
    m_btStart->setEnabled(false);
    m_btEnd->setEnabled(true);
    m_timer->start();
    refreshView();
    refreshInfo();
    focusActiveLine();
}

void TestForm::onEnd()
{
    if (!m_running) {
        close();
        return;
    }
    if (!suiConfirm(this, L("您确定要结束本次测试吗？"), L("信息提示")))
        return;
    m_engine.finish();
    endTest();
}

void TestForm::onLoadDoc()
{
    if (m_running) {
        suiInfo(this, L("测试正在进行中，请先结束测试再重选文章。"));
        return;
    }
    const QString fn = QFileDialog::getOpenFileName(this, L("请选择练习文章"), QString(),
                                                    L("文本文件(*.txt);;所有文件(*.*)"));
    if (fn.isEmpty())
        return;
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) {
        suiInfo(this, L("无法读取该文件！"));
        return;
    }
    QTextStream ts(&f);
    ts.setCodec("GBK");
    QString text = ts.readAll();
    if (text.trimmed().isEmpty()) {
        suiInfo(this, L("该文件内容为空！"));
        return;
    }
    applyText(text, fn);
    suiInfo(this, L("选择文本文件成功！请进入练习！"));
}

void TestForm::onRandomMenu()
{
    QMenu menu(this);
    const char *names[11] = {
        "随机产生五笔字型二级简码汉字", "随机产生五笔字型三级简码汉字",
        "随机产生五笔字型四级简码汉字", "随机产生两字词语练习",
        "随机产生三字词语练习", "随机产生四字词语练习",
        "随机产生五字以上词语练习", "随机产生任意常用文字练习",
        "随机产生纯字母练习", "随机产生纯数字练习",
        "随机产生字母和数字混合文字练习"
    };
    for (int i = 0; i < 11; ++i) {
        if (i == 8)
            menu.addSeparator();
        QAction *a = menu.addAction(L(names[i]));
        a->setData(i);
    }
    QAction *chosen = menu.exec(QCursor::pos());
    if (!chosen)
        return;
    const int kind = chosen->data().toInt();
    const QString text = TextGen::generate(kind, 600);
    applyText(text, TextGen::kindName(kind));
}

void TestForm::onTick()
{
    if (!m_running)
        return;
    refreshInfo();
    if (m_engine.isFinished()) {
        endTest();
        return;
    }
    static int lastLine = -1;
    const int line = m_engine.lineOf(m_engine.pos());
    if (line != lastLine) {
        lastLine = line;
        refreshView();
        focusActiveLine();
    }
}

void TestForm::onChars(const QString &s)
{
    if (!m_running)
        return;
    for (int i = 0; i < s.size(); ++i) {
        if (m_engine.isFinished())
            break;
        m_engine.inputChar(s.at(i));
    }
    refreshView();
    refreshInfo();
    if (m_engine.isFinished())
        endTest();
}

void TestForm::onBackspace()
{
    if (!m_running)
        return;
    m_engine.backspace();
    refreshView();
    refreshInfo();
}

void TestForm::onLineClicked()
{
    if (!m_running) {
        onStart();
        return;
    }
    focusActiveLine();
}

void TestForm::endTest()
{
    if (m_ended)
        return;
    m_ended = true;
    m_running = false;
    m_timer->stop();
    m_btStart->setEnabled(true);
    m_btEnd->setEnabled(false);
    m_engine.finish();

    const int total = m_engine.total();
    const int correct = m_engine.correct();
    const int wrong = m_engine.wrong();
    const int remain = m_engine.remain();
    const int seconds = m_engine.elapsed();
    const int speed = m_engine.speed();
    const int acc = m_engine.accuracy();

    QString summary;
    if (m_mode == TestForm::Exam) {
        ExamRecord r;
        r.no = m_no;
        r.name = m_name;
        r.idcard = m_idcard;
        r.total = total;
        r.correct = correct;
        r.wrong = wrong;
        r.remain = remain;
        r.seconds = seconds;
        r.speed = speed;
        r.accuracy = acc;
        r.when = QDateTime::currentDateTime();
        r.file = m_fileName;
        Store::instance()->addRecord(r);
        if (m_online)
            summary = L("您的成绩已经上传到考试服务器，请放心。");
        else
            summary = L("您的成绩已经安全记录在案，请放心。");
    } else {
        PracticeRecord r;
        r.no = m_no;
        r.name = m_name;
        r.total = total;
        r.correct = correct;
        r.seconds = seconds;
        r.speed = speed;
        r.accuracy = acc;
        r.when = QDateTime::currentDateTime();
        r.file = m_fileName;
        Store::instance()->addPractice(r);
        summary = L("本次练习成绩已记录，继续加油！");
    }

    uploadIfNeeded(total, correct, wrong, remain, seconds, speed, acc);

    ResultForm dlg(this, m_no, m_name, m_idcard, total, correct, wrong, remain,
                   seconds, speed, acc, summary);
    dlg.exec();

    emit finished();
    close();
}

void TestForm::uploadIfNeeded(int total, int correct, int wrong, int remain,
                              int seconds, int speed, int accuracy)
{
    if (!m_online)
        return;
    if (seconds < 60) {
        suiInfo(this, L("本次测试时间小于1分钟，成绩不上传。"));
        return;
    }
    LanClient client(this);
    QString err;
    if (!client.uploadResult(m_no, m_name, m_idcard, total, correct, wrong,
                             remain, seconds, speed, accuracy, m_fileName, &err)) {
        suiInfo(this, L("上传联网练习或测试成绩失败！请检查网络或联系技术人员查找原因。\n") + err);
    }
}

void TestForm::closeEvent(QCloseEvent *e)
{
    if (m_running && !m_ended) {
        if (!suiConfirm(this, L("测试正在进行中，确定要退出吗？退出后本次成绩将不被记录。"), L("信息提示"))) {
            e->ignore();
            return;
        }
        m_timer->stop();
        m_running = false;
    }
    if (!m_ended)
        emit finished();
    e->accept();
}
