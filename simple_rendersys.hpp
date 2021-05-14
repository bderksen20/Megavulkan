
#pragma once

#include "vke_pipeline.hpp"
#include "vk_derk_device.hpp"
#include "vke_gameobj.hpp"

#include <memory>
#include <vector>

namespace vke {

	class SimpleRenderSys {

	public:

		SimpleRenderSys(VkDerkDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout descSetLayout);
		~SimpleRenderSys();

		// delete copy constructors
		SimpleRenderSys(const SimpleRenderSys&) = delete;
		SimpleRenderSys& operator = (const SimpleRenderSys&) = delete;
		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<VkeGameObject>& gameObjects, VkDescriptorSet descSet);

		float world_theta = 0;	// identy matrix push constant

		// descriptor layout reference, set by constructor!
		VkDescriptorSetLayout descriptorSetLayoutRef;

	private:

		void createPipelineLayout();

		//TODO
		//void createDescriptorSetLayout();
		void createPipeline(VkRenderPass renderPass);				//render pass used to create pipeline

		VkDerkDevice &vkDerkDevice;

		// Smart pointer!!! Automatically handles mem mgmt.
		std::unique_ptr<VkePipeline> vkePipeline;

		
		VkPipelineLayout pipelineLayout;

	};

}