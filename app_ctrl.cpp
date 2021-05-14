/* Application Implementation
	- notes...
*/

// GLM - linear algebra math stuff
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Tiny object loader
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>		

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

		// init render system and pass renderer's descriptor set layout 
		SimpleRenderSys simpleRenderSys{ vkDerkDevice, vkeRenderer.getSwapChainRenderPass(), vkeRenderer.descriptorSetLayout};

		// While 
		while (!vkeWindow.shouldClose()) {
			glfwPollEvents();
			
			// beginFrame returns null is swapchain needs recreate
			// going through trouble of seperating begin/end frame to allow for multiple render
			//	passes down the line (reflections and other fun stuffs)
			if (auto commandBuffer = vkeRenderer.beginFrame()) {
				vkeRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSys.renderGameObjects(commandBuffer, gameObjects, vkeRenderer.getCurrentDescriptorSet());	// pull in current descriptor set
				vkeRenderer.endSwapChainRenderPass(commandBuffer);
				vkeRenderer.endFrame();
			}
		}

		// program cleanup????
		//  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		vkDeviceWaitIdle(vkDerkDevice.device());
	}

	void VkeApplication::loadGameObjects() {

		// Model loading sequence - viking room example

		// Step 1. create necessary variables (vertices, etc.)
		std::vector<VkeModel::Vertex> vertices;

		// Step 2. load the model using tinyobj loader...
		// - obj files contain: positions, normals, texture coords, and faces
		tinyobj::attrib_t attrib;							// holds all pos, norms, tex coords in attrib.XXX
		std::vector<tinyobj::shape_t> shapes;				// holds all seperate objs and faces (faces have arrays of verts)
		std::vector<tinyobj::material_t> materials;
		std::string warn, err, path = "./viking_room.obj";	

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
			throw std::runtime_error(warn + err);
		}

		glm::vec3 color = { 0.5f, 0.1f, 0.1f };

		// Iterate over shapes (combine faces into single model)
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				VkeModel::Vertex vertex{};

				vertex.position = { 
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2],
					0.0f
				};

				vertex.color[0] = ((double)rand() / (RAND_MAX));
				vertex.color[1] = ((double)rand() / (RAND_MAX));
				vertex.color[2] = ((double)rand() / (RAND_MAX));


				vertices.push_back(vertex);
			}
		}

		// Step 3. create a VkeModel - upon construction, a vertex buffer is automatically created
		auto vkeModel = std::make_shared<VkeModel>(vkDerkDevice, vertices);
		
		// Step 4. create gameObj to hold the model
		auto viking_room = VkeGameObject::createGameObject();
		viking_room.model = vkeModel;
		gameObjects.push_back(std::move(viking_room));

		// Model Loading Sequence
		//auto lmodel = VkeGameObject::createGameObject();
		//lmodel.model = vkeModel;

		// Triangle Loading Sequence
		/*
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
		*/

	}

}