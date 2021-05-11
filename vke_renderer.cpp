/* Application Implementation
	- notes...
*/

#include "vke_renderer.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
#include <cassert>

namespace vke {

	// Constructor Imp.
	VkeRenderer::VkeRenderer(VkeWindow& window, VkDerkDevice& device) : vkeWindow{ window }, vkDerkDevice { device } {
		//createPipeline();			// changed to recreateSwapChain for window resize (calls createPipeline)
		recreateSwapChain();
		createCommandBuffers();
	}

	// Destructor Imp.
	VkeRenderer::~VkeRenderer() { freeCommandBuffers(); }

	// Adde for window resizing
	void VkeRenderer::recreateSwapChain() {
		auto extent = vkeWindow.getExtent();	// gets current window size

		// Case: wait in cases that the window is "sizeless", example such as minimization
		while (extent.width == 0 || extent.height == 0) {
			extent = vkeWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(vkDerkDevice.device());		// wait until current swapchain is done being used before making a new one

		// TEMPORARY FIX: cannot have two swapcahins on some devices, destroy old in time being
		//vkeSwapChain = nullptr;

		if (vkeSwapChain == nullptr) {
			vkeSwapChain = std::make_unique<VkeSwapChain>(vkDerkDevice, extent);
		}
		else {
			std::shared_ptr<VkeSwapChain> oldSwapChain = std::move(vkeSwapChain);
			vkeSwapChain = std::make_unique<VkeSwapChain>(vkDerkDevice, extent, oldSwapChain);	// move function used for smart / unique pointer memory management
			
			// Probably better to have a callback for this, but leave for now
			if (!oldSwapChain->compareSwapFormats(*vkeSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
																							
		}

		//createPipeline();	// pipeline relies on swapchain, so create new one with swapchain
	}

	// Command buffs recorded and submitted so they can be reused.
	void VkeRenderer::createCommandBuffers() {

		commandBuffers.resize(VkeSwapChain::MAX_FRAMES_IN_FLIGHT);	// for now, 1:1 commmand to frame buffer ratio

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;		// primary vs. secondary buffers
		allocInfo.commandPool = vkDerkDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vkDerkDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}

	}

	void VkeRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			vkDerkDevice.device(),
			vkDerkDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());

		commandBuffers.clear();
	}

	// Implemented to 
	VkCommandBuffer VkeRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame when already in progress!!!");

		auto result = vkeSwapChain->acquireNextImage(&currentImageIndex);	// fetches index of the frame we should render to next (handles cpu+gpu sync)
		// Detect if window/swap chain has been resized 
		// NOTE - VK_ERROR_OUT_OF_DATE_KHR: surface has changed so that no longer compatible with swapchain --> must get new surface properties and recreate swapchain
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;		// indicate frame has not succesfully started
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;
		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		//std::cout << "beginning command buffer: " << i << '\n';
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	void VkeRenderer::endFrame() {
		assert(isFrameStarted && "Can't endFrame when frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = vkeSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);	// submits provided command buffer TO graphics queue --> command buff then executed
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vkeWindow.wasWindowResized()) {
			vkeWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % VkeSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void VkeRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass when frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer()  && "Can't begin render pass on command buffer from a diff frame");


		// First command: begin render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vkeSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vkeSwapChain->getFrameBuffer(currentImageIndex);

		// Setup render area
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = vkeSwapChain->getSwapChainExtent();	//make sure to use swap and not window exten

		// Clear values (what vals we want frame buff to be initially cleared to)
		// structured in a way that: 0 = color attatchment & 1 = depth attatchment
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Begin render pass
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);	//inline says that subsequent render commands are part of primary buffer (no secondary used)

		// Setup dynamic viewport + scissor with swapchain dimensions
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vkeSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vkeSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0,0}, vkeSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		/*
		// Bind pipeline & issue command
		*/


	}

	void VkeRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass when frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a diff frame");

		// End render pass
		vkCmdEndRenderPass(commandBuffer);
	}

}