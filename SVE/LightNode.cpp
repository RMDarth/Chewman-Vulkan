// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "LightNode.h"
#include "ShaderSettings.h"

#include "Engine.h"
#include "SceneManager.h"
#include "LightManager.h"
#include "ShadowMap.h"

#include <glm/gtc/matrix_transform.hpp>

namespace SVE
{

LightNode::LightNode(LightSettings lightSettings, uint32_t lightIndex)
    : _lightIndex(lightIndex)
    , _lightSettings(std::move(lightSettings))
{
    createProjectionMatrix();
    if (_lightSettings.castShadows)
    {
        _shadowmap = std::make_shared<ShadowMap>(_lightIndex);
        _shadowmap->enableShadowMap();
    }
}

const LightSettings& LightNode::getLightSettings()
{
    return _lightSettings;
}

void LightNode::updateViewMatrix(glm::vec3 cameraPos)
{
    _viewMatrix = glm::lookAt(_originalPos + cameraPos, glm::vec3(0, 0, 0) + cameraPos, glm::vec3(0.0f, 1.0f, 0.0f));
}

void LightNode::fillUniformData(UniformData& data, uint32_t lightNum, bool asViewSource)
{
    //updateViewMatrix(data.cameraPos);

    auto model = getTotalTransformation();

    switch (_lightSettings.lightType)
    {
        case LightType::PointLight:
        {
            data.lightViewProjectionList[lightNum] = _projectionMatrix * _viewMatrix;

            PointLight pointLight{};
            pointLight.diffuse = glm::vec4(_lightSettings.diffuseStrength);
            pointLight.specular = glm::vec4(_lightSettings.specularStrength);
            pointLight.ambient = glm::vec4(_lightSettings.ambientStrength);
            pointLight.position = model[3];

            // TODO: Move light attenuation to light settings
            pointLight.constant = 1.0f * 0.05f;
            pointLight.linear = 1.35f * 0.05f;
            pointLight.quadratic = 0.44f * 0.05f;

            data.pointLightList.push_back(pointLight);
            data.lightInfo.lightFlags |= (LightInfo::PointLight1 << (data.pointLightList.size() - 1));

            break;
        }
        case LightType::SunLight:
        {
            // TODO: Probably this data should be for each shadow (if multiple shadowmaps are used)
            data.lightViewProjectionList[lightNum] = _projectionMatrix * _viewMatrix;

            data.dirLight.diffuse = glm::vec4(_lightSettings.diffuseStrength);
            data.dirLight.specular = glm::vec4(_lightSettings.specularStrength);
            data.dirLight.ambient = glm::vec4(_lightSettings.ambientStrength);
            data.dirLight.direction = glm::vec4(-glm::normalize(model[3]));

            data.lightInfo.lightFlags |= LightInfo::DirectionalLight;
            break;
        }
        case LightType::SpotLight:
            // TODO: Add spotlight
            data.lightViewProjectionList[lightNum] = _projectionMatrix * _viewMatrix;
            data.spotLight.diffuse = glm::vec4(_lightSettings.diffuseStrength);
            data.spotLight.specular = glm::vec4(_lightSettings.specularStrength);
            data.spotLight.ambient = glm::vec4(_lightSettings.ambientStrength);
            //data.spotLight.direction = glm::vec4(-glm::normalize(model[3]));

            data.lightInfo.lightFlags |= LightInfo::SpotLight;
            break;
        case LightType::RectLight:
            break;
    }


    if (asViewSource)
    {
        data.view = _viewMatrix;
        data.projection = _projectionMatrix;
    }
}

std::shared_ptr<ShadowMap> LightNode::getShadowMap()
{
    return _shadowmap;
}

const glm::mat4& LightNode::getViewMatrix()
{
    return _viewMatrix;
}

const glm::mat4& LightNode::getProjectionMatrix()
{
    return _projectionMatrix;
}

void LightNode::createViewMatrix()
{
    auto model = getTotalTransformation();
    _originalPos = glm::vec3(model[3]);
    _viewMatrix = glm::lookAt(glm::vec3(model[3]), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
}

void LightNode::createProjectionMatrix()
{
    switch (_lightSettings.lightType)
    {
        case LightType::PointLight:
            _projectionMatrix =  glm::perspective(glm::radians(80.0f),
                                                  1.0f,
                                                  0.01f,
                                                  100.0f);
            _projectionMatrix[1][1] *= -1;
            break;
        case LightType::SunLight:
            _projectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.01f, 50.0f);
            _projectionMatrix[1][1] *= -1;
            break;
        case LightType::SpotLight:
            _projectionMatrix =  glm::perspective(glm::radians(80.0f),
                                                  1.0f,
                                                  0.01f,
                                                  100.0f);
            _projectionMatrix[1][1] *= -1;
            break;
        case LightType::RectLight:
            break;
    }

}

void LightNode::setNodeTransformation(glm::mat4 transform)
{
    SceneNode::setNodeTransformation(transform);
    createViewMatrix();
}


} // namespace SVE