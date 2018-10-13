// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "VulkanShaderInfo.h"
#include "VulkanException.h"
#include "VulkanInstance.h"
#include "Engine.h"
#include <fstream>

namespace SVE
{
namespace
{

std::vector<char> readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file)
    {
        throw VulkanException("Can't open file " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderStageFlagBits getVulkanShaderStage(const ShaderSettings& shaderSettings)
{
    static VkShaderStageFlagBits stageMap[] =
            {
                    VK_SHADER_STAGE_VERTEX_BIT,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    VK_SHADER_STAGE_GEOMETRY_BIT
            };

    return stageMap[static_cast<uint8_t>(shaderSettings.shaderType)];
}

} // anon namespace

VulkanShaderInfo::VulkanShaderInfo(ShaderSettings shaderSettings)
        : _shaderSettings(std::move(shaderSettings))
        , _device(Engine::getInstance()->getVulkanInstance()->getLogicalDevice())
        , _shaderStage(getVulkanShaderStage(_shaderSettings))
{
    createDescriptorSetLayout();
}

VulkanShaderInfo::~VulkanShaderInfo()
{
    deleteDescriptorSetLayout();
}


VkPipelineShaderStageCreateInfo VulkanShaderInfo::createShaderStage()
{
    auto shaderCode = readFile(_shaderSettings.filename);
    _shaderModule = createShaderModule(shaderCode);

    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = _shaderStage;
    shaderStageInfo.module = _shaderModule;
    shaderStageInfo.pName = _shaderSettings.entryPoint.c_str();

    return shaderStageInfo;
}

void VulkanShaderInfo::freeShaderModule()
{
    vkDestroyShaderModule(_device, _shaderModule, nullptr);
}

size_t VulkanShaderInfo::getShaderUniformsSize() const
{
    size_t size = 0;
    const auto& sizeMap = getUniformSizeMap();
    for (const auto& info : _shaderSettings.uniformList)
    {
        if (info.uniformType == UniformType::BoneMatrices)
        {
            size += sizeMap.at(info.uniformType) * _shaderSettings.maxBonesSize;
        } else if (info.uniformType == UniformType::LightPoint)
        {
            size += sizeMap.at(info.uniformType) * _shaderSettings.maxPointLightSize;
        } else
        {
            size += sizeMap.at(info.uniformType);
        }
    }

    return size;
}

const ShaderSettings& VulkanShaderInfo::getShaderSettings() const
{
    return _shaderSettings;
}

VkDescriptorSetLayout VulkanShaderInfo::getDescriptorSetLayout() const
{
    return _descriptorSetLayout;
}

std::vector<VkVertexInputBindingDescription> VulkanShaderInfo::getBindingDescription() const
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    uint32_t binding = 0;

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Position)
    {
        VkVertexInputBindingDescription positionBinding {};
        positionBinding.binding = binding++;
        positionBinding.stride = sizeof(glm::vec3);
        positionBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex or per instance
        bindingDescriptions.push_back(positionBinding);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Color)
    {
        VkVertexInputBindingDescription colorBinding {};
        colorBinding.binding = binding++;
        colorBinding.stride = sizeof(glm::vec3);
        colorBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(colorBinding);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::TexCoord)
    {
        VkVertexInputBindingDescription texCoordBinding {};
        texCoordBinding.binding = binding++;
        texCoordBinding.stride = sizeof(glm::vec2);
        texCoordBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(texCoordBinding);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Normal)
    {
        VkVertexInputBindingDescription normalBinding {};
        normalBinding.binding = binding++;
        normalBinding.stride = sizeof(glm::vec3);
        normalBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(normalBinding);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::BoneWeights)
    {
        VkVertexInputBindingDescription boneWeightBinding {};
        boneWeightBinding.binding = binding++;
        boneWeightBinding.stride = sizeof(glm::vec4);
        boneWeightBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(boneWeightBinding);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::BoneIds)
    {
        VkVertexInputBindingDescription boneIdsBinding {};
        boneIdsBinding.binding = binding++;
        boneIdsBinding.stride = sizeof(glm::ivec4);
        boneIdsBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(boneIdsBinding);
    }

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VulkanShaderInfo::getAttributeDescriptions() const
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    uint32_t bindingAndLoc = 0;

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Position)
    {
        VkVertexInputAttributeDescription vertexAttrib {};
        vertexAttrib.binding = bindingAndLoc;
        vertexAttrib.location = bindingAndLoc++;
        vertexAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions.push_back(vertexAttrib);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Color)
    {
        VkVertexInputAttributeDescription colorAttrib {};
        colorAttrib.binding = bindingAndLoc;
        colorAttrib.location = bindingAndLoc++;
        colorAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions.push_back(colorAttrib);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::TexCoord)
    {
        VkVertexInputAttributeDescription texCoordAttrib {};
        texCoordAttrib.binding = bindingAndLoc;
        texCoordAttrib.location = bindingAndLoc++;
        texCoordAttrib.format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions.push_back(texCoordAttrib);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Normal)
    {
        VkVertexInputAttributeDescription normalAttrib {};
        normalAttrib.binding = bindingAndLoc;
        normalAttrib.location = bindingAndLoc++;
        normalAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions.push_back(normalAttrib);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::BoneWeights)
    {
        VkVertexInputAttributeDescription boneWeightAttrib {};
        boneWeightAttrib.binding = bindingAndLoc;
        boneWeightAttrib.location = bindingAndLoc++;
        boneWeightAttrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions.push_back(boneWeightAttrib);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::BoneIds)
    {
        VkVertexInputAttributeDescription boneIndexAttrib {};
        boneIndexAttrib.binding = bindingAndLoc;
        boneIndexAttrib.location = bindingAndLoc++;
        boneIndexAttrib.format = VK_FORMAT_R32G32B32A32_SINT;
        attributeDescriptions.push_back(boneIndexAttrib);
    }

    return attributeDescriptions;
}

void VulkanShaderInfo::createDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> descriptorList;
    uint32_t bindingNum = 0;
    for (auto i = 0u; i < _shaderSettings.samplerNamesList.size(); i++)
    {
        VkDescriptorSetLayoutBinding samplerLayoutBinding {};
        samplerLayoutBinding.binding = bindingNum;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.stageFlags = _shaderStage;
        samplerLayoutBinding.pImmutableSamplers = nullptr;

        descriptorList.push_back(samplerLayoutBinding);
        bindingNum++;
    }

    if (!_shaderSettings.uniformList.empty())
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = bindingNum; // binding in shader
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = _shaderStage;
        uboLayoutBinding.pImmutableSamplers = nullptr; // used for image sampling

        descriptorList.push_back(uboLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = descriptorList.size();
    descriptorSetLayoutCreateInfo.pBindings = descriptorList.data();

    if (vkCreateDescriptorSetLayout(
            _device,
            &descriptorSetLayoutCreateInfo,
            nullptr,
            &_descriptorSetLayout) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Descriptor Set layout");
    }

}

void VulkanShaderInfo::deleteDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(_device,
                                 _descriptorSetLayout,
                                 nullptr);
}

VkShaderModule VulkanShaderInfo::createShaderModule(const std::vector<char> &code) const
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw VulkanException("Can't create Vulkan Shader module!");
    }

    return shaderModule;
}


} // namespace SVE