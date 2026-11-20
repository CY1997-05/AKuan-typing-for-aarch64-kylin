#ifndef LANPROTOCOL_H
#define LANPROTOCOL_H

// 局域网考试协议（自定义，UDP 发现 + TCP 文本行）
//
// UDP 8849：
//   客户机 -> 广播  "HZTEST_FIND"
//   服务器 -> 单播  "HZTEST_SERVER <机器名>"
//
// TCP 8849（每行以 \n 结束，字段以 \t 分隔，文本字段用 Base64 编码）：
//   客户机 -> "LOGIN\t<考号>\t<exam|practice>"
//   服务器 -> "OK\t<姓名>\t<身份证号>\t<分钟数>\t<Base64 文本>"
//   服务器 -> "ERR\t<Base64 错误信息>"
//   客户机 -> "RESULT\t<考号>\t<姓名>\t<身份证号>\t<应录入>\t<正确>\t<错误>\t
//              <未打完>\t<秒>\t<速度>\t<正确率>\t<Base64 文件名>"
//   服务器 -> "ACK" / "ERR\t..."
//   客户机 -> "QUIT"

#define HZTEST_PORT 8849
#define HZTEST_FIND    "HZTEST_FIND"
#define HZTEST_SERVER  "HZTEST_SERVER"

#endif // LANPROTOCOL_H
