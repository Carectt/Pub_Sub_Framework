# Pub_Sub_Framework · 开发记录

> 最近整理：2026-09-14 · 当前提交：`8dfe47e`（`feat: 打通订阅者收包链路`）
> 对比基线：`8dfe47e`
> 工作区：仅有 `DevLog.md` 的未提交记录整理，未发现新的代码改动

这是一套用 C++17 编写的 Broker 风格发布订阅框架。当前示例围绕机器人或嵌入式场景中的姿态数据分发，使用 IMU 数据发布到 `Pitch` 频道，并由 PID 计算器订阅。

## 提交时间线

### 01 · `e84bcb3` · 2026-08-03 · 添加仓库规则文件

加入 `.gitattributes` 和 `.gitignore`，建立换行规则、忽略范围和基础仓库约定。此时还没有业务代码，提交的作用是确定后续项目文件和构建产物的管理边界。

### 02 · `edd01a4` · 2026-08-03 · 添加 Visual Studio 项目文件

创建 `.slnx`、`.vcxproj`、`.vcxproj.filters` 和 `Pub_Sub_Framework.cpp`，使项目可以被 Visual Studio 打开、编译和调试。此时仍是标准控制台项目，发布订阅模型尚未实现。

### 03 · `718b1d6` · 2026-08-03 · 统一入口文件名称

将 `Pub_Sub_Framework.cpp` 重命名为 `main.cpp`，把项目入口调整为通用的 C/C++ 命名，为后续拆分 `Broker` 和 `Pub_Sub` 模块做准备。

### 04 · `df43661` · 2026-08-03 · 引入 Broker 模块位置

新增 `Broker.hpp`，并将原来的实现文件调整为 `Broker.cpp`。此时 Broker 还没有实际转发逻辑，但项目开始明确把消息中转层作为独立模块保留。

### 05 · `bf7054f` · 2026-08-03 · 调整源码目录结构

移除 Visual Studio 工程文件，将头文件和源文件分别归入 `Inc/` 与 `Src/`。项目从 IDE 工程模板转向更适合跨工具链构建的源码布局。

### 06 · `ee4f4ba` · 2026-08-03 · 建立 CMake 与模块骨架

加入 `CMakeLists.txt`，启用 C++17 并定义可执行目标；同时建立 `Broker`、`Msg`、`Pub_Sub` 和 `Topic` 的文件骨架。模块职责开始被明确，但当时各模块仍主要是空壳。

### 07 · `ce015f3` · 2026-08-04 · 建立发布者和订阅者接口

加入 `Publisher`、`Subscriber` 的类骨架，并恢复 `Src/main.cpp` 示例入口。示例以 `IMU` 和 `PID_Calculer` 表达发布与订阅场景，二者当时只有接口占位，尚未形成消息传递行为。

### 08 · `d961a19` · 2026-08-04 · 接入 Debug 调试配置

补充 VS Code 的 Debug 配置，并让 CMake 默认使用 Debug 构建。`launch.json` 和 `tasks.json` 使本地编译、启动和断点调试路径基本可用，但业务逻辑仍处于接口阶段。

### 09 · `934e455` · 2026-09-06 · 完善频道注册表

实现 `Reg_Map` 和两个 `Map_Add` 重载：发布者可以先创建频道，订阅者可以将自身加入对应频道，并避免同一个订阅者重复登记。`Topic` 模块被移除，频道职责暂时集中到 `Pub_Sub`。此时已经建立频道到订阅者的映射，但还没有消息对象和投递链路。

### 10 · `8dfe47e` · 2026-09-13 · 打通订阅者收包链路

引入 `Msg`，用 `counter` 保存消息序号、用 `std::any payload` 保存泛型载荷。`Publisher::Notify` 创建 `std::shared_ptr<Msg>`，按发布者频道查找 `Reg_Map` 并调用每个订阅者的 `Msg_Push`。

订阅者侧新增 `Msg_Push`、`Get_Value<T>`、`temp_data` 和 `data`：`Register` 启动分离线程，通过 `condition_variable` 等待消息；`Msg_Push` 在互斥锁保护下暂存消息并唤醒等待线程；等待线程再将暂存消息转为可读取数据。至此，频道注册、消息创建、频道投递和订阅端接收已经连成一条基本路径。

## 当前结构

- `Inc/Pub_Sub.hpp`：`Publisher`、`Subscriber`、频道注册表以及发布和读取接口。
- `Src/Pub_Sub.cpp`：频道创建、订阅者登记、订阅等待线程和消息暂存实现。
- `Inc/Msg.hpp`：带序号和 `std::any` 泛型载荷的消息对象。
- `Broker.hpp` / `Broker.cpp`：预留的中转层位置，尚未承载实际转发逻辑。
- `Inc/all.hpp`：公共头文件、线程同步类型和全局 `Map_mtx`。
- `Topic`：已删除，当前频道管理暂时由 `Pub_Sub` 负责。

## 当前已知问题

1. `Publisher::Notify` 在频道不是 `None` 时直接使用 `Reg_Map.find` 的结果，没有处理频道不存在的情况；`Num_Cnt` 当前也没有递增。
2. `Subscriber::Register` 创建后立即 `detach` 等待线程，析构函数没有停止线程或注销注册表，存在对象生命周期和悬垂指针风险。
3. `Subscriber::Msg_Push` 只保留一条 `temp_data`；已有消息尚未转移时，后续消息会被忽略，当前还不是消息队列。
4. `Get_Value<T>` 依赖 `std::any_cast` 类型匹配，类型错误没有专门的错误处理。
5. `Map_mtx` 已声明但尚未用于保护 `Reg_Map`，并发注册和投递仍不安全。
6. `Reg_Map` 位于头文件并保存裸 `Subscriber*`，全局状态的封装和订阅者注销机制仍需完善。
7. 尚未建立可重复的构建与端到端测试；当前示例主要完成对象注册和文本输出，消息收发还缺少自动化验证。

## 未提交变化

当前工作区相对比较基线 `8dfe47e` 没有新的 `Inc/`、`Src/` 或构建配置变化。唯一的未提交变化是本次对 `DevLog.md` 的全文整理，不应视为新的功能提交。
