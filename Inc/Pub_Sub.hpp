#ifndef PUB_SUB_HPP
#define PUB_SUB_HPP

#include "all.hpp"

class Publisher
{
private:
    std::string Channel;

public:
    void Register(std::string channel);
    Publisher(/* args */);
    ~Publisher();
};

class Subscriber
{
private:
    std::string Channel;

public:
    void Register(const std::string &channel);
    Subscriber(/* args */);
    ~Subscriber();
};

void Map_Add(const std::string &channel, Subscriber *Sub);
void Map_Add(const std::string &channel);
inline std::unordered_map<std::string, std::vector<Subscriber *>> Reg_Map;

#endif // PUB_SUB_HPP
