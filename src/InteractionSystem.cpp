#include "InteractionSystem.h"

std::vector<std::unique_ptr<std::function<void(int, int, int, int)>>> star::core::InteractionSystem::keyboardCallbacks		= std::vector<std::unique_ptr<std::function<void(int, int, int, int)>>>();
std::vector<std::unique_ptr<std::function<void(double, double)>>> star::core::InteractionSystem::mouseMovementCallbacks	= std::vector<std::unique_ptr<std::function<void(double, double)>>>();
std::vector<std::unique_ptr<std::function<void(int, int, int)>>> star::core::InteractionSystem::mouseButtonCallbacks		= std::vector<std::unique_ptr<std::function<void(int, int, int)>>>(); 
std::vector<std::unique_ptr<std::function<void(double, double)>>> star::core::InteractionSystem::mouseScrollCallbacks		= std::vector<std::unique_ptr<std::function<void(double, double)>>>();

void star::core::InteractionSystem::registerKeyCallback(std::unique_ptr<std::function<void(int, int, int, int)>> newKeyCallback)
{
	keyboardCallbacks.push_back(std::move(newKeyCallback));
}

void star::core::InteractionSystem::registerMouseMovementCallback(std::unique_ptr<std::function<void(double, double)>> newMouseMvmtCallback)
{
	mouseMovementCallbacks.push_back(std::move(newMouseMvmtCallback));
}

void star::core::InteractionSystem::registerMouseButtonCallback(std::unique_ptr<std::function<void(int, int, int)>> newMouseButtonCallback)
{
	mouseButtonCallbacks.push_back(std::move(newMouseButtonCallback)); 
}

void star::core::InteractionSystem::registerMouseScrollCallback(std::unique_ptr<std::function<void(double, double)>> newMouseScrollCallback)
{
	mouseScrollCallbacks.push_back(std::move(newMouseScrollCallback)); 
}

void star::core::InteractionSystem::glfwKeyHandle(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::function<void(int, int, int, int)> call; 
	//call all registered functions
	for (size_t i = 0; i < keyboardCallbacks.size(); i++) {
		call = *keyboardCallbacks.at(i);
		call(key, scancode, action, mods);
	}
}

void star::core::InteractionSystem::glfwMouseMovement(GLFWwindow* window, double xpos, double ypos)
{
	std::function<void(double, double)> call;

	for (size_t i = 0; i < mouseMovementCallbacks.size(); i++) {
		call = *mouseMovementCallbacks.at(i); 
		call(xpos, ypos); 
	}
}

void star::core::InteractionSystem::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	std::function<void(int, int, int)> call; 

	for (size_t i = 0; i < mouseButtonCallbacks.size(); i++) {
		call = *mouseButtonCallbacks.at(i); 
		call(button, action, mods); 
	}
}

void star::core::InteractionSystem::glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	std::function<void(double, double)> call;

	for (size_t i = 0; i < mouseScrollCallbacks.size(); i++) {
		call = *mouseScrollCallbacks.at(i); 
		call(xoffset, yoffset);
	}
}