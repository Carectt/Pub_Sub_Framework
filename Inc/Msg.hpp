#ifndef MSG_HPP
#define MSG_HPP
#include "all.hpp"
#include <any>

class Msg {
  public:
    uint64_t counter;
    std::any payload;
    template <typename T>
    Msg(const uint64_t &topic, T &&data)
        : counter(topic), payload(std::forward<T>(data)) {}
};
#endif // MSG_HPP
