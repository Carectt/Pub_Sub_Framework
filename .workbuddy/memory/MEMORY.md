# Pub_Sub_Framework · 长期工作上下文

## 项目定位

- 目标是 C++17 的 Broker 风格发布订阅框架，应用背景偏机器人/嵌入式控制。
- 当前示例为 `IMU` 发布者、`PID_Calculer` 订阅者，以及 `Pitch` 频道。
- 构建系统是 CMake，默认 Debug；工具链是 llvm-mingw Clang + clangd + lldb-mi。

## 代码地图

- `Publisher` / `Subscriber` 位于 `Pub_Sub` 模块。
- `Reg_Map` 暂时是 `channel -> vector<Subscriber*>` 的全局注册表。
- `Broker` 只有位置，没有中转实现；`Topic` 模块已经移除。
- `Msg` 正在从空壳变成带泛型载荷的消息类型，但接口尚未稳定。
- `all.hpp` 集中公共 include，并声明 `Map_mtx`。

## 当前状态

截至 2026-09-13，`main` 的 `HEAD` 为 `8590d0f`，相对 `origin/main` 的
`934e455` 多了一次进行中的功能提交。源码当前的主要缺口是消息类型编译问题、
注册表并发保护、析构注销、Broker 投递链路和测试验证。

## 后续工作顺序

1. 先确定消息头部、载荷和默认构造语义，使 `Msg` 能独立编译。
2. 再决定注册表的锁和所有权模型，处理订阅者生命周期。
3. 最后实现发布、投递、回调和最小可验证示例。

## 文档与巡检习惯

- `DevLog.md` 记录每次提交，但保持短段落、窄排版，方便在文本编辑器中阅读。
- 自动巡检若没有新提交，不改 `DevLog.md`，只更新自动化自身的记录并在对话中报告。
- 远端约定：`origin` 指向 GitHub，另有 Gitee push 地址。
