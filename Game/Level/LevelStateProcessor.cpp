// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <sstream>
#include <iomanip>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include "LevelStateProcessor.h"
#include "Game/Game.h"
#include "Game/Controls/ControlDocument.h"
#include "SVE/SceneManager.h"
#include "SVE/LightManager.h"

namespace Chewman
{

LevelStateProcessor::LevelStateProcessor()
    : _progressManager(Game::getInstance()->getProgressManager())
{
    _document = std::make_unique<ControlDocument>("resources/game/GUI/HUD.xml");
    _document->setMouseUpHandler(this);
    _document->hide();

    _loadingFinished = false;
}

LevelStateProcessor::~LevelStateProcessor()
{
}

void LevelStateProcessor::initMap()
{
    auto levelNum = _progressManager.getCurrentLevel();

    if (_gameMapProcessor)
    {
        _oldGameMap = std::move(_gameMapProcessor);
        _oldGameMap->setVisible(false);
        _countToRemove = 5;
    }

    std::stringstream ss;
    ss << "resources/game/levels/level" << levelNum << ".map";
    // TODO: Display some "Loading" message box while level is loading
    auto future = std::async(std::launch::async, [&]
    {
        _gameMapProcessor = std::make_unique<Chewman::GameMapProcessor>(_mapLoader.loadMap(ss.str()));
        _loadingFinished = true;
        _gameMapProcessor->setVisible(true);
    });
}

GameState LevelStateProcessor::update(float deltaTime)
{
    if (_loadingFinished == false)
        return GameState::Level;

    _gameMapProcessor->update(deltaTime);
    _time += deltaTime;
    updateHUD(deltaTime);

    switch (_gameMapProcessor->getState())
    {
        case GameMapState::Game:
            break;
        case GameMapState::Pause:
            break;
        case GameMapState::Animation:
            break;
        case GameMapState::Victory:
            // TODO: Display victory menu
            _progressManager.setCurrentLevel(_progressManager.getCurrentLevel() + 1);
            _progressManager.setVictory(true);
            _progressManager.setStarted(false);
            _progressManager.getPlayerInfo().time = (int)_time;
            return GameState::Score;
        case GameMapState::GameOver:
            _progressManager.setCurrentLevel(1);
            _progressManager.setVictory(false);
            _progressManager.setStarted(false);
            _progressManager.getPlayerInfo().time = (int)_time;
            return GameState::Score;
    }

    if (_countToRemove > 0)
    {
        --_countToRemove;
        if (!_countToRemove)
        {
            auto future = std::async(std::launch::async, [&]
            {
                _oldGameMap.reset();
            });
        }
    }

    return GameState::Level;
}

void LevelStateProcessor::processInput(const SDL_Event& event)
{
    if (!_loadingFinished)
        return;

    _gameMapProcessor->processInput(event);
    processDocument(event, _document.get());
}

void LevelStateProcessor::show()
{
    if (!_progressManager.isStarted())
    {
        _progressManager.setStarted(true);
        _progressManager.setVictory(false);
        _loadingFinished = false;
        initMap();
        _time = 0.0f;
    } else {
        _gameMapProcessor->setState(GameMapState::Game);
        _gameMapProcessor->setVisible(true);
    }

    _document->show();
}

void LevelStateProcessor::hide()
{
    _gameMapProcessor->setVisible(false);
    _document->hide();
}

bool LevelStateProcessor::isOverlapping()
{
    return false;
}

void LevelStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "pause")
        {
            _gameMapProcessor->setState(GameMapState::Pause);
            Game::getInstance()->setState(GameState::Pause);
        }
        if (control->getName() == "lifeimg")
        {
            static bool isNight = false;
            auto sunLight = SVE::Engine::getInstance()->getSceneManager()->getLightManager()->getDirectionLight();
            if (isNight)
            {
                sunLight->getLightSettings().ambientStrength = {0.2f, 0.2f, 0.2f, 1.0f};
                sunLight->getLightSettings().diffuseStrength = {1.0f, 1.0f, 1.0f, 1.0f};
                sunLight->getLightSettings().specularStrength = {0.5f, 0.5f, 0.5f, 1.0f};
                sunLight->setNodeTransformation(
                        glm::translate(glm::mat4(1), glm::vec3(80, 80, -80)));
                isNight = false;
            } else {
                isNight = true;
                sunLight->getLightSettings().ambientStrength = {0.08f, 0.08f, 0.08f, 1.0f};
                sunLight->getLightSettings().diffuseStrength = {0.15f, 0.15f, 0.15f, 1.0f};
                sunLight->getLightSettings().specularStrength = {0.08f, 0.08f, 0.08f, 1.0f};
                sunLight->setNodeTransformation(
                        glm::translate(glm::mat4(1), glm::vec3(-20, 80, 80)));
            }
        }
    }
}

void LevelStateProcessor::updateHUD(float deltaTime)
{
    std::stringstream stream;
    stream << (int)_time / 60 << ":" << std::setfill('0') << std::setw(2) << (int)_time % 60;
    static auto timeControl = _document->getControlByName("time");
    timeControl->setText(stream.str());

    static auto scoreControl = _document->getControlByName("score");
    scoreControl->setText(std::to_string(_progressManager.getPlayerInfo().points));

    static auto livesControl = _document->getControlByName("lifecount");
    livesControl->setText(std::to_string(_progressManager.getPlayerInfo().lives));

    static auto coins = _document->getControlByName("Coins");
    stream.str("");
    stream << _gameMapProcessor->getGameMap()->activeCoins << "/" << _gameMapProcessor->getGameMap()->totalCoins;
    coins->setText(stream.str());

    static auto fps = _document->getControlByName("FPS");
    static std::list<float> fpsList;
    fpsList.push_back(1.0f/deltaTime);
    if (fpsList.size() > 100)
        fpsList.pop_front();
    auto fpsValue = std::accumulate(fpsList.begin(), fpsList.end(), 0) / fpsList.size();
    fps->setText(std::to_string((int) fpsValue));

}

} // namespace Chewman