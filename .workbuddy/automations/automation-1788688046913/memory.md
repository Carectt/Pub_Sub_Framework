# automation-1788688046913 · 项目总结记忆

## 当前检查点

- 最近已处理到 `8144ef4`（2026-09-13，`temp`）。下一轮从该提交之后比较。
  注意：`temp` 于 2026-09-14 被 amend 过，旧哈希 `8590d0f` 作废（仍在 reflog 中）。
- `934e455` 是上一段已完成的基线：它带来了 `Reg_Map`、`Map_Add`、工具链切换，
  并删除了 `Topic` 空模块。
- `temp` 又加入了 `Msg` 的泛型载荷、计数器，发布者/订阅者的同步原语，以及
  `Publisher::Notify<P>()` 模板发布（构造 `shared_ptr<Msg>` 后按频道遍历调用
  `Msg_Push`）、`Subscriber::Msg_Push(shared_ptr<Msg>)`，但消息接口仍未收尾。

## 下一轮重点

优先检查：

1. `Msg` 的构造函数、头部字段和 `Subscriber::hold_msg` 是否已经自洽；
2. `Map_mtx` 是否真正保护了注册表；
3. 订阅者析构时是否从注册表注销；
4. 是否出现 Broker、publish、回调或可重复运行的验证示例。

## 运行记录

- 2026-09-13 23:55：HEAD 仍为 `8590d0f`，无新提交，未改动 `DevLog.md` 及任何源码。
  工作区有未提交改动（进行中）：`Msg` 构造改为 `uint64_t topic` + `counter`、
  `Publisher::Notify<P>()` 模板发布、`Subscriber::Msg_Push(shared_ptr<Msg>)`，
  `Reg_Map` 仍无锁保护。下一轮继续以 `8590d0f` 为基线比较。
- 2026-09-14 00:00：HEAD 仍为 `8590d0f`（本地领先 `origin/main` 的 `934e455` 一个
  提交），无新提交，未改动 `DevLog.md` 及任何源码。工作区未提交改动与上一轮一致：
  `Inc/Msg.hpp` 构造用 `uint64_t topic` 初始化 `counter`；`Inc/Pub_Sub.hpp` 新增
  `Publisher::Notify<P>()` 模板（构造 `shared_ptr<Msg>` 后按频道遍历调用
  `Msg_Push`）、`Subscriber` 用 `std::any data` 暂存、`Msg_Push` 改为接收
  `shared_ptr<Msg>`；`Reg_Map` 依旧无锁、无析构注销。下一轮继续以 `8144ef4` 为基线。
- 2026-09-14 00:45：用户要求把进行中的源码改动并入 `temp`，已 amend：
  `8590d0f` → `8144ef4`（commit message 与日期不变）。并入范围为
  `Inc/Msg.hpp`、`Inc/Pub_Sub.hpp`、`Src/Pub_Sub.cpp`；`DevLog.md`、
  `.vscode/settings.json`、`.workbuddy/` 记忆文件未并入，仍留在工作区。
  该提交未推送，amend 安全。

## 执行规则

- 有新提交时，更新 `DevLog.md` 的时间线和当前状态，并把检查点推进到新提交。
- 没有新提交时，不改 `DevLog.md`，只在这里保留必要的自动化状态，并在对话中报告。
- 检查只读源码和构建结果；除记忆/日志文件外，不替项目直接改代码。
