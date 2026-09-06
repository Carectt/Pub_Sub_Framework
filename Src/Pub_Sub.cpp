#include "Pub_Sub.hpp"
#define _CRT_SECURE_NO_WARNINGS

void Map_Add(const std::string &channel, Subscriber *Sub) {
  auto it = Reg_Map.find(channel);
  if (it != Reg_Map.end()) // 若存在该channel
  {
    for (const auto &val : it->second) {
      if (Sub == val) {
        return; // 结束
      }
      // 遍历读取是否已存在对应的Sub
    }
    // 添加
    it->second.emplace_back(Sub);
  } else // 不存在该channel
  {
    Reg_Map[channel].push_back(Sub);
    // --Reg_Map.emplace(Channel, Sub)是错的，只有operator[]会直接新建vector
  }
}
void Map_Add(const std::string &channel) // Overload函数，用于pub先创立channel
{
  if (Reg_Map.find(channel) == Reg_Map.end()) {
    Reg_Map[channel] = std::vector<Subscriber *>();
  }
}
/*以上为TOPIC部分














*/

Publisher::Publisher(/* args */) {}

void Publisher::Register(std::string channel) {
  Map_Add(channel);
  this->Channel = channel;
  // TODO: 注册发布
}

Publisher::~Publisher() {
  // TODO:  注销发布
}
Subscriber::Subscriber() {}

void Subscriber::Register(const std::string &channel) {
  Map_Add(channel, this);
  this->Channel = channel;
  // TODO: 注册订阅
}

Subscriber::~Subscriber() {
  // TODO:  注销订阅
}
