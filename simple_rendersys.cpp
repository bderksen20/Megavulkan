
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "simple_rendersys.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
namespace vke {

	struct SimplePushConstantData {
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;		// ADDED TO FIX PUSH CONSTANT ALIGNMENT
	};

	// Constructor Imp.
	SimpleRenderSys::SimpleRenderSys(VkDerkDevice& device, VkRenderPass renderPass) : vkDerkDevice{ device } {
		createPipelineLayout();
		createPipeline(renderPass);
	}

	// Destructor Imp.
	SimpleRenderSys::~SimpleRenderSys() {
		vkDestroyPipelineLayout(vkDerkDevice.device(), pipelineLayout, nullptr);
		//createPipeline();
	}

	void SimpleRenderSys::createPipelineLayout() {

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

	void SimpleRenderSys::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");


		PipelineConfigInfo pipelineConfig{};
		VkePipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		vkePipeline = std::make_unique<VkePipeline>(
			vkDerkDevice,
			"simple_shader.vert.spv",
			"simple_shader.frag.spv",
			pipelineConfig);
	}

	void SimpleRenderSys::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<VkeGameObject> &gameObjects) {

		int i = 0;
		for (auto& obj : gameObjects) {
			i += 1;
			obj.transform2d.rotation = glm::mod<float>(obj.transform2d.rotation + 0.001f * i, 2.f * glm::pi<float>());
		}

		vkePipeline->bind(commandBuffer);

		for (auto& obj : gameObjects) {
			//obj.transform2d.rotation = glm::mod(obj.transform2d.rotation + 0.001f, glm::two_pi<float>());		// make the game objs rotate!!!

			SimplePushConstantData push{};
			push.offset = obj.transform2d.translation;
			push.color = obj.color;
			push.transform = obj.transform2d.mat2();

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}

	}

}