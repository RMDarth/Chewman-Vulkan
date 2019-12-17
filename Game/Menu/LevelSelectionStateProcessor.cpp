// Chewman Vulkan game
// Copyright (c) 2018-2019, Igor Barinov
// Licensed under the MIT License
#include "LevelSelectionStateProcessor.h"
#include "Game/Controls/ControlDocument.h"
#include "Game/Controls/BoxSliderControl.h"
#include "Game/Game.h"
#include "Game/Utils.h"

namespace Chewman
{

LevelSelectionStateProcessor::LevelSelectionStateProcessor()
    : _document(std::make_unique<ControlDocument>("resources/game/GUI/levelselectmenu.xml"))
{
    _document->setMouseUpHandler(this);
    _document->hide();
}

LevelSelectionStateProcessor::~LevelSelectionStateProcessor() = default;

GameState LevelSelectionStateProcessor::update(float deltaTime)
{
    _document->update(deltaTime);
    return GameState::LevelSelection;
}

void LevelSelectionStateProcessor::processInput(const SDL_Event& event)
{
    processDocument(event, _document.get());

    if (event.type == SDL_KEYDOWN &&
        event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
    {
        Game::getInstance()->setState(GameState::MainMenu);
    }
}

void LevelSelectionStateProcessor::show()
{
    _document->show();

    auto& scoresManager= Game::getInstance()->getScoresManager();
    auto worldNum = Game::getInstance()->getProgressManager().getCurrentWorld();
    for (auto i = 0; i < 12; ++i)
    {
        auto control = _document->getControlByName(std::to_string(i));
        auto levelNumber = (i+1) + worldNum * 12;
        control->setText(std::to_string(levelNumber));
        control->setCustomAttribute("stars", std::to_string(scoresManager.getStars(levelNumber)));

        auto timeControl = _document->getControlByName("time" + std::to_string(i));
        if (scoresManager.getTime(levelNumber) == 0)
        {
            timeControl->setText("-:--");
            timeControl->setTextColor(glm::vec4(0.5, 0.5, 0.5, 1.0));
        } else {
            timeControl->setText(Utils::timeToString(scoresManager.getTime(levelNumber)));
            timeControl->setTextColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
        }
    }
}

void LevelSelectionStateProcessor::hide()
{
    _document->hide();
}

bool LevelSelectionStateProcessor::isOverlapping()
{
    return true;
}

void LevelSelectionStateProcessor::processEvent(Control* control, IEventHandler::EventType type, int x, int y)
{
    if (type == IEventHandler::MouseUp)
    {
        if (control->getName() == "back")
        {
            hide();
            Game::getInstance()->setState(GameState::WorldSelection);
        }
        else if (control->getType() == ControlType::LevelButton)
        {
            auto levelNum = std::stoul(control->getText());
            auto& progressManager = Game::getInstance()->getProgressManager();
            progressManager.setCurrentLevel(levelNum);
            progressManager.setVictory(false);
            progressManager.setStarted(false);
            progressManager.resetPlayerInfo();
            Game::getInstance()->setState(GameState::Level);
        }
    }
}

} // namespace Chewman