#pragma once

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "command_manager.hpp"
#include "render_process.hpp"
#include "swapchain.hpp"
#include "tool.hpp"
#include "vulkan/vulkan.hpp"

namespace toy2d {

class Context {
public:
  using GetSurfaceCallback = std::function<VkSurfaceKHR(VkInstance)>;
  friend void Init(std::vector<const char *> &, GetSurfaceCallback, int, int);

  static void Init(std::vector<const char *> &extensions, GetSurfaceCallback);
  static void Quit();
  static Context &Instance();

  struct QueueInfo {
    std::optional<uint32_t> graphicsIndex;
    std::optional<uint32_t> presentIndex;
  } queueInfo;

  vk::Instance instance;
  vk::PhysicalDevice phyDevice;
  vk::Device device;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  std::unique_ptr<Swapchain> swapchain;
  std::unique_ptr<RenderProcess> renderProcess;
  std::unique_ptr<CommandManager> commandManager;

private:
  static Context *instance_;
  vk::SurfaceKHR surface_;

  GetSurfaceCallback getSurfaceCb_ = nullptr;

  Context(std::vector<const char *> &extensions, GetSurfaceCallback);
  ~Context();

  void initRenderProcess();
  void initSwapchain(int windowWidth, int windowHeight);
  void initGraphicsPipeline();
  void initCommandPool();

  vk::Instance createInstance(std::vector<const char *> &extensions);
  vk::PhysicalDevice pickupPhysicalDevice();
  vk::Device createDevice(vk::SurfaceKHR);

  void queryQueueInfo(vk::SurfaceKHR);
};

} // namespace toy2d
