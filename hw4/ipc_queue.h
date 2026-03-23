#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace hw4 {

inline constexpr std::uint32_t kQueueMagic = 0x51554555;
inline constexpr std::uint32_t kProtocolVersion = 1;
inline constexpr std::size_t kMaxPayloadSize = 256;
inline constexpr std::size_t kDefaultSlotCount = 64;

enum class SlotState : std::uint32_t {
    Empty = 0,
    Writing = 1,
    Ready = 2
};

struct MessageHeader {
    std::uint32_t type = 0;
    std::uint32_t length = 0;
};

struct QueueHeader {
    std::uint32_t magic = kQueueMagic;
    std::uint32_t version = kProtocolVersion;
    std::uint32_t slot_count = 0;
    std::uint32_t max_payload_size = 0;

    std::atomic<std::uint64_t> write_index{0};
    std::atomic<std::uint64_t> read_index{0};
};

struct MessageSlot {
    std::atomic<std::uint32_t> state{
        static_cast<std::uint32_t>(SlotState::Empty)
    };

    MessageHeader header{};
    std::uint8_t payload[kMaxPayloadSize]{};
};

struct ReceivedMessage {
    std::uint32_t type = 0;
    std::vector<std::uint8_t> payload;
};

class ProducerNode {
public:
    ProducerNode(const std::string& shm_name, std::size_t slot_count);
    ~ProducerNode();

    bool Send(std::uint32_t type, const void* data, std::size_t size);

    ProducerNode(const ProducerNode&) = delete;
    ProducerNode& operator=(const ProducerNode&) = delete;

private:
    std::string shared_memory_name_;
    std::size_t mapped_size_ = 0;
    int shm_fd_ = -1;
    void* mapped_region_ = nullptr;

    QueueHeader* header_ = nullptr;
    MessageSlot* slots_ = nullptr;

    void OpenOrCreate(std::size_t slot_count);
    void Close();
};

class ConsumerNode {
public:
    explicit ConsumerNode(const std::string& shm_name);
    ~ConsumerNode();

    bool Receive(std::uint32_t desired_type, ReceivedMessage& received_msg);

    ConsumerNode(const ConsumerNode&) = delete;
    ConsumerNode& operator=(const ConsumerNode&) = delete;

private:
    std::string shared_memory_name_;
    std::size_t mapped_size_ = 0;
    int shm_fd_ = -1;
    void* mapped_region_ = nullptr;

    QueueHeader* header_ = nullptr;
    MessageSlot* slots_ = nullptr;

    void OpenExisting();
    void Close();
};

} // namespace hw4