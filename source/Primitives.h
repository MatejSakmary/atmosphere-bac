#pragma once
/* Force alignment of glm data types to respect the alignment required
   by Vulkan NOTE: this does not cover nested data structures in that case
   I need to use alignas(16) prefix */
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
/* GLM uses OpenGL default of -1,1 for the perspective projection so
   I need to force it to use Vulkan default which is 0,1 */
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vulkan/vulkan.h>

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texCoord;

    // bool operator==(const Vertex& other) const 
    // {
    //     return pos == other.pos && texCoord == other.texCoord;
    // }

    static std::vector<VkVertexInputBindingDescription> getBindingDescription()
    {
        /* A vertex binding describes at which rate to load data from memory
           throughout the vertices, number of bytes between data etc...*/
        std::vector<VkVertexInputBindingDescription> bindingDescription{};
        bindingDescription.resize(1);
        /* Specify the index of the binding*/
        bindingDescription[0].binding = 0;
        /* Specify number of bytes from one entry to the next*/
        bindingDescription[0].stride = sizeof(Vertex);
        /* Move to the next data entry after each vertex
            -> alternative after each instance, but we are not using instanced rendering */
        bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
    {
        /* An attribute description describes how to extract a vertex attribute from
           a chunk of of vertex data originating from a binding description
            -> We have two attributes so we need two attribute description structs*/
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.resize(2);
        /* From which binding this comes -> set in binding description*/
        attributeDescriptions[0].binding = 0;
        /* Location references location directive of the input in the vertex shader */
        attributeDescriptions[0].location = 0;
        /* Describes the type of data for the attribute R32G32 means vec2
            -> Similarly R32 means float and R32G32B32 means vec3 etc... */
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        /* Specify the number of bytes since the start of the per vertex data to read
           from*/
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord);
        return attributeDescriptions;
    }
};

// namespace std 
// {
//     template<> struct hash<Vertex>
//     {
//         size_t operator() (Vertex const& vertex) const {
//             return ((hash<glm::vec3>()(vertex.pos) ^
//                     (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
//                     (hash<glm::vec2>()(vertex.texCoord) << 1);
//         }
//     };
// }