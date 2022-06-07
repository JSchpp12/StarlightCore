#include "Windowing.h"

star::core::Windowing::Windowing(int width, int height, GLFWkeyfun keyboardCallbackFunction){
    //actually make sure to init glfw
    glfwInit();
    //tell GLFW to create a window but to not include a openGL instance as this is a default behavior
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //disable resizing functionality in glfw as this will not be handled in the first tutorial
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //create a window, 3rd argument allows selection of monitor, 4th argument only applies to openGL
    this->glfwWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    //need to give GLFW a pointer to current instance of this class
    glfwSetWindowUserPointer(this->glfwWindow, this);

    // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    //set keyboard callbacks
    auto callback = glfwSetKeyCallback(this->glfwWindow, keyboardCallbackFunction);
}

bool star::core::Windowing::shouldCloseWindow(){
    return glfwWindowShouldClose(this->glfwWindow); 
}

void star::core::Windowing::pollWindowEvents(){
    glfwPollEvents(); 
}

const char** star::core::Windowing::getRequiredExtensions(uint32_t& extensionCount){
        //get required vulkan extensions from glfw
    return glfwGetRequiredInstanceExtensions(&extensionCount);
}

// static void star::core::Windowing::ResizeWindowCallback(GLFWwindow* window, int width, int height){
//         //retreive the pointer to the instance of the class that was created in initWindow() -> glfwSetWindowUserPointer
//         auto app = reinterpret_cast<HelloSquareApplication*>(glfwGetWindowUserPointer(window));
//         app->frameBufferResized = true;
            
// }