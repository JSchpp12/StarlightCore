#pragma once 
#include "SC/Vertex.hpp"

#include <vulkan/vulkan.hpp>

#include <array> 
namespace star::core{
    struct VulkanVertex{
        common::Vertex vertex; 

        /// <summary>
        /// Generates VkVertexInputBindingDescription from vertex object. This describes at which rate to load data from memory throughout the verticies. 
        /// Such as: number of bytes between data entries or if should move the next data entry after each vertex or after each instance
        /// </summary>
        /// <returns></returns>
        static vk::VertexInputBindingDescription getBindingDescription() {
            vk::VertexInputBindingDescription bindingDescription{};

            //all vertex data is in one array, so only using one binding 
            bindingDescription.binding = 0; //specifies index of the binding in the array of bindings

            //number of bytes from one entry to the next
            bindingDescription.stride = sizeof(common::Vertex);

            //can have one of the following: 
                //1. VK_VERTEX_INPUT_RATE_VERTEX: move to the next data entry after each vertex
                //2. VK_VERTEX_INPUT_RATE_INSTANCE: move to the next data entry after each instance
            //not using instanced rendering so per-vertex data
            bindingDescription.inputRate = vk::VertexInputRate::eVertex; 

            return bindingDescription;
        }

        /// <summary>
        /// Generates attribute data for the verticies. VkVertexInputAttributeDescriptions describes to vulkan how to extract a vertex attribute froma chunk of 
        /// vertex data originating from a binding descritpion. For this program, there are 2: position and color. 
        /// </summary>
        /// <returns>Array containing attribute descriptions</returns>

        static std::array<vk::VertexInputAttributeDescription, 10> getAttributeDescriptions() {
            std::array<vk::VertexInputAttributeDescription, 10> attributeDescriptions{};

            /* Struct */
                //1. binding - which binding the per-vertex data comes in 
                //2. location - references the location directive of the input in the vertex shader 
                //3. format - describes type of data 
                    // common shader and formats used 
                    // float : VK_FORMAT_R32_SFLOAT 
                    // vec2  : VK_FORMAT_R32G32_SFLOAT
                    // vec3  : VK_FORMAT_R32G32B32_SFLOAT
                    // vec4  : VK_FORMAT_R32G32B32A32_SFLOAT
                        //more odd examples
                            // ivec2 : VK_FORMAT_R32G32_SINT -- 2 component vector of 32-bit signed integers
                            // uvec4 : VK_FORMAT_R32G32B32A32_UINT -- 4 component vector of 32-bit unsigned integers 
                            // double: VK_FORMAT_R64_SFLOAT -- double precision 64-bit float 
                //4. offset - specifies the number of bytes since the start of the per-vertex data to read from
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;

            //the binding is loading over vertex at a time and the position attribute is at an offset of 0 bytes from the beginning of the struct.
            //offset macro calculates this distance for us
            attributeDescriptions[0].offset = offsetof(common::Vertex, pos);

            attributeDescriptions[1].binding = 0; 
            attributeDescriptions[1].location = 1; 
            attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[1].offset = offsetof(common::Vertex, normal);

            attributeDescriptions[2].binding = 0; 
            attributeDescriptions[2].location = 2; 
            attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat; 
            attributeDescriptions[2].offset = offsetof(common::Vertex, color); 

            /*Binding for vertex texture coordinates*/
            attributeDescriptions[3].binding = 0; 
            attributeDescriptions[3].location = 3; 
            attributeDescriptions[3].format = vk::Format::eR32G32Sfloat; 
            attributeDescriptions[3].offset = offsetof(common::Vertex, texCoord); 

            attributeDescriptions[4].binding = 0;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[4].offset = offsetof(common::Vertex, aTangent);

            attributeDescriptions[5].binding = 0;
            attributeDescriptions[5].location = 5;
            attributeDescriptions[5].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[5].offset = offsetof(common::Vertex, aBitangent);

            //material bindings 
            attributeDescriptions[6].binding = 0;
            attributeDescriptions[6].location = 6;
            attributeDescriptions[6].format = vk::Format::eR32G32B32Sfloat; 
            attributeDescriptions[6].offset = offsetof(common::Vertex, matAmbient);

            attributeDescriptions[7].binding = 0;
            attributeDescriptions[7].location = 7;
            attributeDescriptions[7].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[7].offset = offsetof(common::Vertex, matDiffuse);

            attributeDescriptions[8].binding = 0;
            attributeDescriptions[8].location = 8;
            attributeDescriptions[8].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[8].offset = offsetof(common::Vertex, matSpecular);

            attributeDescriptions[9].binding = 0;
            attributeDescriptions[9].location = 9;
            attributeDescriptions[9].format = vk::Format::eR32Sfloat;
            attributeDescriptions[9].offset = offsetof(common::Vertex, matShininess);
            return attributeDescriptions;
        }
    };
}