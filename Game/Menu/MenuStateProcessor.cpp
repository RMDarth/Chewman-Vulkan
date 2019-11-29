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
    updateConfigSlider();

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

    _document->show();
    setConfigSliderVisibility(false);
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
    //setConfigSliderVisibility(true);
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
            setConfigSliderVisibility(!_configPanelVisible);
        }
        if (control->getName() == "settings")
        {
            _document->hide();
            Game::getInstance()->setState(GameState::Graphics);
        }
        if (control->getName() == "info")
        {
            _document->hide();
            Game::getInstance()->setState(GameState::Credits);
        }
        if (control->getName() == "score")
        {
            _document->hide();
            Game::getInstance()->setState(GameState::Highscores);
        }
    }
}

void MenuStateProcessor::updateConfigSlider()
{
    _configPanel = _document->getControlByName("configpanel");
    auto pos = _configPanel->getPosition();
    auto size = _configPanel->getSize();
    pos.y = pos.y - (size.x  * 4.0f) + (size.x  * 0.5f) ;
    _configPanel->setPosition(pos);
    _configPanel->setSize({size.x, size.x * 4.0f});
    _configPanel->setRenderOrder(50);

    pos.y = pos.y + (int)(size.x * 0.3f);
    pos.x = pos.x + (int)(size.x * 0.05f);
    auto settingsButton = _document->getControlByName("settings");
    settingsButton->setPosition(pos);
    auto buttonSize = settingsButton->getSize().x;

    pos.y = pos.y + (int)(buttonSize * 0.9f);
    auto soundButton = _document->getControlByName("sound");
    soundButton->setPosition(pos);

    pos.y = pos.y + (int)(buttonSize * 0.9f);
    auto musicButton = _document->getControlByName("music");
    musicButton->setPosition(pos);

    pos.y = pos.y + (int)(buttonSize * 0.9f);
    auto infoButton = _document->getControlByName("info");
    infoButton->setPosition(pos);
}

void MenuStateProcessor::setConfigSliderVisibility(bool visible)
{
    _configPanelVisible = visible;
    _configPanel->setVisible(visible);
    _document->getControlByName("settings")->setVisible(visible);
    _document->getControlByName("sound")->setVisible(visible);
    _document->getControlByName("music")->setVisible(visible);
    _document->getControlByName("info")->setVisible(visible);
}

} // namespace Chewman