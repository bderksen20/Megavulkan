/* Application Implementation
	- notes...
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "app_ctrl.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
namespace vke {

	struct SimplePushConstantData {
		glm::vec2 offset;
		glm::vec3 color;
	};

	// Constructor Imp.
	VkeApplication::VkeApplication() {
		loadModels();
		createPipelineLayout();
		//createPipeline();			// changed to recreateSwapChain for window resize (calls createPipeline)
		recreateSwapChain();
		createCommandBuffers();
	}

	// Destructor Imp.
	VkeApplication::~VkeApplication() {
		vkDestroyPipelineLayout(vkDerkDevice.device(), pipelineLayout, nullptr);
		createPipeline();
		createCommandBuffers();
	}

	void VkeApplication::vke_app_run() {

		// While 
		while (!vkeWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(vkDerkDevice.device());
	}

	void VkeApplication::loadModels() {

		std::vector<VkeModel::Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		vkeModel = std::make_unique<VkeModel>(vkDerkDevice, vertices);
	}

	void VkeApplication::createPipelineLayout() {

		// Push constant range
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;	// signals that we want push constant data in both vertex and fragment shaders!
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		//pipelineLayoutInfo.pushConstantRangeCount = 0;	updated for push constants
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(vkDerkDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void VkeApplication::createPipeline() {
		assert(vkeSwapChain != nullptr && "Cannot create pipeline before swapchain!");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");


		PipelineConfigInfo pipelineConfig{};
		VkePipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = vkeSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		vkePipeline = std::make_unique<VkePipeline>(
			vkDerkDevice,
			"simple_shader.vert.spv",
			"simple_shader.frag.spv",
			pipelineConfig);
	}

	// Adde for window resizing
	void VkeApplication::recreateSwapChain() {
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
			vkeSwapChain = std::make_unique<VkeSwapChain>(vkDerkDevice, extent, std::move(vkeSwapChain));	// move function used for smart / unique pointer memory management
			// Free and create new command buffers if....
			if (vkeSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		createPipeline();	// pipeline relies on swapchain, so create new one with swapchain
	}

	// Command buffs recorded and submitted so they can be reused.
	void VkeApplication::createCommandBuffers() {

		commandBuffers.resize(vkeSwapChain->imageCount());	// for now, 1:1 commmand to frame buffer ratio

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;		// primary vs. secondary buffers
		allocInfo.commandPool = vkDerkDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vkDerkDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}

		// Removed for window resizing tutorial
		// Record draw commands to buffers
		//for (int i = 0; i < commandBuffers.size(); i++) {

			// Removed for window resizing tutorial
			// Relocated to "recordCommandBuffer" fxn
			/*
			*/
		//}
		
	}

	void VkeApplication::freeCommandBuffers() {
		vkFreeCommandBuffers(
			vkDerkDevice.device(), 
			vkDerkDevice.getCommandPool(), 
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());

		commandBuffers.clear();
	}

	void VkeApplication::recordCommandBuffer(int imageIndex) {

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		//std::cout << "beginning command buffer: " << i << '\n';
		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// First command: begin render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vkeSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vkeSwapChain->getFrameBuffer(imageIndex);

		// Setup render area
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = vkeSwapChain->getSwapChainExtent();	//make sure to use swap and not window exten

		// Clear values (what vals we want frame buff to be initially cleared to)
		// structured in a way that: 0 = color attatchment & 1 = depth attatchment
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Begin render pass
		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);	//inline says that subsequent render commands are part of primary buffer (no secondary used)

		// Setup dynamic viewport + scissor with swapchain dimensions
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vkeSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vkeSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0,0}, vkeSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		// Bind pipeline & issue command
		vkePipeline->bind(commandBuffers[imageIndex]);
		//vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);	// draw command: draw 3 vertices in only 1 instance
		vkeModel->bind(commandBuffers[imageIndex]);

		// APPLY PUSH CONSTANTS BEFORE DRAW
		// Draw vertical column of copys of model varying in color/position!
		for (int j = 0; j < 4; j++) {
			SimplePushConstantData push{};
			push.offset = { 0.0f, -0.4f + j * 0.25f };
			push.color = { 0.0f, 0.0f, 0.2f + 0.2f * j };

			vkCmdPushConstants(
				commandBuffers[imageIndex], 
				pipelineLayout, 
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
				0, 
				sizeof(SimplePushConstantData), 
				&push);

			// Drawing 4 triangles / models
			vkeModel->draw(commandBuffers[imageIndex]);
		}

		//vkeModel->draw(commandBuffers[imageIndex]);

		// End render pass
		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	
	void VkeApplication::drawFrame() {
		uint32_t imageIndex;
		auto result = vkeSwapChain->acquireNextImage(&imageIndex);	// fetches index of the frame we should render to next (handles cpu+gpu sync)

		// Detect if window/swap chain has been resized 
		// NOTE - VK_ERROR_OUT_OF_DATE_KHR: surface has changed so that no longer compatible with swapchain --> must get new surface properties and recreate swapchain
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		recordCommandBuffer(imageIndex);
		result = vkeSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);	// submits provided command buffer TO graphics queue --> command buff then executed
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vkeWindow.wasWindowResized()) {
			vkeWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

	}
}