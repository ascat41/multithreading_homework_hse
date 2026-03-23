#include "ipc_queue.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace hw4 {
namespace {

std::size_t CalculateMappedSize(std::size_t slot_count) {
    return sizeof(QueueHeader) + slot_count * sizeof(MessageSlot);
}

void BindPointers(void* mapping, QueueHeader*& header, MessageSlot*& slots) {
    header = static_cast<QueueHeader*>(mapping);
    slots = reinterpret_cast<MessageSlot*>(
        static_cast<std::uint8_t*>(mapping) + sizeof(QueueHeader)
    );
}

std::runtime_error MakeError(const std::string& message) {
    return std::runtime_error(message + ": " + std::strerror(errno));
}

} // namespace

ProducerNode::ProducerNode(const std::string& shm_name, std::size_t slot_count)
    : shared_memory_name_(shm_name) {
    OpenOrCreate(slot_count);
}

ProducerNode::~ProducerNode() {
    Close();
}

ConsumerNode::ConsumerNode(const std::string& shm_name)
    : shared_memory_name_(shm_name) {
    OpenExisting();
}

ConsumerNode::~ConsumerNode() {
    Close();
}

void ProducerNode::OpenOrCreate(std::size_t slot_count) {
    if (slot_count == 0) {
        throw std::runtime_error("slot_count must be greater than 0");
    }

    mapped_size_ = CalculateMappedSize(slot_count);

    shm_unlink(shared_memory_name_.c_str());
    shm_fd_ = shm_open(shared_memory_name_.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd_ == -1) {
        throw MakeError("shm_open failed");
    }

    if (ftruncate(shm_fd_, static_cast<off_t>(mapped_size_)) == -1) {
        Close();
        throw MakeError("ftruncate failed");
    }

    mapped_region_ = mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (mapped_region_ == MAP_FAILED) {
        mapped_region_ = nullptr;
        Close();
        throw MakeError("mmap failed");
    }

    BindPointers(mapped_region_, header_, slots_);

    std::memset(mapped_region_, 0, mapped_size_);

    header_->magic = kQueueMagic;
    header_->version = kProtocolVersion;
    header_->slot_count = static_cast<std::uint32_t>(slot_count);
    header_->max_payload_size = static_cast<std::uint32_t>(kMaxPayloadSize);
    header_->write_index.store(0, std::memory_order_relaxed);
    header_->read_index.store(0, std::memory_order_relaxed);

    for (std::size_t i = 0; i < slot_count; ++i) {
        slots_[i].state.store(
            static_cast<std::uint32_t>(SlotState::Empty),
            std::memory_order_relaxed
        );
        slots_[i].header.type = 0;
        slots_[i].header.length = 0;
    }
}

void ConsumerNode::OpenExisting() {
    shm_fd_ = shm_open(shared_memory_name_.c_str(), O_RDWR, 0666);
    if (shm_fd_ == -1) {
        throw MakeError("shm_open failed");
    }

    const std::size_t header_size = sizeof(QueueHeader);
    mapped_region_ = mmap(nullptr, header_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (mapped_region_ == MAP_FAILED) {
        mapped_region_ = nullptr;
        Close();
        throw MakeError("mmap header failed");
    }

    header_ = static_cast<QueueHeader*>(mapped_region_);

    if (header_->magic != kQueueMagic) {
        Close();
        throw std::runtime_error("invalid queue magic");
    }

    if (header_->version != kProtocolVersion) {
        Close();
        throw std::runtime_error("protocol version mismatch");
    }

    mapped_size_ = CalculateMappedSize(header_->slot_count);

    if (munmap(mapped_region_, header_size) == -1) {
        Close();
        throw MakeError("munmap header failed");
    }

    mapped_region_ = mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (mapped_region_ == MAP_FAILED) {
        mapped_region_ = nullptr;
        Close();
        throw MakeError("mmap full queue failed");
    }

    BindPointers(mapped_region_, header_, slots_);
}

void ProducerNode::Close() {
    if (mapped_region_ != nullptr) {
        munmap(mapped_region_, mapped_size_);
        mapped_region_ = nullptr;
    }

    if (shm_fd_ != -1) {
        close(shm_fd_);
        shm_fd_ = -1;
    }

    header_ = nullptr;
    slots_ = nullptr;
    mapped_size_ = 0;
}

void ConsumerNode::Close() {
    if (mapped_region_ != nullptr) {
        munmap(mapped_region_, mapped_size_);
        mapped_region_ = nullptr;
    }

    if (shm_fd_ != -1) {
        close(shm_fd_);
        shm_fd_ = -1;
    }

    header_ = nullptr;
    slots_ = nullptr;
    mapped_size_ = 0;
}

bool ProducerNode::Send(std::uint32_t type, const void* data, std::size_t size) {
    if (data == nullptr && size != 0) {
        return false;
    }

    if (size > kMaxPayloadSize) {
        return false;
    }

    const std::uint64_t logical_index =
        header_->write_index.fetch_add(1, std::memory_order_relaxed);

    const std::size_t slot_index =
        static_cast<std::size_t>(logical_index % header_->slot_count);

    MessageSlot& slot = slots_[slot_index];

    while (slot.state.load(std::memory_order_acquire) !=
           static_cast<std::uint32_t>(SlotState::Empty)) {
        // active wait
    }

    slot.state.store(
        static_cast<std::uint32_t>(SlotState::Writing),
        std::memory_order_relaxed
    );

    slot.header.type = type;
    slot.header.length = static_cast<std::uint32_t>(size);

    if (size != 0) {
        std::memcpy(slot.payload, data, size);
    }

    slot.state.store(
        static_cast<std::uint32_t>(SlotState::Ready),
        std::memory_order_release
    );

    return true;
}

bool ConsumerNode::Receive(std::uint32_t desired_type, ReceivedMessage& received_msg) {
    const std::uint64_t logical_index =
        header_->read_index.load(std::memory_order_relaxed);

    const std::size_t slot_index =
        static_cast<std::size_t>(logical_index % header_->slot_count);

    MessageSlot& slot = slots_[slot_index];

    if (slot.state.load(std::memory_order_acquire) !=
        static_cast<std::uint32_t>(SlotState::Ready)) {
        return false;
    }

    const std::uint32_t message_type = slot.header.type;
    const std::uint32_t message_length = slot.header.length;

    if (message_length > kMaxPayloadSize) {
        slot.header.type = 0;
        slot.header.length = 0;
        slot.state.store(
            static_cast<std::uint32_t>(SlotState::Empty),
            std::memory_order_release
        );
        header_->read_index.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    if (message_type == desired_type) {
        received_msg.type = message_type;
        received_msg.payload.resize(message_length);

        if (message_length != 0) {
            std::memcpy(received_msg.payload.data(), slot.payload, message_length);
        }
    }

    slot.header.type = 0;
    slot.header.length = 0;
    slot.state.store(
        static_cast<std::uint32_t>(SlotState::Empty),
        std::memory_order_release
    );

    header_->read_index.fetch_add(1, std::memory_order_relaxed);

    return message_type == desired_type;
}

} // namespace hw4