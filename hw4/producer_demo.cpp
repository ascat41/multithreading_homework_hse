#include "ipc_queue.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

int main() {
    try {
        const std::string shared_memory_name = "/hw4_demo_queue";
        const std::size_t slot_count = 8;

        hw4::ProducerNode producer(shared_memory_name, slot_count);

        std::cout << "Producer started.\n";
        std::cout << "Queue created: " << shared_memory_name << '\n';
        std::this_thread::sleep_for(std::chrono::seconds(2));

        struct TestMessage {
            std::uint32_t type;
            std::string text;
        };

        std::vector<TestMessage> messages = {
            {1, "hello from producer"},
            {2, "this message should be dropped"},
            {1, "second useful message"},
            {2, "another ignored message"},
            {1, "final message"}
        };

        for (const auto& message : messages) {
            const bool ok = producer.Send(
                message.type,
                message.text.data(),
                message.text.size()
            );

            std::cout << "Send: type=" << message.type
                      << ", text=\"" << message.text
                      << "\", result=" << (ok ? "ok" : "fail") << '\n';

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::cout << "Producer finished sending messages.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Producer error: " << ex.what() << '\n';
        return 1;
    }
}