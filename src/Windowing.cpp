#include "Windowing.h"

star::core::Windowing::Windowing(int width, int height, GLFWkeyfun keyboardCallbackFunction){
    //actually make sure to init glfw
    glfwInit();
    //tell GLFW to create a window but to not include a openGL instance as this is a default behavior
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //disable resizing functionality in glfw as this will not be handled in the first tutorial
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //create a window, 3rd argument allows selection of monitor, 4th argument only applies to openGL
    this->window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

    //need to give GLFW a pointer to current instance of this class
    glfwSetWindowUserPointer(this->window, this);

    // glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    //set keyboard callbacks
    auto callback = glfwSetKeyCallback(this->window, keyboardCallbackFunction);
}

bool star::core::Windowing::ShouldCloseWindow(){
    return glfwWindowShouldClose(this->window); 
}

void star::core::Windowing::PollWindowEvents(){
    glfwPollEvents(); 
}

// static void star::core::Windowing::ResizeWindowCallback(GLFWwindow* window, int width, int height){
//         //retreive the pointer to the instance of the class that was created in initWindow() -> glfwSetWindowUserPointer
//         auto app = reinterpret_cast<HelloSquareApplication*>(glfwGetWindowUserPointer(window));
//         app->frameBufferResized = true;
            
// }