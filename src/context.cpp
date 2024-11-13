#include "toy2d/context.hpp"
#include <iostream>
#include <iterator>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

namespace toy2d {

std::unique_ptr<Context> Context::instance_ = nullptr;

void Context::Init() { instance_.reset(new Context); }

void Context::Quit() { instance_.reset(); }

Context::Context() {
  createInstance();
  printPhyiscalDevices();
  pickupPhyiscalDevice();
  queryQueueFamilyIndices();
  createDevice();
  getQueues();
}

Context::~Context() {
  device.destroy();
  instance.destroy();
}

void Context::createInstance() {
  vk::InstanceCreateInfo createInfo;
  vk::ApplicationInfo appInfo;
  appInfo.setApiVersion(VK_API_VERSION_1_3);
  createInfo.setPApplicationInfo(&appInfo);
  instance = vk::createInstance(createInfo);

  std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

  RemoveNosupportedElems<const char *, vk::LayerProperties>(
      layers, vk::enumerateInstanceLayerProperties(),
      [](const char *e1, const vk::LayerProperties &e2) {
        return std::strcmp(e1, e2.layerName) == 0;
      });
  createInfo.setPEnabledLayerNames(layers);

  instance = vk::createInstance(createInfo);
}

void Context::printPhyiscalDevices() {
  auto devices = instance.enumeratePhysicalDevices();

  std::cout << "physical devices | vulkan apiVersion | device type :"
            << std::endl;
  for (auto &device : devices) {
    auto property = device.getProperties();
    auto deviceType = property.deviceType;
    std::cout << "\t" << property.deviceName << " | " << property.apiVersion
              << " | ";
    if (deviceType == vk::PhysicalDeviceType::eCpu) {
      std::cout << "CPU";
    } else if (deviceType == vk::PhysicalDeviceType::eOther) {
      std::cout << "Other";
    } else if (deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      std::cout << "Discrete Gpu";
    } else if (deviceType == vk::PhysicalDeviceType::eVirtualGpu) {
      std::cout << "Virtual Gpu";
    } else if (deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
      std::cout << "Integrated Gpu";
    }
    std::cout << std::endl;
  }
}

void Context::pickupPhyiscalDevice() {
  auto devices = instance.enumeratePhysicalDevices();
  phyDevice = devices[0];

  std::cout << "pick up physical device :" << std::endl;
  std::cout << "\t" << phyDevice.getProperties().deviceName << std::endl;
  std::cout << "physical device support extensions:" << std::endl;
  auto extensions = phyDevice.enumerateDeviceExtensionProperties();
  for (auto &extension : extensions) {
    std::cout << "\t" << extension.extensionName << std::endl;
  }
}

void Context::createDevice() {
  vk::DeviceCreateInfo createInfo;
  vk::DeviceQueueCreateInfo queueCreateInfo;
  float priorities = 1.0;
  queueCreateInfo.setPQueuePriorities(&priorities)
      .setQueueCount(1)
      .setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value());
  createInfo.setQueueCreateInfos(queueCreateInfo);

  device = phyDevice.createDevice(createInfo);
}

void Context::queryQueueFamilyIndices() {
  auto properties = phyDevice.getQueueFamilyProperties();
  for (int i = 0; i < properties.size(); i++) {
    const auto &property = properties[i];
    if (property.queueFlags | vk::QueueFlagBits::eGraphics) {
      queueFamilyIndices.graphicsQueue = i;
      break;
    }
  }
}

void Context::getQueues() {
  graphcisQueue = device.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
}

} // namespace toy2d
