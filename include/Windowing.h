#pragma once 
#define GLFW_INCLUDE_VULKAN

#include "SC/Application.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <memory> 

namespace star{
    namespace core{
        class Windowing{
        public: 
            GLFWwindow* glfwWindow;

            Windowing(int width, int height, GLFWkeyfun keyboardCallbackFunction); 

            bool shouldCloseWindow(); 

            void pollWindowEvents();

            const char** getRequiredExtensions(uint32_t& extensionCount); 

            //TODO: renderer needs to handle this 
            // static void ResizeWindowCallback(GLFWwindow* window, int width, int height)

        protected: 

        private:  
        }; 
    }
}