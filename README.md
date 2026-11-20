# HZTest — 中文打字速度测试软件（Qt 5.5.1 复刻版）

本工程是把原版 **《阿宽软件系列之中文打字速度测试软件 V1.41（网络版）》**（`HZTest.exe`）
完整逆向还原成的一个 **Qt 5.5.1 / C++ / qmake** 工程，界面尺寸、控件位置、文案、词库、
评分口径与局域网考试流程均按原版还原。

![主界面](docs/screenshots/01_main_menu.png)

---

## 一、原版程序分析结论

| 项目 | 结论 |
|------|------|
| 可执行文件 | `HZTest.exe`，1.01 MB，PE32 / x86 / GUI |
| 编译器 | **Delphi / C++Builder（VCL）**，时间戳 `0x2A425E19`、链接器 2.25 是 Delphi 特征 |
| 保护 | 加壳（节区随机名 `r267t3iy`/`4esce9br`…，导入表只剩 kernel32/user32，代码段高熵） |
| 界面库 | **SUIPack 6.2**（`TsuiForm / TsuiButton / TsuiPanel / TsuiThemeManager`），皮肤资源 `BLUEGLASS_* / DEEPBLUE_* / WINXP_* / MACOS_*` |
| 网络 | 客户端/服务器组件（ICS `TWSocket`），**UDP 8849 广播发现 + TCP 8849 传数据** |
| 窗体 | 15 个（RCDATA 中的二进制 DFM：`TFMABOUT / TFMCANDIDATES / TFMFIRST / TFMGENTXT / TFMHINT / TFMLOGIN / TFMMAIN / TFMOPTION / TFMPASS / TFMREPORT / TFMRESULT / TFMSELECTIP / TFMSERVER / TFRMMGR / TFRMMSNPOPFORM`） |

> 说明：原程序**不是 Qt 程序**（是 Delphi + SUIPack 皮肤），本工程按用户要求用 Qt 5.5.1 重写，
> 并保持与原版一致的界面与功能。

### 逆向过程（产物都在仓库根目录上一级的 `_re/` 中）

1. `peinfo.py` / `pe2.py`：解析 PE 头、节表、资源目录，识别 Delphi + 加壳。
2. `dump.py`：启动 `HZTest.exe`，用 `ReadProcessMemory` 把**运行时已解压的 6.5 MB 镜像**整块读出来，
   重建为 `_re/unpacked.exe`（此时 `.rsrc` 中的真实资源、DFM、位图、字符串全部可读）。
3. `extract.py`：导出全部 268 个资源（15 个窗体 DFM、图标、位图、皮肤）。
4. `dfm.py`：按 FPC `TValueType` 规范实现二进制 DFM 解析器（`vaList / vaSet / vaCollection /
   vaBinary / vaString …`），把 15 个窗体解析成 `_re/dfm/*.dfm.txt` + `*.json`。
5. `inventory.py`：生成全部窗体的控件清单（类名/名称/坐标/尺寸/标题/字体），即
   `_re/inventory.txt`，本工程的界面布局直接按它逐个控件摆放。
6. `export_data.py`：把窗体里内置的**五笔词库**导出为 `data/*.txt`：
   二级简码 31 行、三级简码 199 行、四级简码 122 行、两字词 1496 行、三字词 386 行、
   四字词 1108 行、五字以上词 1109 行、常用字 94 行、范文 24 行。
7. `strings.py`：从镜像里提取全部中文文案（登录/考试/成绩/服务器日志/提示语），
   复刻版里的提示语尽量沿用原文。

---

## 二、工程结构

```
HZTest-Qt/
├── HZTest.pro                  qmake 工程（Qt 5.5.1 + MSVC2010 32bit）
├── build.bat                   一键编译脚本（qmake + nmake）
├── resources.qrc               内置词库与“关于”文本
├── data/                       从原版窗体中导出的词库
│   ├── wubi_level2/3/4.txt     五笔二/三/四级简码
│   ├── word2/3/4/5.txt         两/三/四/五字以上词语
│   ├── common.txt              常用单字
│   └── practice.txt sample.txt about.txt
├── portable/                   单文件版启动器源码（launcher.cpp / packer.cpp / portable.rc）
├── dist/                       单文件版输出：HZTestPortable.exe
├── build_portable.bat          一键生成单文件版
├── docs/screenshots/           运行截图
└── src/
    ├── main.cpp                入口：加载配置/考生库/主题
    ├── common.h/.cpp           L() 中文字符串、目录、时间格式化、考号/身份证校验
    ├── config.h/.cpp           测试设置（对应 fmOption），存 hztest.ini
    ├── store.h/.cpp            考生资料 + 考试记录 + 练习记录（异或+十六进制“加密”保存）
    ├── textgen.h/.cpp          随机文本生成（五笔简码/词组/字母/数字/混合）
    ├── typing.h/.cpp           打字评分引擎：逐字比对、计时、速度、正确率
    ├── theme.h/.cpp            3 套皮肤（深蓝/蓝玻璃/墨绿）+ QSS
    ├── suiform.h/.cpp          仿 SUIPack 无边框窗口（自绘标题栏、拖动、关闭按钮）
    ├── widgets.h/.cpp          控件工厂、消息框、打字行控件 TypeLine（含输入法预编辑）
    ├── forms/
    │   ├── firstform           fmFirst    主菜单（483×541）
    │   ├── optionform          fmOption   测试设置（889×604）
    │   ├── loginform           fmLogin    登录（310×237）
    │   ├── testform            fmMain     打字测试主界面（756×541）
    │   ├── resultform          fmResult   测试结果（517×468）
    │   ├── reportform          fmReport   查看测试/练习记录（769×574）
    │   ├── candidateform       fmCandidates 考生资料录入修改（887×519）
    │   ├── gentxtform          fmGenTxt   随机测试文本生成器（699×610）
    │   ├── selectipform        fmSelectIP 自动搜索考试服务器（409×413）
    │   ├── serverform          fmServer   考试服务器（728×583）
    │   ├── hintform            fmHint     信息提示（641×86）
    │   ├── aboutform           fmAbout    关于（528×563）
    │   └── passform            fmPass     密码输入（397×204）
    └── net/
        ├── lanserver.h/.cpp    UDP 广播应答 + TCP 登录/收成绩
        ├── lanclient.h/.cpp    UDP 搜索 + TCP 登录取卷/上传成绩
        └── lanprotocol.h       协议定义
```

## 三、构建与运行

环境（本机已验证）：

* Qt **5.5.1** msvc2010 32bit：`D:\Qt\Qt5.5.1\5.5\msvc2010`
* MSVC 2010（VC10）：`D:\Microsoft Visual Studio 10.0\VC\vcvarsall.bat`
* Windows 10 x64

```bat
cd HZTest-Qt
build.bat
```

`build.bat` 会依次执行 `vcvarsall x86` → `qmake` → 修补 nmake 的批量编译规则 → `nmake`，
产物为 `bin\HZTest.exe`。

> **为什么要修补 Makefile**：qmake 生成的 `{xxx}.cpp{yyy}.obj::` 是 nmake 的“批量推断规则”，
> 会把全部 .cpp 塞进**同一次 cl 调用**；在部分机器上 32 位 cl 连续编译 40 个 Qt 翻译单元会
> 内存耗尽并崩溃（`0xc0000005`，控制台刷满 `error D8030`）。`build.bat` 把 `::` 改成 `:`，
> 让每个文件单独调用 cl，即可稳定编译。

运行：`bin` 目录已经用 `windeployqt` 部署了 Qt5Core/Gui/Widgets/Network、平台插件和 MSVC 运行库，
**双击 `bin\HZTest.exe` 即可运行**（无需设置 PATH）。

### 单文件绿色版（拷到别的电脑直接双击）

`at
build_portable.bat
`

产物：**dist\HZTestPortable.exe（约 21 MB，就一个文件）**。它把

`
HZTest.exe + Qt5Core/Gui/Widgets/Network + platforms\qwindows.dll
+ msvcp100.dll + msvcr100.dll + libEGL/libGLESV2 + D3Dcompiler_43.dll + qt.conf
`

以 HZPK1 归档格式压进自己的 RCDATA 资源里（portable/packer.cpp 打包、
portable/launcher.cpp 解包），运行过程：

1. **首次运行**：把内嵌的运行库释放到 %LOCALAPPDATA%\HZTest\runtime\，写入 payload.stamp；
   之后运行（资源大小未变）直接用缓存，**不再重复释放**（实测第二次 0.2 秒出窗口）。
2. 通过环境变量 HZTEST_DATA_DIR 把**数据目录指到 exe 所在目录**，所以
   hztest.ini / candidates.dat / records.dat / practice.dat / server.log 都生成在这个单文件旁边
   —— 整个文件夹拷到 U 盘/其他电脑即可继续用，成绩不丢。
3. 若 exe 所在目录不可写（例如放在 Program Files），自动退回 %LOCALAPPDATA%\HZTest\data\。
4. 启动真正的 HZTest.exe 并等待其退出，返回同样的退出码。

> 目标机不需要安装 Qt、不需要 VC++ 运行库，也不需要设置 PATH —— 双击即可。
> 唯一的外部痕迹是 %LOCALAPPDATA%\HZTest\runtime 运行库缓存目录（可随时删除，下次运行会自动重建）。

**关于“真正的静态链接”**：本方案是“自解压 + 动态加载”，Qt 仍以 DLL 形式加载，只是这些 DLL
由程序自己释放，用户看不到。若要把 Qt 真正静态链接进 exe（exe 内不含任何 DLL），必须用
configure -static 重新编译整个 Qt 5.5.1（几小时、约 10 GB 磁盘），且 LGPL 下静态链接需要
提供可重链接的目标文件（或购买商业授权）。当前方案不改变 Qt 的链接方式，LGPL 合规、体积也小得多。

### 在 Linux 下构建运行（中文输入法适配）

本机（openKylin / aarch64）用的是自编译的 Qt 5.9.9（`/usr/local/Qt-5.9.9`）：

```bash
/usr/local/Qt-5.9.9/bin/qmake HZTest.pro
make -j4
./bin/HZTest
```

这套 Qt 的输入法前端插件只有 `compose` / `xim` / `ibus`，**没有 fcitx 前端**；而麒麟等国产系统
默认把 `QT_IM_MODULE` 设成 `fcitx`，于是 Qt 找不到任何可用的输入法前端 —— 打字界面里敲拼音
只会出英文字母，中文输入法等于失效。

程序启动时（构造 `QApplication` 之前）会自动做一次适配，见 `src/main.cpp` 的
`setupLinuxInputMethod()`：

1. 若当前 Qt 没有 fcitx 前端、但有 ibus 前端，且本机确实存在 IBus 服务
   （`~/.config/ibus/bus/` 下有地址文件，fcitx5 会写），就把 `QT_IM_MODULE` 设为 `ibus`。
   fcitx5 自带 IBus 前端，用户实际用的输入法、候选框、切换键全都不变，只是 Qt 与 fcitx5
   之间的通道换成了 IBus 协议。
2. Qt 的 ibus 前端是靠“PATH 里有没有 `ibus-daemon` 这条命令”判断 IBus 是否可用的。只装
   fcitx5 的机器上没有这条命令，所以构建时会把只用于存在性探测的空壳脚本
   （`data/im/ibus-daemon`）复制到 `bin/im/ibus-daemon`，运行时仅把它加进**本进程**的 PATH
   ——不改动系统，也不会被真正执行。

关掉这段适配用 `HZTEST_NO_IM_BRIDGE=1 ./bin/HZTest`；想自己指定输入法前端就设
`QT_IM_MODULE`（`fcitx` / `ibus` / `compose`）。若换成自带 fcitx 前端的 Qt（例如发行版
Qt 5.15），程序检测到 fcitx 插件后会自动跳过这段适配。

另外，`TypeLine`（自绘的输入条）补了几处自绘控件必须做的事，否则输入法候选框会跟着鼠标跑：
回答 `inputMethodQuery`（`ImCursorRectangle` / `ImCursorPosition` / `ImSurroundingText` 等），
并在光标移动、换行、获焦点后调用 `QInputMethod::update(Qt::ImCursorRectangle)` 通知输入法。

## 四、功能对照

| 原版 | Qt 版 | 说明 |
|------|-------|------|
| 主菜单 9 个按钮 | `FirstForm` | 按钮位置、文案、禁用逻辑一致；`换个界面` 循环 3 套皮肤 |
| 测试设置 | `OptionForm` | 三种模式（单机/客户机/服务器）、考试时长、文本文件、多次考试策略、限定输入法、联网练习选项、管理员密码 |
| 登录 | `LoginForm` | 考号校验、考生库验证、次数限制、考试状态（重复登录）检查、输入法枚举（读注册表 Keyboard Layouts） |
| 打字测试 | `TestForm` | 6 行 × 25 字/行，满 6 行翻页；正确字绿色、错误字红色、光标闪烁；支持中文输入法预编辑；进入界面后**直接按回车键即可开始练习并计时**（练习中按回车不会触发任何按钮） |
| 随机产生文字 | `TestForm` 右键菜单 | 11 种：五笔二/三/四级简码、两/三/四/五字以上词、常用字、纯字母、纯数字、混合 |
| 测试结果 | `ResultForm` | 考号/姓名/身份证/总字数/正确/错误/剩余/用时/正确速度/正确率 |
| 查看测试记录 | `ReportForm` | 12 列表格，导出 TXT / Excel（SpreadsheetML），清除记录 |
| 联网练习记录 | `ReportForm`（模式 1） | 按考生查看历次练习，导出 XLS、清除记录 |
| 考生资料 | `CandidateForm` | 增删改、随机身份证号、批量生成、清考试记录、恢复登录标志、XLS 导入导出 |
| 随机文本生成器 | `GenTxtForm` | 生成字数、包含全角数字/字母/标点/单字/各类词组，预览与保存 |
| 考试服务器 | `ServerForm` | 启动 TCP/UDP 8849，实时日志（同时写 `server.log`）、考生数、中止考试并查看结果 |
| 自动搜索服务器 | `SelectIPForm` | UDP 广播 `HZTEST_FIND`，3.5 秒收集应答 IP |
| 关于 | `AboutForm` | 原文（含联系方式）逐字保留 |

## 五、评分口径

与原版一致，按“正确字数 / 分钟”计算：

```
正确速度 = 正确字数 × 60 / 用时秒          （四舍五入）
正确率   = 正确字数 × 100 / (正确 + 错误)
总字数   = 本次下发的文本总字数
未打完   = 总字数 - 已输入字数
```

* 用时从**第一次击键**开始计时，到 `限定分钟` 或文本打完为止。
* 服务端只保存 **用时 ≥ 60 秒** 的联网成绩（原版日志中的那句“小于1分钟，数据未保存”）。
* 同一考号多次考试：`只允许一次` / `取最后一次` / `取最好成绩` 三种策略。

## 六、局域网协议（复刻版自定义，用于替代原版 ICS 私有协议）

```
UDP 8849
  客户机 -> 255.255.255.255:8849   "HZTEST_FIND"
  服务器 -> 客户机                  "HZTEST_SERVER <机器名>"

TCP 8849（按行，\t 分隔，文本字段 Base64(UTF-8)）
  客户机 -> "LOGIN\t<考号>\t<exam|practice>"
  服务器 -> "OK\t<姓名>\t<身份证>\t<分钟>\t<Base64 文本>"
  服务器 -> "ERR\t<Base64 原因>"
  客户机 -> "RESULT\t<考号>\t<姓名>\t<身份证>\t<应录入>\t<正确>\t<错误>\t<未打完>\t
             <秒>\t<速度>\t<正确率>\t<Base64 文件名>"
  服务器 -> "ACK" / "ERR\t..."
  客户机 -> "QUIT"
```

服务器端会校验：考号是否存在、是否已登录、考试次数是否超限、成绩是否完整、时长是否 < 60 秒，
并把每条事件写入界面日志与 `server.log`。

## 七、数据文件

运行目录（默认 `bin\`）下自动生成：

| 文件 | 内容 | 保护方式 |
|------|------|----------|
| `hztest.ini` | 全部设置（QSettings INI） | 明文，便于排查；原版为二进制加密 |
| `candidates.dat` | 考号/姓名/身份证/考试次数/登录标志 | 逐字节异或 + 十六进制 |
| `records.dat` | 考试成绩记录 | 同上 |
| `practice.dat` | 联网练习记录 | 同上 |
| `server.log` | 服务器运行日志（GBK） | 明文 |

## 八、与 1.41 原版的差异（有意为之）

1. **加壳/注册机制不还原**：原版联网功能需要注册码，复刻版直接可用。
2. **界面风格按“无皮肤”版原版还原**：标准 Windows 边框 + 浅灰客户区 + 银色按钮；
   打字区是“上方黑色目标文字 + 下方蓝色(0,128,192)输入条”，
   与用户提供的原版实际运行截图一致（`换个界面` 仍保留 3 套配色）。
3. **联网协议自定**：与原版不互通（原版是 ICS 私有报文，且被加壳无法完整还原）。
4. **Excel 导出**：原版用 OLE 调 Excel，复刻版写 Excel 2003 XML（`.xls`，Excel/WPS 可直接打开）。
5. `五笔一级简码` 原版菜单里没有，复刻版同样只提供二/三/四级。
6. **回车键开始**：打字界面进入后焦点就在第一条输入条上，直接按回车即可开始练习并计时
   （原版必须先点“开始练习”）。练习进行中按回车被吞掉，不会误触“重选练习文章”等按钮。

## 九、界面还原说明（对照原版实际运行效果）

* **窗口**：使用系统标准边框（原生标题栏 + 最小化/最大化/关闭），客户区尺寸严格等于
  DFM 中的 `ClientWidth × ClientHeight`（主菜单 483×541、登录 310×237、测试 756×541…），
  控件坐标也直接用 DFM 的 `Left/Top/Width/Height`，所以布局与原版逐像素对齐。
* **配色**：客户区 `#F0F0F0`，按钮为银灰渐变 + 1px 深灰描边，输入框白底黑字，
  强调文字用红色楷体（与原版 `clRed + 楷体_GB2312` 一致）。
* **打字区**：每行节距 67px —— 上方 30px 是**黑色目标文字**（由窗体自绘），
  下方 30px 是**蓝色输入条**（`RGB(0,128,192)`，即原版 TEdit 的 `Color = 12615680`）。
  输入条里显示的是**你实际敲进去的字符**：打对的显示为白色，打错的**原样显示你输错的那个字**并标红
  （不是把目标文字标红），光标为黄色竖线。
* **状态栏**：`第 x/y 页,第 z/总字数 字`、`已用时间 mm:ss`、`当前速度 n 字/分钟`，
  文案格式与原版一致。
* **字体**：宋体（正文 15px、按钮 16px、打字区 29px），楷体（红色标签 19px、标题 24px），
  标题按 DFM 的像素高度设置。

## 十、截图

| 主界面 | 登录 |
|---|---|
| ![主界面](docs/screenshots/01_main_menu.png) | ![登录](docs/screenshots/02_login.png) |

| 打字测试（未开始） | 打字测试（输入中：蓝色输入条内显示已输入的字） |
|---|---|
| ![测试](docs/screenshots/03_test_screen.png) | ![输入中](docs/screenshots/13_test_typing.png) |

| 测试结果 | 测试设置 |
|---|---|
| ![结果](docs/screenshots/04_result.png) | ![设置](docs/screenshots/05_options.png) |

| 考生资料 | 随机文本生成器 |
|---|---|
| ![考生](docs/screenshots/06_candidates.png) | ![生成器](docs/screenshots/07_text_generator.png) |

| 考试服务器 | 联网考试（客户机） |
|---|---|
| ![服务器](docs/screenshots/08_lan_server.png) | ![客户机](docs/screenshots/09_lan_client.png) |

| 测试记录 | 关于 |
|---|---|
| ![记录](docs/screenshots/10_records.png) | ![关于](docs/screenshots/11_about.png) |

| 自动搜索服务器 | 密码输入 |
|---|---|
| ![搜索](docs/screenshots/12_select_ip.png) | ![密码](docs/screenshots/14_password.png) |

> 想自己重新生成这批截图：`HZTest.exe --shots <输出目录>`（依次打开各界面抓图后退出）。
