// Class to encapsulate lots of functionality
#pragma once

#include "vke_window.hpp"
#include "vk_derk_device.hpp"
#include "vke_swap_chain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace vke {

	class VkeRenderer {

	public:

		VkeRenderer(VkeWindow& vkeWindow, VkDerkDevice &vkDerkDevice);
		~VkeRenderer();

		// delete copy constructors
		VkeRenderer(const VkeRenderer&) = delete;
		VkeRenderer& operator = (const VkeRenderer&) = delete;
		
		VkRenderPass getSwapChainRenderPass() const { return vkeSwapChain->getRenderPass(); }
		bool isFrameInProgress() const {return isFrameStarted;}

		VkCommandBuffer getCurrentCommandBuffer() const { 
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress.");
			return commandBuffers[currentFrameIndex]; 
		}

		VkDescriptorSet getCurrentDescriptorSet() const {
			return descriptorSets[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		//void run() {}; empty implementation
		void vke_app_run();

		// Split command buffer handling into beginning and end
		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		// descriptor set layout, sent to render system for binding
		VkDescriptorSetLayout descriptorSetLayout;


	private:

		// currently handles uniform buffers + descriptor sets
		void createUniformBuffers();
		void createCommandBuffers();
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();

		void freeUniformBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		void updateUniformBuffer(uint32_t currentImage);

		// Init this app's window!
		VkeWindow& vkeWindow;
		
		VkDerkDevice& vkDerkDevice;
		std::unique_ptr<VkeSwapChain> vkeSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		// To track state of frame in progress
		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted;

		// Uniform buffers, descriptor sets, and more
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;


	};

}