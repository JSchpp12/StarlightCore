#pragma once 

#include "SC/Interactivity.hpp"

#include <glfw/glfw3.h>

#include <optional>
#include <functional>
#include <vector>

namespace star {
	namespace core {
		class InteractionSystem {
		public:

			/// <summary>
			/// Register a new user defined keyboard callback
			/// </summary>
			/// <param name="userKeyCallback">The callback function to register. Must be of type foo(int, int, int, int)</param>
			static void registerKeyCallback(std::unique_ptr<std::function<void(int, int, int, int)>> newKeyCallback);

			/// <summary>
			/// Register a new defined mouse movement callback
			/// </summary>
			/// <param name="userMouseMvmtCall">The callback function to register must be of type foo(double, double)</param>
			static void registerMouseMovementCallback(std::unique_ptr<std::function<void(double, double)>> newMouseMvmtCallback);

			/// <summary>
			/// Register a new defined mouse button callback
			/// </summary>
			/// <param name="newMouseButtonCallback"></param>
			static void registerMouseButtonCallback(std::unique_ptr<std::function<void(int, int, int)>> newMouseButtonCallback);

			/// <summary>
			/// Register a new defined mouse scroll callback
			/// </summary>
			/// <param name="newMouseScrollCallback"></param>
			static void registerMouseScrollCallback(std::unique_ptr <std::function<void(double, double)>> newMouseScrollCallback);

			/// <summary>
			/// Main callback point from glfw when a key interaction is registered by glfw. 
			/// </summary>
			/// <param name="window"></param>
			/// <param name="key"></param>
			/// <param name="scancode"></param>
			/// <param name="action"></param>
			/// <param name="mods"></param>
			static void glfwKeyHandle(GLFWwindow* window, int key, int scancode, int action, int mods);

			/// <summary>
			/// Main callback point from glfw when mouse movement is registered by glfw.
			/// </summary>
			/// <param name="window"></param>
			/// <param name="xpos"></param>
			/// <param name="ypos"></param>
			static void glfwMouseMovement(GLFWwindow* window, double xpos, double ypos);

			/// <summary>
			/// Main callback point from glfw when a mouse button interaction is registered by glfw.
			/// </summary>
			/// <param name="window"></param>
			/// <param name="button"></param>
			/// <param name="action"></param>
			/// <param name="mods"></param>
			static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

			/// <summary>
			/// Main callback point from glfw when a mouse scroll interaction is registered by glfw. 
			/// </summary>
			/// <param name="window"></param>
			/// <param name="xoffset"></param>
			/// <param name="yoffset"></param>
			static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);


		protected:
			//pointer to user defined keyboard callback function located in application program
			static std::vector<std::unique_ptr<std::function<void(int, int, int, int)>>> keyboardCallbacks;

			//pointer to user defined mouse movement callback
			static std::vector<std::unique_ptr<std::function<void(double, double)>>> mouseMovementCallbacks;

			//container for pointers to user defined mouse button callbacks
			static std::vector<std::unique_ptr<std::function<void(int, int, int)>>> mouseButtonCallbacks;

			//container for pointers to defined mouse scroll callbacks
			static std::vector<std::unique_ptr<std::function<void(double, double)>>> mouseScrollCallbacks;
		private:

		};
	}
}