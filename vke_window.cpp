/* Vulkan Window Implementation */

#include "vke_window.hpp"
#include <stdexcept>

namespace vke {

	// Constructor Imp: member initializer list
	VkeWindow::VkeWindow(int w, int h, std::string name) : width{ w }, height{ h }, window_name{ name }{
		initWindow();
	}
	
	// Init window imp.
	void VkeWindow::initWindow() {

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// glfw base is in context of opengl: tell it not to do that
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);		// disable window resizing after creation
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		// Create window pointer
		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);

		// Addded for window resizing
		glfwSetWindowUserPointer(window, this);								// window's pointer is paired to the same value as the parent "vke" obj
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);	// NOTE: glfw library allows for a callbak to be registered and performed whenever window is resized
	}

	void VkeWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {

		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}

	}

	// Callback for setting new window sizes
	void VkeWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto vkeWindow = reinterpret_cast<VkeWindow*>(glfwGetWindowUserPointer(window));
		vkeWindow->frameBufferResized = true;
		vkeWindow->width = width;
		vkeWindow->height = height;
	}

	// Destructor: destroy window + end glfw
	VkeWindow::~VkeWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}