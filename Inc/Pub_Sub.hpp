#ifndef PUB_SUB_HPP
#define PUB_SUB_HPP

#include <iostream>
class Publisher
{
private:
    // /* data */
    // std::string name = "";
public:
    void Register(void);
    Publisher(/* args */);
    ~Publisher();
};
class Subscriber
{
private:
    /* data */
public:
    void Register(void);
    Subscriber(/* args */);
    ~Subscriber();
};

#endif // PUB_SUB_HPP
