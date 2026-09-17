#include "main.hpp"
Publisher IMU;
Subscriber PID_Calculer;
int main() {
    static int testnum = 0;
    static int recnum = 0;
    IMU.Register("Pitch");
    PID_Calculer.Register("Pitch");
    while (true) {
        IMU.Notify(testnum);
        testnum++;
        recnum = PID_Calculer.Get_Value<int>();
        std::cout << recnum << "\n";
    }

    std::cout << "Hello World!\n";
    std::cin.get();
}