// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <sstream>
#include <iomanip>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include "LevelStateProcessor.h"
#include "Game/Game.h"
#include "Game/Level/GameMapLoader.h"
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
    _counterControl = _document->getControlByName("Counter");
    _counterControl->setDefaultMaterial("counter1.png");
    _counterControl->setDefaultMaterial("counter2.png");
    _counterControl->setDefaultMaterial("counter3.png");

    _document->hide();

    _loadingFinished = false;
}

LevelStateProcessor::~LevelStateProcessor() = default;

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
    std::cout << "Start loading level " << levelNum << std::endl;

    auto future = std::async(std::launch::async, [&]
    {
        _gameMapProcessor = std::make_unique<GameMapProcessor>(Game::getInstance()->getGameMapLoader().loadMap(ss.str()));

        auto sunLight = SVE::Engine::getInstance()->getSceneManager()->getLightManager()->getDirectionLight();
        if (!_gameMapProcessor->getGameMap()->isNight)
        {
            sunLight->getLightSettings().ambientStrength = {0.2f, 0.2f, 0.2f, 1.0f};
            sunLight->getLightSettings().diffuseStrength = {1.0f, 1.0f, 1.0f, 1.0f};
            sunLight->getLightSettings().specularStrength = {0.5f, 0.5f, 0.5f, 1.0f};
            sunLight->setNodeTransformation(
                    glm::translate(glm::mat4(1), glm::vec3(80, 80, -80)));

            sunLight->getLightSettings().castShadows = Game::getInstance()->getGraphicsManager().getSettings().useShadows;
        } else {
            sunLight->getLightSettings().ambientStrength = {0.08f, 0.08f, 0.08f, 1.0f};
            sunLight->getLightSettings().diffuseStrength = {0.15f, 0.15f, 0.15f, 1.0f};
            sunLight->getLightSettings().specularStrength = {0.08f, 0.08f, 0.08f, 1.0f};
            sunLight->setNodeTransformation(
                    glm::translate(glm::mat4(1), glm::vec3(-20, 80, 80)));

            sunLight->getLightSettings().castShadows = false;
        }

        _loadingFinished = true;
        _gameMapProcessor->setVisible(true);


    });
}

GameState LevelStateProcessor::update(float deltaTime)
{
    if (_loadingFinished == false)
        return GameState::Level;

    if (deltaTime > 0.15)
        deltaTime = 0.15;

    _gameMapProcessor->update(deltaTime);
    updateHUD(deltaTime);

    switch (_gameMapProcessor->getState())
    {
        case GameMapState::Game:
            _time += deltaTime;
            if (_counterTime > 0)
            {
                _counterControl->setVisible(false);
                _counterTime = -1;
            }
            break;
        case GameMapState::Pause:
            break;
        case GameMapState::Animation:
            break;
        case GameMapState::Victory:
            // TODO: Display victory menu
            _progressManager.setVictory(true);
            _progressManager.setStarted(false);
            _progressManager.getPlayerInfo().time = (int)_time;
            return GameState::Score;
        case GameMapState::GameOver:
            _progressManager.setVictory(false);
            _progressManager.setStarted(false);
            _progressManager.getPlayerInfo().time = (int)_time;
            return GameState::Score;
        case GameMapState::LevelStart:
        {
            _counterTime += deltaTime;
            if (_counterTime > 1.33f)
                _counterControl->setDefaultMaterial("counter1.png");
            else if (_counterTime > 0.67f)
                _counterControl->setDefaultMaterial("counter2.png");
            else
                _counterControl->setDefaultMaterial("counter3.png");
        }
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
        _counterTime = 0.01f;
        _gameMapProcessor->setState(GameMapState::LevelStart);

        if (_gameMapProcessor->getGameMap()->hasTutorial)
        {
            //_gameMapProcessor->setState(GameMapState::Pause);
            Game::getInstance()->setState(GameState::Tutorial);
        }
    } else {
        if (_gameMapProcessor->getState() != GameMapState::LevelStart)
            _gameMapProcessor->setState(GameMapState::Game);
    }
    _gameMapProcessor->setVisible(true);
    _document->show();
    if (_gameMapProcessor->getState() != GameMapState::LevelStart)
        _counterControl->setVisible(false);

    std::stringstream ss;
    ss << "Level " << _progressManager.getCurrentLevel() << ": " << _gameMapProcessor->getGameMap()->name;
    _document->getControlByName("level")->setText(ss.str());
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

                sunLight->getLightSettings().castShadows = Game::getInstance()->getGraphicsManager().getSettings().useShadows;
                isNight = false;
            } else {
                isNight = true;
                sunLight->getLightSettings().ambientStrength = {0.08f, 0.08f, 0.08f, 1.0f};
                sunLight->getLightSettings().diffuseStrength = {0.15f, 0.15f, 0.15f, 1.0f};
                sunLight->getLightSettings().specularStrength = {0.08f, 0.08f, 0.08f, 1.0f};
                sunLight->setNodeTransformation(
                        glm::translate(glm::mat4(1), glm::vec3(-20, 80, 80)));

                sunLight->getLightSettings().castShadows = false;
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
    auto fpsValue = std::accumulate(fpsList.begin(), fpsList.end(), 0.0f) / fpsList.size();
    fps->setText(std::to_string((int) fpsValue));

}

} // namespace Chewman