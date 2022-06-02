#pragma once 
#include "SC/Application.hpp"

#include <GLFW/glfw3.h>

#include <memory> 

namespace star{
    namespace core{
        class Windowing{
        public: 
            Windowing(int width, int height, GLFWkeyfun keyboardCallbackFunction); 

            bool ShouldCloseWindow(); 

            void PollWindowEvents();

            //TODO: renderer needs to handle this 
            // static void ResizeWindowCallback(GLFWwindow* window, int width, int height)

        protected: 

        private: 
            GLFWwindow* window; 
        }; 
    }
}