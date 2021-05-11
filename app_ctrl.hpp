/* Application Header 
	- HAS a vke window!
*/
#pragma once

#include "vke_window.hpp"
#include "vk_derk_device.hpp"
#include "vke_gameobj.hpp"
#include "vke_renderer.hpp"

#include <memory>
#include <vector>

namespace vke {

	class VkeApplication {

		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

			VkeApplication();
			~VkeApplication();

			// delete copy constructors
			VkeApplication(const VkeApplication&) = delete;
			VkeApplication& operator = (const VkeApplication&) = delete;

			//void run() {}; empty implementation
			void vke_app_run();

		private:

			//void loadModels();
			void loadGameObjects();

			// Init this app's window!
			VkeWindow vkeWindow{WIDTH, HEIGHT, "VK Window..."};
			VkDerkDevice vkDerkDevice{ vkeWindow };
			VkeRenderer vkeRenderer{ vkeWindow, vkDerkDevice };

			// Init graphics pipeline! Removed for new unique pipeline
			// VkePipeline vkePipeline{vkDerkDevice, "simple_shader.vert.spv", "simple_shader.frag.spv", VkePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};

			// Replaced for game objs tutorial
			//std::unique_ptr<VkeModel> vkeModel;
			std::vector<VkeGameObject> gameObjects;
	};

}