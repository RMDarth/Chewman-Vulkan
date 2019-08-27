// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "LightNode.h"
#include "ShaderSettings.h"

#include "Engine.h"
#include "SceneManager.h"
#include "LightManager.h"

#include <glm/gtc/matrix_transform.hpp>

namespace SVE
{

LightNode::LightNode(LightSettings lightSettings, uint32_t lightIndex)
    : _lightIndex(lightIndex)
    , _lightSettings(std::move(lightSettings))
{
    createProjectionMatrix();
}

LightSettings& LightNode::getLightSettings()
{
    return _lightSettings;
}

void LightNode::updateViewMatrix(glm::vec3 cameraPos)
{
    if (_lightSettings.lightType == LightType::SunLight)
        _viewMatrix = glm::lookAt(_originalPos + cameraPos, glm::vec3(0, 0, 0) + cameraPos, glm::vec3(0.0f, 1.0f, 0.0f));
}

void LightNode::fillUniformData(UniformData& data, uint32_t lightNum, bool asViewSource)
{
    //updateViewMatrix(data.cameraPos);

    if (!asViewSource)
    {
        auto model = getTotalTransformation();
        switch (_lightSettings.lightType)
        {
            case LightType::ShadowPointLight:
            {
                data.lightPointViewProjectionList[lightNum] = _projectionMatrix * _viewMatrix;

                PointLight pointLight{};
                pointLight.diffuse = glm::vec4(_lightSettings.diffuseStrength);
                pointLight.specular = glm::vec4(_lightSettings.specularStrength);
                pointLight.ambient = glm::vec4(_lightSettings.ambientStrength);
                pointLight.position = model[3];

                pointLight.constant = _lightSettings.constAtten;
                pointLight.linear = _lightSettings.linearAtten;
                pointLight.quadratic = _lightSettings.quadAtten;

                data.shadowPointLightList.push_back(pointLight);
                data.lightInfo.lightFlags |= (LightInfo::PointLight1 << (data.shadowPointLightList.size() - 1));
                if (_lightSettings.castShadows)
                    data.lightInfo.lightShadowFlags |= (LightInfo::PointLight1 << (data.shadowPointLightList.size() - 1));

                break;
            }
            case LightType::PointLight:
            {
                PointLight pointLight{};
                pointLight.diffuse = glm::vec4(_lightSettings.diffuseStrength);
                pointLight.specular = glm::vec4(_lightSettings.specularStrength);
                pointLight.ambient = glm::vec4(_lightSettings.ambientStrength);
                pointLight.position = model[3];

                pointLight.constant = _lightSettings.constAtten;
                pointLight.linear = _lightSettings.linearAtten;
                pointLight.quadratic = _lightSettings.quadAtten;

                data.pointLightList.push_back(pointLight);
                data.lightInfo.pointLightNum = data.pointLightList.size();
            }
            case LightType::SunLight:
            {
                data.lightDirectViewProjectionList.clear();
                for (auto &projectionMatrix : _projectionList)
                {
                    data.lightDirectViewProjectionList.push_back(projectionMatrix * _viewMatrix);
                }

                data.dirLight.diffuse = glm::vec4(_lightSettings.diffuseStrength);
                data.dirLight.specular = glm::vec4(_lightSettings.specularStrength);
                data.dirLight.ambient = glm::vec4(_lightSettings.ambientStrength);
                data.dirLight.direction = glm::vec4(-glm::normalize(model[3]));

                data.lightInfo.lightFlags |= LightInfo::DirectionalLight;
                if (_lightSettings.castShadows)
                    data.lightInfo.lightShadowFlags |= LightInfo::DirectionalLight;
                break;
            }
            case LightType::SpotLight:
                // TODO: Add spotlight
                data.lightPointViewProjectionList[lightNum] = _projectionMatrix * _viewMatrix;
                data.spotLight.diffuse = glm::vec4(_lightSettings.diffuseStrength);
                data.spotLight.specular = glm::vec4(_lightSettings.specularStrength);
                data.spotLight.ambient = glm::vec4(_lightSettings.ambientStrength);
                //data.spotLight.direction = glm::vec4(-glm::normalize(model[3]));

                data.lightInfo.lightFlags |= LightInfo::SpotLight;
                if (_lightSettings.castShadows)
                    data.lightInfo.lightShadowFlags |= LightInfo::SpotLight;
                break;
            case LightType::LineLight:
            {
                LineLight lineLight{};
                lineLight.diffuse = glm::vec4(_lightSettings.diffuseStrength);
                lineLight.specular = glm::vec4(_lightSettings.specularStrength);
                lineLight.ambient = glm::vec4(_lightSettings.ambientStrength);
                lineLight.startPosition = model[3];
                lineLight.endPosition = glm::vec4(_lightSettings.secondPoint, 1.0f);

                lineLight.constant = _lightSettings.constAtten;
                lineLight.linear = _lightSettings.linearAtten;
                lineLight.quadratic = _lightSettings.quadAtten;

                data.lineLightList.push_back(lineLight);
                data.lightInfo.lightLineNum = data.lineLightList.size();
                break;
            }
            case LightType::RectLight:
                break;
        }
    }
    else // as viewsource
    {
        data.view = _viewMatrix;
        data.projection = _projectionMatrix;

        //data.viewProjectionList.clear();
        assert(_projectionList.empty() || _viewList.empty());
        for (auto& projectionMatrix : _projectionList)
        {
            data.viewProjectionList.push_back(projectionMatrix * _viewMatrix);
        }
        for (auto& viewMatrix : _viewList)
        {
            data.viewProjectionList.push_back(_projectionMatrix * viewMatrix);
        }
    }
}

bool LightNode::castShadows() const
{
    return _lightSettings.castShadows;
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

    if (_lightSettings.lightType == LightType::SunLight)
        _viewMatrix = glm::lookAt(glm::vec3(model[3]), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
    else
    {
        _viewList.clear();
        _viewList.push_back(glm::lookAt(_originalPos, _originalPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3( 0.0, 1.0, 0.0)));
        _viewList.push_back(glm::lookAt(_originalPos, _originalPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3( 0.0, 1.0, 0.0)));
        _viewList.push_back(glm::lookAt(_originalPos, _originalPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        _viewList.push_back(glm::lookAt(_originalPos, _originalPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        _viewList.push_back(glm::lookAt(_originalPos, _originalPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3( 0.0, 1.0, 0.0)));
        _viewList.push_back(glm::lookAt(_originalPos, _originalPos + glm::vec3( 0.0, 0.0,1.0), glm::vec3( 0.0, 1.0, 0.0)));
    }
}

void LightNode::createProjectionMatrix()
{
    switch (_lightSettings.lightType)
    {
        case LightType::ShadowPointLight:
        {
            _projectionMatrix = glm::perspective(glm::radians(90.0f),
                                                 1.0f,
                                                 0.1f,
                                                 100.0f);

            _projectionMatrix[1][1] *= -1;
            break;
        }
        case LightType::SunLight:
        {
            _projectionMatrix = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.01f, 50.0f);
            _projectionMatrix[1][1] *= -1;

            // TODO: Get far/near view distance from camera
            float far = 500;
            float near = 1;
            _projectionList.clear();
            for (auto i = 0u; i < MAX_CASCADES; i++)
            {
                float distance = near * powf(far / near, (float)(i + 1) / MAX_CASCADES);
                auto projectionMatrix = glm::ortho(-distance, distance, -distance, distance, 0.01f, distance*2 + 30);
                projectionMatrix[1][1] *= -1;
                _projectionList.push_back(projectionMatrix);
            }

            break;
        }
        case LightType::SpotLight:
            _projectionMatrix =  glm::perspective(glm::radians(80.0f),
                                                  1.0f,
                                                  0.01f,
                                                  20.0f);
            _projectionMatrix[1][1] *= -1;
            break;
        case LightType::LineLight:
        case LightType::RectLight:
        case LightType::PointLight:
            break;
    }

}

void LightNode::setNodeTransformation(glm::mat4 transform)
{
    SceneNode::setNodeTransformation(transform);
    createViewMatrix();
}


} // namespace SVE