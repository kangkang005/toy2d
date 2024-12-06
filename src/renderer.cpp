#include "toy2d/renderer.hpp"

namespace toy2d {

const std::array<Vertex, 3> vertices = {
    Vertex{0.0, -0.5},
    Vertex{0.5, 0.5},
    Vertex{-0.5, 0.5},
};

Renderer::Renderer(int maxFlightCount)
    : maxFlightCount_(maxFlightCount), curFrame_(0) {
    createFences();
    createSemaphores();
    createCmdBuffers();
    createVertexBuffer();
    bufferVertexData();
}

Renderer::~Renderer() {
    hostVertexBuffer_.reset();
    deviceVertexBuffer_.reset();
    auto& device = Context::Instance().device;
    for (auto& sem : imageAvaliableSems_) {
        device.destroySemaphore(sem);
    }
    for (auto& sem : renderFinishSems_) {
        device.destroySemaphore(sem);
    }
    for (auto& fence : fences_) {
        device.destroyFence(fence);
    }
}

void Renderer::DrawTriangle() {
    auto& ctx = Context::Instance();
    auto& device = ctx.device;
    // waitAll is true, mean that wait for all fences changed to signaled
    if (device.waitForFences(fences_[curFrame_], true,
                             std::numeric_limits<uint64_t>::max()) !=
        vk::Result::eSuccess) {
        throw std::runtime_error("wait for fence failed");
    }
    device.resetFences(fences_[curFrame_]);

    auto& swapchain = ctx.swapchain;
    auto resultValue = device.acquireNextImageKHR(
        swapchain->swapchain, std::numeric_limits<uint64_t>::max(),
        imageAvaliableSems_[curFrame_], nullptr);
    if (resultValue.result != vk::Result::eSuccess) {
        throw std::runtime_error("wait for image in swapchain failed");
    }
    auto imageIndex = resultValue.value;

    auto& cmdMgr = ctx.commandManager;
    cmdBufs_[curFrame_].reset();

    // acquire swapchain image
    // beginComamndBuffer {
    //   beginRenderPass {
    //     bind framebuffer
    //     bind resources: pipline, geometry, descriptor set..
    //     Draw
    //   } endRenderPass
    // } endComamndBuffer
    // submit command buffer to queue
    // present
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBufs_[curFrame_].begin(beginInfo);
    {
        vk::ClearValue clearValue;
        clearValue.setColor(
            vk::ClearColorValue(std::array<float, 4>{0.1, 0.1, 0.1, 1}));
        vk::RenderPassBeginInfo renderPassBegin;
        renderPassBegin.setRenderPass(ctx.renderProcess->renderPass)
            .setFramebuffer(swapchain->framebuffers[imageIndex])
            .setClearValues(clearValue)
            .setRenderArea(vk::Rect2D({}, swapchain->GetExtent()));
        cmdBufs_[curFrame_].beginRenderPass(&renderPassBegin,
                                            vk::SubpassContents::eInline);
        {
            cmdBufs_[curFrame_].bindPipeline(
                vk::PipelineBindPoint::eGraphics,
                ctx.renderProcess->graphicsPipeline);
            vk::DeviceSize offset = 0;
            cmdBufs_[curFrame_].bindVertexBuffers(
                0, deviceVertexBuffer_->buffer, offset);
            cmdBufs_[curFrame_].draw(3, 1, 0, 0);
        }
        cmdBufs_[curFrame_].endRenderPass();
    }
    cmdBufs_[curFrame_].end();

    // command submit queue to GPU by command buffer
    vk::SubmitInfo submit;
    vk::PipelineStageFlags flags =
        vk::PipelineStageFlagBits::eColorAttachmentOutput;
    // comamnd buffer execution wait for finishing to read image from swapchain
    submit.setCommandBuffers(cmdBufs_[curFrame_])
        .setWaitSemaphores(imageAvaliableSems_[curFrame_])
        // DstStageMask of subpass dependency
        .setWaitDstStageMask(flags)
        .setSignalSemaphores(renderFinishSems_[curFrame_]);
    ctx.graphicsQueue.submit(submit, fences_[curFrame_]);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(renderFinishSems_[curFrame_])
        .setSwapchains(swapchain->swapchain)
        .setImageIndices(imageIndex);
    if (ctx.presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
        throw std::runtime_error("present queue execute failed");
    }

    curFrame_ = (curFrame_ + 1) % maxFlightCount_;
}

// sync between CPU with GPU, GPU send fence state(signaled or unsignaled) to
// CPU
void Renderer::createFences() {
    fences_.resize(maxFlightCount_, nullptr);

    for (auto& fence : fences_) {
        vk::FenceCreateInfo fenceCreateInfo;
        // init fence are in signaled state, avoid to stay at wait in the
        // beginning
        fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        fence = Context::Instance().device.createFence(fenceCreateInfo);
    }
}

// sync between queues at GPU
void Renderer::createSemaphores() {
    auto& device = Context::Instance().device;
    vk::SemaphoreCreateInfo info;

    imageAvaliableSems_.resize(maxFlightCount_);
    renderFinishSems_.resize(maxFlightCount_);

    for (auto& sem : imageAvaliableSems_) {
        sem = device.createSemaphore(info);
    }

    for (auto& sem : renderFinishSems_) {
        sem = device.createSemaphore(info);
    }
}

void Renderer::createCmdBuffers() {
    cmdBufs_.resize(maxFlightCount_);

    for (auto& cmd : cmdBufs_) {
        cmd = Context::Instance().commandManager->CreateOneCommandBuffer();
    }
}

// create staging buffer(CPU) and device buffer(GPU)
void Renderer::createVertexBuffer() {
    // vertex data -> host buffer(eTransferSrc) -> device buffer(eTransferDst)
    // eHostVisible: host can visit memory but not device do, define host memory
    hostVertexBuffer_.reset(
        new Buffer(sizeof(vertices), vk::BufferUsageFlagBits::eTransferSrc,
                   vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent));
    // eDeviceLocal: device can visit memory but not host do, define device
    // memory
    deviceVertexBuffer_.reset(
        new Buffer(sizeof(vertices),
                   vk::BufferUsageFlagBits::eVertexBuffer |
                       vk::BufferUsageFlagBits::eTransferDst,
                   vk::MemoryPropertyFlagBits::eDeviceLocal));
}

void Renderer::bufferVertexData() {
    // map memory that buffer must include
    //  vk::MemoryPropertyFlagBits::eHostVisible usage
    void* ptr = Context::Instance().device.mapMemory(
        hostVertexBuffer_->memory, 0, hostVertexBuffer_->size);
    // copy vextex data to host buffer, vectex data -> host buffer
    memcpy(ptr, vertices.data(), sizeof(vertices));
    // cancel map after copy
    Context::Instance().device.unmapMemory(hostVertexBuffer_->memory);

    auto cmdBuf = Context::Instance().commandManager->CreateOneCommandBuffer();

    vk::CommandBufferBeginInfo begin;
    begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    // copy vertex data from host buffer to device buffer, host buffer -> device
    // buffer
    cmdBuf.begin(begin);
    {
        vk::BufferCopy region;
        region.setSize(hostVertexBuffer_->size).setSrcOffset(0).setDstOffset(0);
        // copy data between buffers
        // srcBuffer: buffer must include vk::BufferUsageFlagBits::eTransferSrc
        // dstBuffer: buffer must include vk::BufferUsageFlagBits::eTransferDst
        cmdBuf.copyBuffer(hostVertexBuffer_->buffer,
                          deviceVertexBuffer_->buffer, region);
    }
    cmdBuf.end();

    vk::SubmitInfo submit;
    submit.setCommandBuffers(cmdBuf);
    Context::Instance().graphicsQueue.submit(submit);

    // GPU wait for which finish to copy from host buffer to device buffer
    // device.waitIdle: CPU wait for all task executed finally at GPU
    Context::Instance().device.waitIdle();

    // host buffer is not useful when finish to copy, release it.
    Context::Instance().commandManager->FreeCmd(cmdBuf);
}

} // namespace toy2d
