#pragma once

#include "vulkan/vulkan.hpp"
#include <iostream>
#include <limits>

namespace toy2d {

class Renderer final {
public:
  Renderer();
  ~Renderer();

  void Render();

private:
  vk::CommandPool cmdPool_;
  vk::CommandBuffer cmdBuf_;

  vk::Semaphore imageAvaliable_;
  vk::Semaphore imageDrawFinish_;
  vk::Fence cmdAvaliableFence_;

  void initCmdPool();
  void allocCmdBuf();
  void createSems();
  void createFence();
};

} // namespace toy2d
