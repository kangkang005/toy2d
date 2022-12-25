#include "toy2d/shader.hpp"
#include "toy2d/context.hpp"

namespace toy2d {

std::unique_ptr<Shader> Shader::instance_ = nullptr;

void Shader::Init(const std::string& vertexSource, const std::string& fragSource) {
    instance_.reset(new Shader(vertexSource, fragSource));
}

void Shader::Quit() {
    instance_.reset();
}

Shader::Shader(const std::string& vertexSource, const std::string& fragSource) {
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = vertexSource.size();
    createInfo.pCode = (uint32_t*)vertexSource.data();

    vertexModule = Context::GetInstance().device.createShaderModule(createInfo);

    createInfo.codeSize = fragSource.size();
    createInfo.pCode = (uint32_t*)fragSource.data();
    fragmentModule = Context::GetInstance().device.createShaderModule(createInfo);

    initStage();
}

Shader::~Shader() {
    auto& device = Context::GetInstance().device;
    device.destroyShaderModule(vertexModule);
    device.destroyShaderModule(fragmentModule);
}

void Shader::initStage() {
    stage_.resize(2);
    stage_[0].setStage(vk::ShaderStageFlagBits::eVertex)
             .setModule(vertexModule)
             .setPName("main");
    stage_[1].setStage(vk::ShaderStageFlagBits::eFragment)
             .setModule(fragmentModule)
             .setPName("main");
}



}