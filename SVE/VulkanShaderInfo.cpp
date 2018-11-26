// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "VulkanShaderInfo.h"
#include "VulkanException.h"
#include "VulkanInstance.h"
#include "Engine.h"
#include "LightManager.h"
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
                    VK_SHADER_STAGE_GEOMETRY_BIT,
                    VK_SHADER_STAGE_COMPUTE_BIT
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
        } else if (info.uniformType == UniformType::LightPointViewProjectionList)
        {
            size += sizeMap.at(info.uniformType) * _shaderSettings.maxLightSize;
        } else if (info.uniformType == UniformType::LightDirectViewProjectionList)
        {
            size += sizeMap.at(info.uniformType) * _shaderSettings.maxCascadeLightSize;
        }
        else if (info.uniformType == UniformType::ViewProjectionMatrixList)
        {
            size += sizeMap.at(info.uniformType) * _shaderSettings.maxViewProjectionMatrices;
        }
        else
        {
            size += sizeMap.at(info.uniformType);
        }
    }

    return size;
}

size_t VulkanShaderInfo::getShaderStorageBuffersSize() const
{
    size_t size = 0;
    const auto& sizeMap = getStorageBufferSizeMap();
    for (const auto& type : _shaderSettings.bufferList)
    {
        size += sizeMap.at(type);
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
    uint32_t stride = 0;

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Position)
    {
        VkVertexInputBindingDescription positionBinding {};
        positionBinding.binding = binding++;
        positionBinding.stride = getVertexDataSize(VertexInfo::Position);
        positionBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex or per instance
        bindingDescriptions.push_back(positionBinding);
        stride += positionBinding.stride;
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Color)
    {
        VkVertexInputBindingDescription colorBinding {};
        colorBinding.binding = binding++;
        colorBinding.stride = getVertexDataSize(VertexInfo::Color);
        colorBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(colorBinding);
        stride += colorBinding.stride;
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::TexCoord)
    {
        VkVertexInputBindingDescription texCoordBinding {};
        texCoordBinding.binding = binding++;
        texCoordBinding.stride = getVertexDataSize(VertexInfo::TexCoord);
        texCoordBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(texCoordBinding);
        stride += texCoordBinding.stride;
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Normal)
    {
        VkVertexInputBindingDescription normalBinding {};
        normalBinding.binding = binding++;
        normalBinding.stride = getVertexDataSize(VertexInfo::Normal);
        normalBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(normalBinding);
        stride += normalBinding.stride;
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::BoneWeights)
    {
        VkVertexInputBindingDescription boneWeightBinding {};
        boneWeightBinding.binding = binding++;
        boneWeightBinding.stride = getVertexDataSize(VertexInfo::BoneWeights);
        boneWeightBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(boneWeightBinding);
        stride += boneWeightBinding.stride;
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::BoneIds)
    {
        VkVertexInputBindingDescription boneIdsBinding {};
        boneIdsBinding.binding = binding++;
        boneIdsBinding.stride = getVertexDataSize(VertexInfo::BoneIds);
        boneIdsBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(boneIdsBinding);
        stride += boneIdsBinding.stride;
    }
    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Custom)
    {
        for (auto i = 0; i < _shaderSettings.vertexInfo.customCount; i++)
        {
            VkVertexInputBindingDescription customBinding {};
            customBinding.binding = binding++;
            customBinding.stride = getVertexDataSize(VertexInfo::Custom);
            customBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescriptions.push_back(customBinding);
            stride += customBinding.stride;
        }
    }

    // for combined binding only one struct should be set
    if (!_shaderSettings.vertexInfo.separateBinding)
    {
        bindingDescriptions.clear();
        VkVertexInputBindingDescription combinedBindingDescription {};
        combinedBindingDescription.binding = 0;
        combinedBindingDescription.stride = stride;
        combinedBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions.push_back(combinedBindingDescription);
    }

    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VulkanShaderInfo::getAttributeDescriptions() const
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    uint32_t binding = 0;
    uint32_t location = 0;
    uint32_t offset = 0;

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Position)
    {
        VkVertexInputAttributeDescription vertexAttrib {};
        vertexAttrib.binding = _shaderSettings.vertexInfo.separateBinding ? binding++ : binding;
        vertexAttrib.location = location++;
        vertexAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions.push_back(vertexAttrib);

        offset += getVertexDataSize(VertexInfo::Position);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Color)
    {
        VkVertexInputAttributeDescription colorAttrib {};
        colorAttrib.binding = _shaderSettings.vertexInfo.separateBinding ? binding++ : binding;
        colorAttrib.location = location++;
        colorAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        colorAttrib.offset = _shaderSettings.vertexInfo.separateBinding ? 0 : offset;
        attributeDescriptions.push_back(colorAttrib);

        offset += getVertexDataSize(VertexInfo::Color);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::TexCoord)
    {
        VkVertexInputAttributeDescription texCoordAttrib {};
        texCoordAttrib.binding = _shaderSettings.vertexInfo.separateBinding ? binding++ : binding;
        texCoordAttrib.location = location++;
        texCoordAttrib.format = VK_FORMAT_R32G32_SFLOAT;
        texCoordAttrib.offset = _shaderSettings.vertexInfo.separateBinding ? 0 : offset;
        attributeDescriptions.push_back(texCoordAttrib);

        offset += getVertexDataSize(VertexInfo::TexCoord);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Normal)
    {
        VkVertexInputAttributeDescription normalAttrib {};
        normalAttrib.binding = _shaderSettings.vertexInfo.separateBinding ? binding++ : binding;
        normalAttrib.location = location++;
        normalAttrib.format = VK_FORMAT_R32G32B32_SFLOAT;
        normalAttrib.offset = _shaderSettings.vertexInfo.separateBinding ? 0 : offset;
        attributeDescriptions.push_back(normalAttrib);

        offset += getVertexDataSize(VertexInfo::Normal);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::BoneWeights)
    {
        VkVertexInputAttributeDescription boneWeightAttrib {};
        boneWeightAttrib.binding = _shaderSettings.vertexInfo.separateBinding ? binding++ : binding;
        boneWeightAttrib.location = location++;
        boneWeightAttrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        boneWeightAttrib.offset = _shaderSettings.vertexInfo.separateBinding ? 0 : offset;
        attributeDescriptions.push_back(boneWeightAttrib);

        offset += getVertexDataSize(VertexInfo::BoneWeights);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::BoneIds)
    {
        VkVertexInputAttributeDescription boneIndexAttrib {};
        boneIndexAttrib.binding = _shaderSettings.vertexInfo.separateBinding ? binding++ : binding;
        boneIndexAttrib.location = location++;
        boneIndexAttrib.format = VK_FORMAT_R32G32B32A32_SINT;
        boneIndexAttrib.offset = _shaderSettings.vertexInfo.separateBinding ? 0 : offset;
        attributeDescriptions.push_back(boneIndexAttrib);

        offset += getVertexDataSize(VertexInfo::BoneIds);
    }

    if (_shaderSettings.vertexInfo.vertexDataFlags & VertexInfo::Custom)
    {
        for (auto i = 0; i < _shaderSettings.vertexInfo.customCount; i++)
        {
            VkVertexInputAttributeDescription customAttribute {};
            customAttribute.binding = _shaderSettings.vertexInfo.separateBinding ? binding++ : binding;
            customAttribute.location = location++;
            customAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            customAttribute.offset = _shaderSettings.vertexInfo.separateBinding ? 0 : offset;
            attributeDescriptions.push_back(customAttribute);

            offset += getVertexDataSize(VertexInfo::Custom);
        }
    }

    return attributeDescriptions;
}

void VulkanShaderInfo::createDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> descriptorList;
    uint32_t bindingNum = 0;

    if (_shaderSettings.shaderType == ShaderType::ComputeShader)
    {
        VkDescriptorSetLayoutBinding texelStorageLayoutBinding {};
        texelStorageLayoutBinding.binding = bindingNum;
        texelStorageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        texelStorageLayoutBinding.descriptorCount = 1;
        texelStorageLayoutBinding.stageFlags = _shaderStage;
        texelStorageLayoutBinding.pImmutableSamplers = nullptr;

        descriptorList.push_back(texelStorageLayoutBinding);
        bindingNum++;
    }

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
        bindingNum++;
    }

    if (!_shaderSettings.bufferList.empty())
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = bindingNum; // binding in shader
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = _shaderStage;
        uboLayoutBinding.pImmutableSamplers = nullptr; // used for image sampling

        descriptorList.push_back(uboLayoutBinding);
    }

    if (descriptorList.empty())
    {
        _descriptorSetLayout = VK_NULL_HANDLE;
        return;
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
    _descriptorSetLayout = VK_NULL_HANDLE;
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

uint32_t VulkanShaderInfo::getVertexDataSize(VertexInfo::VertexDataType vertexDataType) const
{
    switch (vertexDataType)
    {
        case VertexInfo::Position:
            return sizeof(float) * _shaderSettings.vertexInfo.positionSize;
        case VertexInfo::Color:
            return sizeof(float) * _shaderSettings.vertexInfo.colorSize;
        case VertexInfo::TexCoord:
            return sizeof(glm::vec2);
        case VertexInfo::Normal:
            return sizeof(glm::vec3);
        case VertexInfo::BoneWeights:
            return sizeof(glm::vec4);
        case VertexInfo::BoneIds:
            return sizeof(glm::ivec4);
        case VertexInfo::Custom:
            return sizeof(glm::vec4);
    }

    throw VulkanException("Unsupported vertex type");
}

} // namespace SVE