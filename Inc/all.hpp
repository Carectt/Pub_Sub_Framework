#ifndef ALL_HPP
#define ALL_HPP
// 用于存放公用include
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
// 全局互斥锁（inline 避免 多个编译单元重复定义）
inline std::mutex Map_mtx;
#endif // ALL_HPP
