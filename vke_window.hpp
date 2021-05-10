/* Vulkan Window Header File */

#pragma once

// Tells glfw to include vulkan headers as well
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace vke {

	class VkeWindow {

		// Constructor + destructor
		public:
			VkeWindow(int w, int h, std::string name);
			~VkeWindow();

			// Make sure no dangly pointers
			VkeWindow(const VkeWindow&) = delete;
			VkeWindow& operator = (const VkeWindow&) = delete;

			// Check whether window should be closed
			bool shouldClose() { return glfwWindowShouldClose(window); }

			VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

			bool wasWindowResized() { return frameBufferResized; }
			void resetWindowResizedFlag() { frameBufferResized = false; }

			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		// Member variable defs
		private: 
			static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
			void initWindow();	// helper fxn

			//const int width;
			//const int height;
			// changed for dynamic window resizing
			int width;
			int height;
			bool frameBufferResized = false;

			std::string window_name;

			GLFWwindow* window;
	};
}