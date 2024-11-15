#pragma once

#include "swapchain.hpp"
#include "tool.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <optional>

namespace toy2d {

class Context final {
public:
  static void Init(const std::vector<const char *> &extensions,
                   CreateSurfaceFunc func);
  static void Quit();

  // Singleton Pattern
  static Context &GetInstance() {
    assert(instance_);
    return *instance_;
  }

  ~Context();

  struct QueueFamliyIndices final {
    std::optional<uint32_t> graphicsQueue;
    std::optional<uint32_t> presentQueue;

    operator bool() const {
      return graphicsQueue.has_value() && presentQueue.has_value();
    }
  };

  vk::Instance instance;
  vk::PhysicalDevice phyDevice;
  vk::Device device;
  vk::Queue graphcisQueue;
  vk::Queue presentQueue;
  vk::SurfaceKHR surface;
  std::unique_ptr<Swapchain> swapchain;
  QueueFamliyIndices queueFamilyIndices;

  void InitSwapchain(int w, int h) { swapchain.reset(new Swapchain(w, h)); }

  void DestroySwapchain() { swapchain.reset(); }

private:
  static std::unique_ptr<Context> instance_;

  Context(const std::vector<const char *> &extensions, CreateSurfaceFunc func);

  void printInstanceExtensions(const std::vector<const char *> &extensions);
  void createInstance(const std::vector<const char *> &extensions);
  void pickupPhyiscalDevice();
  void createDevice();
  void getQueues();

  void queryQueueFamilyIndices();
};

} // namespace toy2d
