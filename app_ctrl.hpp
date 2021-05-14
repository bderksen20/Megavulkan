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

			// Replaced for game objs tutorial
			std::vector<VkeGameObject> gameObjects;
	};

}