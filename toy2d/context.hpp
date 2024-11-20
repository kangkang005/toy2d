#pragma once

#include "tool.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>

namespace toy2d {

class Context final {
  public:
    static void Init();
    static void Quit();

    static Context& GetInstance() {
        assert(instance_);
        return *instance_;
    }

    ~Context();

    struct QueueFamliyIndices final {
        std::optional<uint32_t> graphicsQueue;
    };

    vk::Instance instance;
    vk::PhysicalDevice phyDevice;
    vk::Device device;
    vk::Queue graphcisQueue;
    QueueFamliyIndices queueFamilyIndices;

  private:
    static std::unique_ptr<Context> instance_;

    Context();

    void createInstance();
    void printPhyiscalDevices();
    void pickupPhyiscalDevice();
    void createDevice();
    void getQueues();

    void queryQueueFamilyIndices();
};

} // namespace toy2d
