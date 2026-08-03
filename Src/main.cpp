#include "main.hpp"
#define _CRT_SECURE_NO_WARNINGS
Publisher IMU;
Subscriber PID_Calculer;
int main()
{
    IMU.Register();
    PID_Calculer.Register();
    std::cout << "Hello World!\n";
}