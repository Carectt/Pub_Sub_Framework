#ifndef PUB_SUB_HPP
#define PUB_SUB_HPP

#include "Msg.hpp"
#include "all.hpp"

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
    std::any data;      // 用于读取的数据
                        // Msg hold_msg;

  public:
    void Register(const std::string &channel);
    void Msg_Push(const std::shared_ptr<Msg> &msg);
    template <typename T> T Get_Value();
    // void consume()
    Subscriber(/* args */);
    ~Subscriber();
};

void Map_Add(const std::string &channel, Subscriber *Sub);
void Map_Add(const std::string &channel);
inline std::unordered_map<std::string, std::vector<Subscriber *>> Reg_Map;

template <typename P> inline void Publisher::Notify(const P &data) {
    auto msg = std::make_shared<Msg>(this->Num_Cnt, data);
    if (this->Channel != "None") {
        auto it = Reg_Map.find(this->Channel);
        for (int i = 0; i < it->second.size(); i++) {
            it->second[i]->Msg_Push(msg);
        }
    }
}
#endif // PUB_SUB_HPP

template <typename T> inline T Subscriber::Get_Value() {
    std::unique_lock<std::mutex> lock(this->mtx);
    return T(std::any_cast<T>(this->data));
}
