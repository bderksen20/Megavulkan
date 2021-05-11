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

	private:

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		// Init this app's window!
		VkeWindow& vkeWindow;
		VkDerkDevice& vkDerkDevice;
		std::unique_ptr<VkeSwapChain> vkeSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		// To track state of frame in progress
		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted;

	};

}