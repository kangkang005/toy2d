#include "toy2d/buffer.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

// buffer is a continuous memory
Buffer::Buffer(size_t size, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags property)
    : size(size) {
    createBuffer(size, usage);
    printMemoryInfo();
    auto info = queryMemoryInfo(property);
    allocateMemory(info);
    bindingMem2Buf();
}

Buffer::~Buffer() {
    Context::Instance().device.freeMemory(memory);
    Context::Instance().device.destroyBuffer(buffer);
}

void Buffer::createBuffer(size_t size, vk::BufferUsageFlags usage) {
    vk::BufferCreateInfo createInfo;
    createInfo.setSize(size).setUsage(usage).setSharingMode(
        vk::SharingMode::eExclusive);

    buffer = Context::Instance().device.createBuffer(createInfo);
}

// allocate memory at heap corresponding memory type
void Buffer::allocateMemory(MemoryInfo info) {
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setMemoryTypeIndex(info.index).setAllocationSize(info.size);
    memory = Context::Instance().device.allocateMemory(allocInfo);
}

void Buffer::printMemoryInfo() {
    auto requirements =
        Context::Instance().device.getBufferMemoryRequirements(buffer);
    std::cout << "memoryTypeBits : " << requirements.memoryTypeBits
              << std::endl;

    auto properties = Context::Instance().phyDevice.getMemoryProperties();
    for (int i = 0; i < properties.memoryTypeCount; i++) {
        std::cout << "memory " << i << " support:";
        auto memoryType = properties.memoryTypes[i];
        if ((memoryType.propertyFlags &
             vk::MemoryPropertyFlagBits::eHostVisible) ==
            vk::MemoryPropertyFlagBits::eHostVisible) {
            std::cout << " HostVisible";
        }
        if ((memoryType.propertyFlags &
             vk::MemoryPropertyFlagBits::eDeviceLocal) ==
            vk::MemoryPropertyFlagBits::eDeviceLocal) {
            std::cout << " DeviceLocal";
        }
        if ((memoryType.propertyFlags &
             vk::MemoryPropertyFlagBits::eHostCoherent) ==
            vk::MemoryPropertyFlagBits::eHostCoherent) {
            std::cout << " HostCoherent";
        }
        std::cout << std::endl;

        uint32_t heapIndex = memoryType.heapIndex;
        auto memoryHeap = properties.memoryHeaps[heapIndex];
        std::cout << "heap " << heapIndex << ": "
                  << (memoryHeap.size) / (1024 * 1024 * 1024) << " GB";
        if ((memoryHeap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) ==
            vk::MemoryHeapFlagBits::eDeviceLocal) {
            std::cout << " DeviceLocal";
        }
        std::cout << std::endl;
    }
}

Buffer::MemoryInfo Buffer::queryMemoryInfo(vk::MemoryPropertyFlags property) {
    MemoryInfo info;
    auto requirements =
        Context::Instance().device.getBufferMemoryRequirements(buffer);
    info.size = requirements.size;

    // Fetch all the memory properties of the physical device.
    auto properties = Context::Instance().phyDevice.getMemoryProperties();
    // Loop through each of the memory type fields in the properties.
    for (int i = 0; i < properties.memoryTypeCount; i++) {
        // If the current memory type is available and has all the property
        // flags required, we have found a position in the physical device
        // memory indices that is compatible.
        if ((1 << i) & requirements.memoryTypeBits &&
            properties.memoryTypes[i].propertyFlags & property) {
            info.index = i;
            break;
        }
    }

    return info;
}

void Buffer::bindingMem2Buf() {
    Context::Instance().device.bindBufferMemory(buffer, memory, 0);
}

} // namespace toy2d
