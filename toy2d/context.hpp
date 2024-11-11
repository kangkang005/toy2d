#pragma once

#include "tool.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <iostream>
#include <memory>

namespace toy2d {

class Context final {
public:
  static void Init();
  static void Quit();

  static Context &GetInstance() {
    assert(instance_);
    return *instance_;
  }

  ~Context();

  vk::Instance instance;

private:
  static std::unique_ptr<Context> instance_;

  Context();
};

} // namespace toy2d
