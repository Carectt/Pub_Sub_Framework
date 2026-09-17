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
    std::unique_lock<std::mutex> reg_lock(Map_mtx);
    Map_Add(channel);
    reg_lock.unlock();
    this->Channel = channel;
    // TODO: 注册发布
}

Publisher::~Publisher() {
    // TODO:  注销发布
}
Subscriber::Subscriber() {}

void Subscriber::Register(const std::string &channel) {
    std::unique_lock<std::mutex> reg_lock(Map_mtx);
    Map_Add(channel, this);
    reg_lock.unlock();
    this->Channel = channel;
    // 使用thread和核心交互，确保pub的遍历不会卡住
    std::thread Sub_thread([this]() {
        while (true) {
            std::unique_lock<std::mutex> lock(this->mtx); // 上锁
            this->cv.wait(
                lock, [this] { return this->temp_data.has_value(); }); // cv等待
            this->data = this->temp_data;
            this->temp_data.reset();
            this->processed_count.fetch_add(1, std::memory_order_relaxed); // 消息转入可读槽后计数加一
        }
        // lock.unlock();
    });
    Sub_thread.detach();
    // TODO: 注册订阅
}

void Subscriber::Msg_Push(const std::shared_ptr<Msg> &msg) {
    std::unique_lock<std::mutex> lock(mtx);   // 上锁
    if (this->temp_data.has_value() == false) // if上一个msg已经被处理好了
    {
        this->temp_data = msg;
        this->accepted_count.fetch_add(1, std::memory_order_relaxed); // 消息进入临时槽后计数加一
        this->cv.notify_one();
    }
}

Subscriber::~Subscriber() {
    // TODO:  注销订阅
}
