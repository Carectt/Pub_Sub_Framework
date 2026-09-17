#include "Pub_Sub.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace {

template <typename T>
bool eventually(Subscriber &subscriber, const T &expected) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::chrono::steady_clock::now() < deadline) {
        try {
            if (subscriber.Get_Value<T>() == expected) return true;
        } catch (const std::bad_any_cast &) {
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}

bool check(const char *name, bool success) {
    std::cout << (success ? "PASS " : "FAIL ") << name << '\n';
    return success;
}

} // namespace

int main() {
    bool passed = true;
    Publisher unregistered;
    unregistered.Notify(123);
    passed &= check("unregistered publish does not crash", true);

    Publisher empty;
    empty.Register("smoke.empty");
    empty.Notify(123);
    passed &= check("channel without subscribers does not crash", true);

    // Subscribers remain alive because the framework retains raw pointers and
    // their worker threads have no shutdown mechanism.
    auto first = std::make_unique<Subscriber>();
    auto second = std::make_unique<Subscriber>();
    auto isolated = std::make_unique<Subscriber>();
    first->Register("smoke.main");
    second->Register("smoke.main");
    isolated->Register("smoke.other");
    Publisher main_publisher;
    Publisher other_publisher;
    main_publisher.Register("smoke.main");
    other_publisher.Register("smoke.other");
    main_publisher.Notify(101);
    other_publisher.Notify(202);
    passed &= check("integer delivery to both subscribers",
                    eventually(*first, 101) && eventually(*second, 101));
    passed &= check("channel isolation", eventually(*isolated, 202));

    Publisher text_publisher;
    auto text_subscriber = std::make_unique<Subscriber>();
    text_publisher.Register("smoke.text");
    text_subscriber->Register("smoke.text");
    text_publisher.Notify(std::string("payload"));
    passed &= check("string payload", eventually(*text_subscriber, std::string("payload")));
    try {
        (void)text_subscriber->Get_Value<int>();
        passed &= check("wrong payload type reports error", false);
    } catch (const std::bad_any_cast &) {
        passed &= check("wrong payload type reports error", true);
    }

    first.release();
    second.release();
    isolated.release();
    text_subscriber.release();
    return passed ? 0 : 1;
}
