# Pub/Sub 独立压力测试

本目录只新增测试代码，通过公开头文件链接现有实现，不修改项目原有的 `Inc/`、`Src/` 或根 `CMakeLists.txt`。

## 构建与运行

```powershell
cmake -S tests -B tests/build -DCMAKE_BUILD_TYPE=Release
cmake --build tests/build --config Release
.\tests\build\pub_sub_probe.exe
```

程序只做发布压力测试，不执行正确性或健壮性检查。它会依次测试 0、1、8、32 个订阅者的场景，统计发送数量、运行时间、`Notify` 调用速率和调用延迟 P50/P95/P99。发送结束后还会检查订阅者当前可读的载荷，分别统计至少收到过一条消息、以及读到了最终序号消息的订阅者数。默认每种扇出发送 100 条消息，发送间隔至少 1 ms，最高约 1000 条/秒。

可选参数：

```text
--messages N         设置每种扇出发送的消息数，默认 100
--interval-ms N      设置最小消息发送间隔（毫秒），默认 1，不能小于 1
--help                显示帮助
```

例如，下面的命令会以每 1 ms 至多发送一条的节奏，在四种订阅者数量下分别运行 10,000 条消息：

```powershell
.\tests\build\pub_sub_probe.exe --messages 10000 --interval-ms 1
```

送达列读取的是压力结束后订阅者当前持有的消息，不是完整历史计数；因此它能验证实际载荷是否到达及末条消息是否可见，但不能统计中间每一条消息。速率和延迟只衡量发布端调用 `Notify` 的表现，不代表订阅端成功处理消息的端到端吞吐量。当前框架没有取消订阅接口，测试中的订阅者会保留到进程退出，以免注册表持有悬垂指针。
