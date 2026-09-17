#include "Pub_Sub.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct Scenario {
    std::size_t channels;
    std::size_t publishers_per_channel;
    std::size_t subscribers_per_channel;
};

std::size_t parse_messages(const char *text) {
    try {
        const std::string value(text);
        std::size_t end = 0;
        const auto number = std::stoull(value, &end);
        if (end == value.size() && number > 0 &&
            number <= std::numeric_limits<std::size_t>::max()) {
            return static_cast<std::size_t>(number);
        }
    } catch (const std::exception &) {
    }
    std::cerr << "--messages 需要一个大于 0 的整数\n";
    std::exit(2);
}

void run_case(const Scenario &scenario, std::size_t messages, std::size_t case_number) {
    const auto publisher_count = scenario.channels * scenario.publishers_per_channel;
    const auto subscriber_count = scenario.channels * scenario.subscribers_per_channel;
    std::vector<Publisher *> publishers;
    std::vector<Subscriber *> subscribers;
    publishers.reserve(publisher_count);
    subscribers.reserve(subscriber_count);

    // Complete registration before concurrent sends; there is no unsubscribe.
    for (std::size_t channel = 0; channel < scenario.channels; ++channel) {
        const std::string name = "probe." + std::to_string(case_number) + "." +
                                 std::to_string(channel);
        for (std::size_t i = 0; i < scenario.subscribers_per_channel; ++i) {
            auto *subscriber = new Subscriber();
            subscriber->Register(name);
            subscribers.push_back(subscriber);
        }
        for (std::size_t i = 0; i < scenario.publishers_per_channel; ++i) {
            auto *publisher = new Publisher();
            publisher->Register(name);
            publishers.push_back(publisher);
        }
    }

    std::atomic<std::size_t> ready{0};
    std::atomic<bool> start{false};
    std::vector<std::thread> workers;
    workers.reserve(publisher_count);
    for (std::size_t publisher = 0; publisher < publisher_count; ++publisher) {
        workers.emplace_back([&, publisher] {
            ready.fetch_add(1);
            while (!start.load()) std::this_thread::yield();
            for (std::size_t sequence = 0; sequence < messages; ++sequence) {
                publishers[publisher]->Notify(static_cast<std::uint64_t>(sequence));
            }
        });
    }
    while (ready.load() != publisher_count) std::this_thread::yield();
    const auto begin = Clock::now();
    start.store(true);
    for (auto &worker : workers) worker.join();
    const auto sent = Clock::now();

    // After sending, wait until each accepted message has reached the data slot.
    const auto deadline = Clock::now() + std::chrono::seconds(2);
    std::uint64_t accepted = 0;
    std::uint64_t processed = 0;
    do {
        accepted = processed = 0;
        for (const auto *subscriber : subscribers) {
            accepted += subscriber->Accepted_Count();
            processed += subscriber->Processed_Count();
        }
        if (processed == accepted) break;
        std::this_thread::yield();
    } while (Clock::now() < deadline);
    const auto end = Clock::now();

    const auto sent_messages = publisher_count * messages;
    const auto expected_deliveries = sent_messages * scenario.subscribers_per_channel;
    const double send_seconds = std::chrono::duration<double>(sent - begin).count();
    const double total_seconds = std::chrono::duration<double>(end - begin).count();
    std::cout << scenario.channels << '/' << publisher_count << '/' << subscriber_count
              << '\t' << sent_messages << '\t' << expected_deliveries
              << '\t' << processed << '\t' << std::fixed << std::setprecision(2)
              << (expected_deliveries ? 100.0 * processed / expected_deliveries : 0.0)
              << '\t' << std::setprecision(0) << sent_messages / send_seconds
              << '\t' << processed / total_seconds
              << '\t' << std::setprecision(3) << send_seconds << '\n';

    for (auto *publisher : publishers) delete publisher;
    // The registry and detached subscriber workers retain these pointers.
}

} // namespace

int main(int argc, char **argv) {
    std::size_t messages = 100000;
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "--help") {
            std::cout << "用法: pub_sub_probe [--messages N]\n"
                         "  --messages N  每个发布者连续发送的消息数（默认 100000）\n";
            return 0;
        }
        if (arg == "--messages" && i + 1 < argc) {
            messages = parse_messages(argv[++i]);
        } else {
            std::cerr << "未知参数或缺少参数值: " << arg << '\n';
            return 2;
        }
    }

    std::cout << "高频并发发布/订阅测试：每个发布者连续发送 " << messages
              << " 条，不限速\n"
              << "频道/发布/订阅\t发布数\t应送达数\t实际处理数\t送达率(%)\t发布条/秒\t处理份/秒\t发送秒数\n";
    const Scenario cases[] = {
        {1, 1, 1}, {1, 1, 4}, {1, 1, 16},
        {1, 4, 1}, {1, 4, 4}, {1, 4, 16},
    };
    for (std::size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        run_case(cases[i], messages, i);
    }
    std::cout << "每条消息向同频道每位订阅者各送一份；送达率=实际处理数/应送达数。"
                 "处理速率含发送完成后的排空时间。\n";
}
