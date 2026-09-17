#include "Pub_Sub.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct StressOptions {
    std::size_t message_count = 100;
    std::chrono::milliseconds interval{1};
};

std::shared_ptr<Msg> read_message(Subscriber &subscriber) {
    try {
        return subscriber.Get_Value<std::shared_ptr<Msg>>();
    } catch (const std::bad_any_cast &) {
        return nullptr;
    }
}

bool get_payload_sequence(const std::shared_ptr<Msg> &message, std::uint64_t &sequence) {
    if (!message) {
        return false;
    }
    try {
        sequence = std::any_cast<std::uint64_t>(message->payload);
        return true;
    } catch (const std::bad_any_cast &) {
        return false;
    }
}

double percentile(std::vector<double> samples, double quantile) {
    if (samples.empty()) {
        return 0.0;
    }
    std::sort(samples.begin(), samples.end());
    const auto index = static_cast<std::size_t>(
        quantile * static_cast<double>(samples.size() - 1));
    return samples[index];
}

std::uint64_t parse_positive_integer(const char *text, const char *option) {
    try {
        std::size_t parsed = 0;
        const std::string value(text);
        const auto number = std::stoull(value, &parsed);
        if (parsed != value.size() || number == 0) {
            throw std::invalid_argument("expected positive integer");
        }
        return number;
    } catch (const std::exception &) {
        std::cerr << option << " 需要一个大于 0 的整数\n";
        std::exit(2);
    }
}

void run_stress_case(std::size_t fanout, const StressOptions &options) {
    static std::size_t channel_number = 0;
    const std::string channel = "stress.channel." + std::to_string(channel_number++);

    Publisher publisher;
    publisher.Register(channel);

    // The current framework has no unsubscribe operation. Keep test subscribers
    // alive until process exit so the registry never holds dangling pointers.
    std::vector<Subscriber *> subscribers;
    subscribers.reserve(fanout);
    for (std::size_t i = 0; i < fanout; ++i) {
        auto *subscriber = new Subscriber();
        subscriber->Register(channel);
        subscribers.push_back(subscriber);
    }
    if (fanout > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    constexpr std::size_t max_latency_samples = 20000;
    const std::size_t sample_stride =
        std::max<std::size_t>(1, options.message_count / max_latency_samples);
    std::vector<double> latency_us;
    latency_us.reserve(std::min(options.message_count, max_latency_samples));

    const auto total_start = Clock::now();
    for (std::size_t i = 0; i < options.message_count; ++i) {
        const auto send_start = Clock::now();
        publisher.Notify(static_cast<std::uint64_t>(i));
        const auto send_end = Clock::now();

        if (i % sample_stride == 0) {
            latency_us.push_back(
                std::chrono::duration<double, std::micro>(send_end - send_start).count());
        }

        // Measure the interval from one send start to the next. If Notify itself
        // takes longer than the interval, the next send naturally starts later.
        std::this_thread::sleep_until(send_start + options.interval);
    }
    const auto total_end = Clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    std::size_t subscribers_with_any_message = 0;
    std::size_t subscribers_with_last_message = 0;
    for (Subscriber *subscriber : subscribers) {
        std::uint64_t received_sequence = 0;
        if (get_payload_sequence(read_message(*subscriber), received_sequence)) {
            ++subscribers_with_any_message;
            if (received_sequence == options.message_count - 1) {
                ++subscribers_with_last_message;
            }
        }
    }

    const double seconds = std::chrono::duration<double>(total_end - total_start).count();
    const double send_rate = seconds == 0.0
                                 ? 0.0
                                 : static_cast<double>(options.message_count) / seconds;

    std::cout << std::left << std::setw(10) << fanout << std::right << std::setw(13)
              << options.message_count << std::setw(14) << std::fixed << std::setprecision(1)
              << seconds << std::setw(14) << std::setprecision(1) << send_rate << std::setw(12)
              << std::setprecision(3) << percentile(latency_us, 0.50) << std::setw(12)
              << percentile(latency_us, 0.95) << std::setw(12)
              << percentile(latency_us, 0.99) << std::setw(13)
              << (std::to_string(subscribers_with_any_message) + "/" + std::to_string(fanout))
              << std::setw(13)
              << (std::to_string(subscribers_with_last_message) + "/" + std::to_string(fanout))
              << "\n";
}

void run_stress(const StressOptions &options) {
    const double max_send_rate = 1000.0 / static_cast<double>(options.interval.count());
    std::cout << "发布压力测试（最多 " << std::fixed << std::setprecision(1)
              << max_send_rate << " 条消息/秒；每种扇出 "
              << options.message_count << " 条）\n";
    std::cout << std::left << std::setw(10) << "订阅者数" << std::right << std::setw(13)
              << "发送数" << std::setw(14) << "耗时(s)" << std::setw(14) << "速率(msg/s)"
              << std::setw(12) << "P50(us)" << std::setw(12) << "P95(us)" << std::setw(12)
              << "P99(us)" << std::setw(13) << "有消息可读" << std::setw(13) << "末条可读"
              << "\n";

    for (const std::size_t fanout : {std::size_t{0}, std::size_t{1}, std::size_t{8},
                                     std::size_t{32}}) {
        run_stress_case(fanout, options);
    }

    std::cout << "发送间隔至少为 " << options.interval.count()
              << " ms；“有消息可读”统计可读到任意载荷的订阅者，“末条可读”校验最终序号载荷。"
                 "速率是 Notify 调用速率，不代表订阅端完整处理了全部消息。\n";
}

void print_help() {
    std::cout << "用法: pub_sub_probe [--messages N] [--interval-ms N]\n"
                 "  --messages N      每种订阅者扇出发送的消息数（默认 100）\n"
                 "  --interval-ms N   消息最小发送间隔，单位 ms（默认 1，最小值 1）\n"
                 "  --help            显示帮助\n";
}

} // namespace

int main(int argc, char **argv) {
    StressOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--help") {
            print_help();
            return 0;
        }
        if (argument == "--messages" && i + 1 < argc) {
            const auto value = parse_positive_integer(argv[++i], "--messages");
            if (value > std::numeric_limits<std::size_t>::max()) {
                std::cerr << "--messages 超出当前平台可支持的范围\n";
                return 2;
            }
            options.message_count = static_cast<std::size_t>(value);
            continue;
        }
        if (argument == "--interval-ms" && i + 1 < argc) {
            const auto value = parse_positive_integer(argv[++i], "--interval-ms");
            if (value < 1) {
                std::cerr << "--interval-ms 的最小值是 1 ms\n";
                return 2;
            }
            if (value > static_cast<std::uint64_t>(
                            std::numeric_limits<std::chrono::milliseconds::rep>::max())) {
                std::cerr << "--interval-ms 超出可支持的范围\n";
                return 2;
            }
            options.interval = std::chrono::milliseconds(value);
            continue;
        }
        std::cerr << "未知参数或缺少参数值: " << argument << "\n";
        return 2;
    }

    run_stress(options);
    return 0;
}
