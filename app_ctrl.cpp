/* Application Implementation
	- notes...
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "simple_rendersys.hpp"
#include "app_ctrl.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
namespace vke {

	// Constructor Imp.
	VkeApplication::VkeApplication() {
		loadGameObjects();
	}

	// Destructor Imp.
	VkeApplication::~VkeApplication() {}

	void VkeApplication::vke_app_run() {

		SimpleRenderSys simpleRenderSys{ vkDerkDevice, vkeRenderer.getSwapChainRenderPass() };


		// While 
		while (!vkeWindow.shouldClose()) {
			glfwPollEvents();
			
			// beginFrame returns null is swapchain needs recreate
			// going through trouble of seperating begin/end frame to allow for multiple render
			//	passes down the line (reflections and other fun stuffs)
			if (auto commandBuffer = vkeRenderer.beginFrame()) {
				vkeRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSys.renderGameObjects(commandBuffer, gameObjects);
				vkeRenderer.endSwapChainRenderPass(commandBuffer);
				vkeRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(vkDerkDevice.device());
	}

	void VkeApplication::loadGameObjects() {

		std::vector<VkeModel::Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto vkeModel = std::make_shared<VkeModel>(vkDerkDevice, vertices);

		std::vector<glm::vec3> colors{
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}  //
		};

		for (int i = 0; i < 40; i++) {
			auto triangle = VkeGameObject::createGameObject();
			triangle.model = vkeModel;
			//triangle.color = { .1f, .8f * log(i), .1f };
			triangle.color = colors[i % colors.size()];
			triangle.transform2d.translation.x = .2f;
			triangle.transform2d.scale = glm::vec2(.5f);
			triangle.transform2d.scale += (i * 0.02f);
			triangle.transform2d.rotation = .0025f * glm::pi<float>() * i;		//

			gameObjects.push_back(std::move(triangle));
		}

	}

}