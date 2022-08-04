#include "Star_Window.hpp"

namespace star::core{
	StarWindow::StarWindow(int width, int height, std::string name, 
        GLFWkeyfun keyboardCallbackFunction, GLFWmousebuttonfun mouseButtonCallback, 
        GLFWcursorposfun cursorPositionCallback, GLFWscrollfun scrollCallback) : 
        height(height), 
        width(width)
    {
		initWindow(keyboardCallbackFunction, mouseButtonCallback, cursorPositionCallback,scrollCallback);
	}

	StarWindow::~StarWindow() {
		glfwDestroyWindow(this->window); 
		glfwTerminate(); 
	}

    void star::core::StarWindow::createWindowSurface(vk::Instance instance, vk::UniqueSurfaceKHR& surface) {
        VkSurfaceKHR surfaceTmp;
        if (glfwCreateWindowSurface(instance, this->window, nullptr, &surfaceTmp) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
        surface = vk::UniqueSurfaceKHR(surfaceTmp, instance);
    }

    void StarWindow::initWindow(GLFWkeyfun keyboardCallbackFunction, GLFWmousebuttonfun mouseButtonCallback, GLFWcursorposfun cursorPositionCallback, GLFWscrollfun scrollCallback) {
        //actually make sure to init glfw
        glfwInit();
        //tell GLFW to create a window but to not include a openGL instance as this is a default behavior
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //disable resizing functionality in glfw as this will not be handled in the first tutorial
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        //create a window, 3rd argument allows selection of monitor, 4th argument only applies to openGL
        this->window = glfwCreateWindow(width, height, "Starlight", nullptr, nullptr);

        //need to give GLFW a pointer to current instance of this class
        glfwSetWindowUserPointer(this->window, this);

        // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

        //set keyboard callbacks
        auto callback = glfwSetKeyCallback(this->window, keyboardCallbackFunction);

        auto cursorCallback = glfwSetCursorPosCallback(this->window, cursorPositionCallback);

        auto mouseBtnCallback = glfwSetMouseButtonCallback(this->window, mouseButtonCallback);

        auto mouseScrollCallback = glfwSetScrollCallback(this->window, scrollCallback);

        // this->glfwRequiredExtensions = std::make_unique<std::vector<vk::ExtensionProperties>>(new std::vector<vk::ExtensionProperties>(**requiredExtensions)); 
        //this->glfwRequiredExtensions = std::make_unique<const char**>(glfwGetRequiredInstanceExtensions(this->glfwRequiredExtensionsCount.get()));
    }
}