#include "main.hpp"
#define _CRT_SECURE_NO_WARNINGS
Publisher IMU;
Subscriber PID_Calculer;
int main()
{
    IMU.Register("Pitch");
    PID_Calculer.Register("Pitch");
    std::cout << "Hello World!\n";
    std::cin.get();
}