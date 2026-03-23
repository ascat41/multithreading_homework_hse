#include "ipc_queue.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

int main() {
    try {
        const std::string shared_memory_name = "/hw4_demo_queue";
        const std::uint32_t message_type_filter = 1;
        const int expected_message_count = 3;

        hw4::ConsumerNode consumer(shared_memory_name);

        std::cout << "Consumer started.\n";
        std::cout << "Reading messages of type " << message_type_filter << " from "
                  << shared_memory_name << '\n';

        int messages_received = 0;

        while (messages_received < expected_message_count) {
            hw4::ReceivedMessage message;

            if (consumer.Receive(message_type_filter, message)) {
                std::string text(message.payload.begin(), message.payload.end());

                std::cout << "Received message: ";
                std::cout << "type=" << message.type
                          << ", length=" << message.payload.size()
                          << ", text=\"" << text << "\"\n";

                ++messages_received;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        std::cout << "Consumer finished after receiving "
                  << messages_received << " matching messages.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Consumer error: " << ex.what() << '\n';
        return 1;
    }
}