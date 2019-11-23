// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "MenuStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/Level/GameMapLoader.h"
#include "Game/Game.h"
#include "SVE/SceneManager.h"

namespace Chewman
{

MenuStateProcessor::MenuStateProcessor()
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/startmenu.xml");
    _document->setMouseUpHandler(this);
    _document->hide();

    _gameMapProcessor = std::make_unique<GameMapProcessor>(Game::getInstance()->getGameMapLoader().loadMap("resources/game/levels/load.map", "load"));
    _gameMapProcessor->getGameMap()->player->setCameraFollow(false);
}

MenuStateProcessor::~MenuStateProcessor() = default;

GameState MenuStateProcessor::update(float deltaTime)
{
    _gameMapProcessor->update(deltaTime);
    return GameState::MainMenu;
}

void MenuStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());
}

void MenuStateProcessor::show()
{
    _gameMapProcessor->setVisible(true);
    auto camera = SVE::Engine::getInstance()->getSceneManager()->getMainCamera();

    auto windowSize = SVE::Engine::getInstance()->getRenderWindowSize();

    if ((float)windowSize.x / windowSize.y < 1.4)
        camera->setPosition({3.15083, 12.5414, 9.36191});  // 3:4
    else
        camera->setPosition({5.48245, 8.69662, 4.26211}); // wide

    camera->setYawPitchRoll({-0.411897, -0.614356, 0});

    // -1.36845 20.7681 19.9432

    _document->show();
}

void MenuStateProcessor::hide()
{
    _document->hide();
    _gameMapProcessor->setVisible(false);
}

bool MenuStateProcessor::isOverlapping()
{
    return false;
}

void MenuStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "start")
        {
            _document->hide();

            auto& progressManager = Game::getInstance()->getProgressManager();
            progressManager.setCurrentLevel(1);
            progressManager.setVictory(false);
            progressManager.setStarted(false);
            progressManager.resetPlayerInfo();
            Game::getInstance()->setState(GameState::WorldSelection);
        }
        if (control->getName() == "config")
        {
            Game::getInstance()->setState(GameState::Graphics);
        }
        if (control->getName() == "score")
        {
            _document->hide();
            Game::getInstance()->setState(GameState::Highscores);
        }
    }
}

} // namespace Chewman