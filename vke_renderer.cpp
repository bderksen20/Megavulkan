/* Application Implementation
	- notes...
*/

#include "vke_renderer.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
#include <cassert>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace vke {

	// Uniform buffer struct for transformations
	struct UniformBuffo {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	// Constructor Imp.
	VkeRenderer::VkeRenderer(VkeWindow& window, VkDerkDevice& device) : vkeWindow{ window }, vkDerkDevice { device } {
		//createPipeline();			// changed to recreateSwapChain for window resize (calls createPipeline)
		
		// TODO:: ok to put in renderer? does this make sense??
		createDescriptorSetLayout();

		recreateSwapChain();
		createUniformBuffers();		// swap chain needs to be created before this
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
	}

	// Destructor Imp.
	// TODO: test uniform buffer destruction with renderer 
	// - NOTE: should they be destroyed with the swap chain??? probably.... also need to recreate with new swapchain then
	VkeRenderer::~VkeRenderer() { 
		freeCommandBuffers(); 
		freeUniformBuffers();
	}

	// Create descriptor layout for uniform buffer
	void VkeRenderer::createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};							// need to describe binding via this struct
		uboLayoutBinding.binding = 0;												// specify binding used in shader
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;										// possible to have multiple values in array (could have mult transforms, one for a bone in a skeleton for animation?)
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;					// describe in which shaders this descriptor is used (only vertex for now)
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(vkDerkDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

	}
	// TODO: janky
	//void VkeRenderer::setDescriptorSetLayoutRef(VkDescriptorSetLayout descriptorSetLayout) { descriptorSetLayoutRef = descriptorSetLayout; }

	// TODO:: ok to put in renderer? does this make sense??
	// Allocate uniform buffers
	void VkeRenderer::createUniformBuffers() {

		VkDeviceSize bufferSize = sizeof(UniformBuffo);

		uniformBuffers.resize(vkeSwapChain->imageCount());			// pointer to swapchain, get n images in swapchain
		uniformBuffersMemory.resize(vkeSwapChain->imageCount());

		// Iterate to create buffers
		for (size_t i = 0; i < vkeSwapChain->imageCount(); i++) {
			vkDerkDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i], uniformBuffersMemory[i]);
		}

	}

	// TODO:: test
	void VkeRenderer::createDescriptorSets() {

		// describe descriptor set allocation: specify pool to alloc from, number of desc. sets, and the layout (IN RENDERSYS???)
		std::vector<VkDescriptorSetLayout> layouts(vkeSwapChain->imageCount(), descriptorSetLayout);		//descriptorSetLayoutRef may be messed up
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(vkeSwapChain->imageCount());
		allocInfo.pSetLayouts = layouts.data();

		// allocate into member var
		descriptorSets.resize(vkeSwapChain->imageCount());
		if (vkAllocateDescriptorSets(vkDerkDevice.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		// desc. sets alloc'd, now populate every descriptor
		for (size_t i = 0; i < vkeSwapChain->imageCount(); i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBuffo);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(vkDerkDevice.device(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	// TODO:: test
	void VkeRenderer::createDescriptorPool() {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(vkeSwapChain->imageCount());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		poolInfo.maxSets = static_cast<uint32_t>(vkeSwapChain->imageCount());	// max num of descriptor sets that can be alloced
		if (vkCreateDescriptorPool(vkDerkDevice.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	// TODO:: test
	void VkeRenderer::updateUniformBuffer(uint32_t currentImage) {

		// Chrono timekeeping...
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		// Create and prime transforms...
		UniformBuffo ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), vkeSwapChain->getSwapChainExtent().width / (float)vkeSwapChain->getSwapChainExtent().height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		// Copy buff into current uniform buffer
		void* data;
		vkMapMemory(vkDerkDevice.device(), uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(vkDerkDevice.device(), uniformBuffersMemory[currentImage]);

	}

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

	// TODO: test
	void VkeRenderer::freeUniformBuffers() {
		for (size_t i = 0; i < vkeSwapChain->imageCount(); i++) {
			vkDestroyBuffer(vkDerkDevice.device(), uniformBuffers[i], nullptr);
			vkFreeMemory(vkDerkDevice.device(), uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(vkDerkDevice.device(), descriptorPool, nullptr);
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

		// TODO:: test uniform buffer
		updateUniformBuffer(currentImageIndex);

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