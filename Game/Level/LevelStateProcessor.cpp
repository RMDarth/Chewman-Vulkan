// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include <sstream>
#include <iomanip>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include "LevelStateProcessor.h"
#include "Game/Game.h"
#include "Game/SystemApi.h"
#include "Game/Level/GameMapLoader.h"
#include "Game/Controls/ControlDocument.h"
#include "SVE/SceneManager.h"
#include "SVE/LightManager.h"
#include "GameUtils.h"


namespace Chewman
{

LevelStateProcessor::LevelStateProcessor()
    : _progressManager(Game::getInstance()->getProgressManager())
    , _document(std::make_unique<ControlDocument>(isWideScreen() ? "resources/game/GUI/HUDWide.xml" : "resources/game/GUI/HUD.xml"))
{
    _document->setMouseUpHandler(this);
    _counterControl = _document->getControlByName("Counter");
    _counterControl->setDefaultMaterial("counter1.png");
    _counterControl->setDefaultMaterial("counter2.png");
    _counterControl->setDefaultMaterial("counter3.png");

    _loadingControl = _document->getControlByName("loading");

    _document->getControlByName("upStick")->setMouseDownHandler(this);
    _document->getControlByName("downStick")->setMouseDownHandler(this);
    _document->getControlByName("leftStick")->setMouseDownHandler(this);
    _document->getControlByName("rightStick")->setMouseDownHandler(this);
    _document->getControlByName("stickBackground")->setMouseDownHandler(this);
    _document->getControlByName("stickBackground")->setMouseMoveHandler(this);

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
        _progressManager.setGameMapService(_gameMapProcessor.get());
        _progressManager.setCurrentLevelInfo({
            _gameMapProcessor->getGameMap()->timeFor2Stars,
            _gameMapProcessor->getGameMap()->timeFor3Stars,
            _gameMapProcessor->getGameMap()->name } );

        if (!_gameMapProcessor->getGameMap()->isNight || Game::getInstance()->getGraphicsManager().getSettings().dynamicLights == LightSettings::Off)
        {
            setSunLight(SunLightType::Day);
        } else {
            setSunLight(SunLightType::Night);
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
                _loadingControl->setVisible(false);
                _counterTime = -1;
            }
            break;
        case GameMapState::Pause:
            break;
        case GameMapState::Animation:
            if (_gameMapProcessor->getGameMap()->player->isDying())
                _time += deltaTime;
            break;
        case GameMapState::Victory:
            _progressManager.setVictory(true);
            _progressManager.setStarted(false);
            _progressManager.getPlayerInfo().time = (int)_time;
            return GameState::Score;
        case GameMapState::GameOver:
        {
            _progressManager.setVictory(false);
            _progressManager.getPlayerInfo().time = (int) _time;
            if (_reviveUsed)
            {
                Game::getInstance()->getProgressManager().setStarted(false);
                return GameState::Score;
            }
            else
            {
                _reviveUsed = true;
                return GameState::Revive;
            }
        }
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
                _loadingControl->setVisible(false);
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

    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_MINIMIZED)
    {
        _gameMapProcessor->setState(GameMapState::Pause);
        Game::getInstance()->setState(GameState::Pause);
    }
    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        _gameMapProcessor->setState(GameMapState::Pause);
        Game::getInstance()->setState(GameState::Pause);
    }
}

void LevelStateProcessor::show()
{
    if (!_progressManager.isStarted())
    {
        // new level started
        _progressManager.setStarted(true);
        _progressManager.setVictory(false);
        _progressManager.getPlayerInfo().livesLostOnLevel = 0;
        _loadingFinished = false;
        _lastDirection = MoveDirection::None;
        _reviveUsed = false;
        initMap();
        _time = 0.0f;
        _counterTime = 0.01f;
        _gameMapProcessor->setState(GameMapState::LevelStart);

        if (_gameMapProcessor->getGameMap()->hasTutorial)
        {
            Game::getInstance()->setState(GameState::Tutorial);
        }
    } else {
        if (_gameMapProcessor->getState() != GameMapState::LevelStart)
            _gameMapProcessor->setState(GameMapState::Game);
    }
    _gameMapProcessor->setVisible(true);
    _document->show();
    if (!_countToRemove)
    {
        _loadingControl->setVisible(false);
    }
    if (_gameMapProcessor->getState() != GameMapState::LevelStart)
    {
        _loadingControl->setVisible(false);
        _counterControl->setVisible(false);
    }

    std::stringstream ss;
    ss << Game::getInstance()->getLocaleManager().getLocalizedString("Level") << " "
       << _progressManager.getCurrentLevel() << ": " << _gameMapProcessor->getGameMap()->name;
    _document->getControlByName("level")->setText(ss.str());

    _useOnScreenControl = Game::getInstance()->getGameSettingsManager().getSettings().showOnScreenControls;
    _document->getControlByName("upStick")->setVisible(_useOnScreenControl);
    _document->getControlByName("downStick")->setVisible(_useOnScreenControl);
    _document->getControlByName("leftStick")->setVisible(_useOnScreenControl);
    _document->getControlByName("rightStick")->setVisible(_useOnScreenControl);
    _document->getControlByName("FPS")->setVisible(_showFPS);

    System::hideAds();
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
            _loadingControl->setVisible(false);
            _counterControl->setVisible(false);
            _document->getControlByName("camera")->setVisible(false);
            _gameMapProcessor->setState(GameMapState::Pause);
            Game::getInstance()->setState(GameState::Pause);
        }
        else if (control->getName() == "lifeimg")
        {
            _showFPS = !_showFPS;
            _document->getControlByName("FPS")->setVisible(_showFPS);
        }
        else if (control->getName() == "camera")
        {
            auto& gameSettings = Game::getInstance()->getGameSettingsManager();
            auto cameraStyle = static_cast<uint8_t>(gameSettings.getSettings().cameraStyle);
            cameraStyle = (cameraStyle + 1) % 3;
            gameSettings.getSettings().cameraStyle = static_cast<CameraStyle>(cameraStyle);
            gameSettings.store();
        }
    }
    if (type == IEventHandler::MouseDown || type == IEventHandler::MouseUp)
    {
        if (_useOnScreenControl)
        {
            if (control->getName() == "leftStick")
            {
                _gameMapProcessor->setNextMove(MoveDirection::Down);
                _lastDirection = MoveDirection::Down;
            } else if (control->getName() == "rightStick")
            {
                _gameMapProcessor->setNextMove(MoveDirection::Up);
                _lastDirection = MoveDirection::Up;
            } else if (control->getName() == "upStick")
            {
                _gameMapProcessor->setNextMove(MoveDirection::Right);
                _lastDirection = MoveDirection::Right;
            } else if (control->getName() == "downStick")
            {
                _gameMapProcessor->setNextMove(MoveDirection::Left);
                _lastDirection = MoveDirection::Left;
            } else if (control->getName() == "stickBackground")
            {
                _gameMapProcessor->setNextMove(_lastDirection);
            }
        }
    }
    if (type == IEventHandler::MouseMove)
    {
        if (_useOnScreenControl && control->getName() == "stickBackground")
        {
            _gameMapProcessor->setNextMove(_lastDirection);
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

    if (_showFPS)
    {
        auto fpsValue = std::accumulate(fpsList.begin(), fpsList.end(), 0.0f) / fpsList.size();
        fps->setText("FPS: " + std::to_string((int) fpsValue));
    }

    updatePowerUps();
    if (_useOnScreenControl)
        updateArrows();
}

void LevelStateProcessor::updatePowerUps()
{
    static const std::map<PowerUpType, std::pair<std::string /*Material*/, float /*maxTime*/>> iconInfo =
        {
            { PowerUpType::Acceleration, { "IconSpeedMaterial",         AccelerationTotalTime}},
            { PowerUpType::Slow,         { "IconSlowMaterial",          SlowTotalTime}},
            { PowerUpType::Freeze,       { "IconFreezeMaterial",        FreezeTotalTime}},
            { PowerUpType::Pentagram,    { "IconPentagramMaterial",     PentagrammTotalTime}},
            { PowerUpType::Teeth,        { "IconTeethMaterial",         TeethTotalTime}},
            { PowerUpType::Jackhammer,   { "IconJackhammerMaterial",    JackhammerTotalTime}}

        };
    auto affectorMap = _gameMapProcessor->getCurrentAffectors();
    int size = affectorMap.size();
    int controlId = 1;

    using ElementType = std::pair<const PowerUpType, float>;
    while (size > 0)
    {
        auto minIter = std::min_element(affectorMap.begin(), affectorMap.end(), [](const ElementType& e1, const ElementType& e2) { return e1.second > e2.second; });
        PowerUpType curType = minIter->first;
        float remainingTime = minIter->second;
        float maxTime = iconInfo.at(curType).second;
        affectorMap.erase(minIter);

        std::string controlName = "powerup";
        controlName.push_back('0' + controlId);
        auto control = _document->getControlByName(controlName);
        control->setRawMaterial(iconInfo.at(curType).first);
        control->getOverlay()->setCustomData(1.0 - remainingTime / maxTime);
        ++controlId;

        size = affectorMap.size();
    }

    for (; controlId <= 5; ++controlId)
    {
        std::string controlName = "powerup";
        controlName.push_back('0' + controlId);
        _document->getControlByName(controlName)->setRawMaterial("");
    }
}

void LevelStateProcessor::updateArrows()
{
    static auto upArrow = _document->getControlByName("upStick");
    static auto downArrow = _document->getControlByName("downStick");
    static auto leftArrow = _document->getControlByName("leftStick");
    static auto rightArrow = _document->getControlByName("rightStick");

    upArrow->setColor(glm::vec4(1));
    downArrow->setColor(glm::vec4(1));
    leftArrow->setColor(glm::vec4(1));
    rightArrow->setColor(glm::vec4(1));

    switch (_gameMapProcessor->getNextMove())
    {
        case MoveDirection::Left:
            downArrow->setColor(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
            break;
        case MoveDirection::Up:
            rightArrow->setColor(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
            break;
        case MoveDirection::Right:
            upArrow->setColor(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
            break;
        case MoveDirection::Down:
            leftArrow->setColor(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
            break;
    }
}

} // namespace Chewman