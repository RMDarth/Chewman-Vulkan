// VSE (Vulkan Simple Engine) Library
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under CC BY 4.0
#include "Nun.h"

#include <SVE/Engine.h>
#include <SVE/SceneManager.h>
#include <SVE/MeshEntity.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "RandomWalkerAI.h"

namespace Chewman
{

Nun::Nun(CellInfoMap& map, glm::ivec2 startPos)
    : Enemy(map, startPos)
{
    _ai = std::make_shared<RandomWalkerAI>(*_mapTraveller, 80);

    auto* engine = SVE::Engine::getInstance();

    auto realMapPos = _mapTraveller->getRealPosition();
    auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);

    _rootNode = engine->getSceneManager()->createSceneNode();
    auto transform = glm::translate(glm::mat4(1), position);
    //transform = glm::rotate(transform, glm::radians(90.0f) * static_cast<uint32_t>(_mapTraveller.getCurrentDirection()), glm::vec3(0.0f, 1.0f, 0.0f));

    _rootNode->setNodeTransformation(transform);
    // TODO: Attach to map root node
    engine->getSceneManager()->getRootNode()->attachSceneNode(_rootNode);

    auto nunMesh = std::make_shared<SVE::MeshEntity>("nun");
    nunMesh->setMaterial("NunMaterial");
    _rootNode->attachEntity(nunMesh);
}

void Nun::update()
{
    _ai->update();
    auto realMapPos = _mapTraveller->getRealPosition();
    auto position = glm::vec3(realMapPos.y, 0, -realMapPos.x);

    _rootNode->setNodeTransformation(glm::translate(glm::mat4(1), position));
}

} // namespace Chewman