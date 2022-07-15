#pragma once 

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace star::core{
	class StarWindow {
	public:
		StarWindow(int width, int height, std::string name, 
			GLFWkeyfun keyboardCallbackFunction, GLFWmousebuttonfun mouseButtonCallback, 
			GLFWcursorposfun cursorPositionCallback, GLFWscrollfun scrollCallback);

		~StarWindow(); 

		//no copy 
		StarWindow(const StarWindow&) = delete; 
		StarWindow& operator=(const StarWindow&) = delete; 

		void createWindowSurface(vk::Instance instance, vk::UniqueSurfaceKHR& surface);

		bool shouldClose() { return glfwWindowShouldClose(this->window); }
		vk::Extent2D getExtent() { return { static_cast<uint32_t>(this->width), static_cast<uint32_t>(this->height) }; }
		bool wasWindowResized() { return this->frambufferResized; }
		void resetWindowResizedFlag() { this->frambufferResized = false; }
		GLFWwindow* getGLFWwindow() const { return this->window; }

	protected:

	private: 
		int height, width; 
		bool frambufferResized = false; 
		std::string windowName; 
		GLFWwindow* window; 

		void initWindow(GLFWkeyfun keyboardCallbackFunction, GLFWmousebuttonfun mouseButtonCallback, GLFWcursorposfun cursorPositionCallback, GLFWscrollfun scrollCallback);
	};
}