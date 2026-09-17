#ifndef PUB_SUB_HPP
#define PUB_SUB_HPP

#include "Msg.hpp"
#include "all.hpp"
#include <atomic>

class Publisher {
  private:
    std::string Channel;
    std::condition_variable cv;
    std::mutex mtx;
    uint64_t Num_Cnt = 0; // 内部发包序号

  public:
    template <typename P> void Notify(const P &data);
    void Register(std::string channel);
    Publisher(/* args */);
    ~Publisher();
};

class Subscriber {
  private:
    std::string Channel = "None";
    std::condition_variable cv;
    std::mutex mtx;
    std::any temp_data; // 用于临时储存
    std::any data = std::make_shared<Msg>(0, 0);
    std::atomic<uint64_t> accepted_count{0};  // 已进入临时消息槽的消息数，初始为 0
    std::atomic<uint64_t> processed_count{0}; // 已转入可读消息槽的消息数，初始为 0
    ; // 用于读取的msg数据
      // Msg hold_msg;

  public:
    void Register(const std::string &channel);
    void Msg_Push(const std::shared_ptr<Msg> &msg);
    uint64_t Accepted_Count() const { return accepted_count.load(); }   // 读取已接收数量的当前快照
    uint64_t Processed_Count() const { return processed_count.load(); } // 读取已处理数量的当前快照
    template <typename T> T Get_Value();
    // void consume()
    Subscriber(/* args */);
    ~Subscriber();
};

void Map_Add(const std::string &channel, Subscriber *Sub);
void Map_Add(const std::string &channel);
inline std::unordered_map<std::string, std::vector<Subscriber *>> Reg_Map;

template <typename P> inline void Publisher::Notify(const P &data) {
    if (this->Channel.empty() == false) {
        auto msg = std::make_shared<Msg>(this->Num_Cnt, data);
        if (this->Channel != "None") {
            this->Num_Cnt++;
            auto it = Reg_Map.find(this->Channel);
            for (int i = 0; i < it->second.size(); i++) {
                it->second[i]->Msg_Push(msg);
            }
        }
    } else {
        return;
    }
}
#endif // PUB_SUB_HPP

template <typename T> inline T Subscriber::Get_Value() {
    std::unique_lock<std::mutex> lock(this->mtx);
    const auto &msg = std::any_cast<const std::shared_ptr<Msg> &>(this->data);
    return std::any_cast<T>(msg->payload);
    //
}
