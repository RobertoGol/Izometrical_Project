#include "persistence/Progression.hpp"
#include <algorithm>
#include <iostream>

namespace bunker
{

    bool Progression::awardXP(GameState &gs, int amount, std::string *eventText)
    {
        if (amount <= 0)
            return false;

        auto &prog = gs.characterProg;
        prog.experience += amount;
        bool leveledUp = false;

        while (prog.experience >= xpRequiredForLevel(prog.level))
        {
            prog.experience -= xpRequiredForLevel(prog.level);
            prog.level += 1;
            prog.unusedPoints += Config::POINTS_PER_LEVEL;

            prog.maxHp += Config::HP_PER_LEVEL;
            prog.maxMp += Config::MP_PER_LEVEL;

            prog.hp = prog.maxHp;
            prog.mp = prog.maxMp;

            gs.playerMaxHealth = prog.maxHp;
            gs.playerHealth = prog.hp;

            leveledUp = true;

            std::cout << "[LEVEL UP] Уровень " << prog.level
                      << "! HP:" << prog.maxHp
                      << " MP:" << prog.maxMp
                      << " Очков:" << prog.unusedPoints << std::endl;
        }

        if (eventText != nullptr)
        {
            if (leveledUp)
            {
                *eventText = "LEVEL UP! Lv." + std::to_string(prog.level) + " | +" + std::to_string(Config::POINTS_PER_LEVEL) + " attribute points";
            }
            else
            {
                *eventText = "+" + std::to_string(amount) + " XP";
            }
        }

        return leveledUp;
    }

    void Progression::onEnemyKilled(GameState &gs)
    {
        awardXP(gs, 25);
    }

    void Progression::onContainerOpened(GameState &gs)
    {
        awardXP(gs, 10);
    }

    void Progression::onPipPadFound(GameState &gs)
    {
        awardXP(gs, 100);
    }

    void Progression::onBaseCleared(GameState &gs)
    {
        awardXP(gs, 500);
    }

    int Progression::xpRequiredForLevel(int level)
    {
        int safeLevel = std::max(1, level);
        return Config::BASE_XP_PER_LEVEL + (safeLevel - 1) * Config::XP_INCREMENT;
    }

    float Progression::xpPercent(const GameState &gs)
    {
        int required = xpRequiredForLevel(gs.characterProg.level);
        if (required <= 0)
            return 0.0f;
        return static_cast<float>(gs.characterProg.experience) / static_cast<float>(required);
    }

    bool Progression::spendPoint(GameState &gs, StatType stat)
    {
        auto &prog = gs.characterProg;
        if (prog.unusedPoints <= 0)
            return false;

        prog.unusedPoints--;

        switch (stat)
        {
        case StatType::MaxHP:
            prog.maxHp += 5.0f;
            gs.playerMaxHealth = prog.maxHp;
            break;
        case StatType::MaxMP:
            prog.maxMp += 5.0f;
            break;
        case StatType::MoveSpeed:
            gs.playerSpeed += 0.3f;
            break;
        case StatType::DamageBonus:
            break;
        case StatType::ErosionResist:
            break;
        }

        std::cout << "[SKILL] Очко потрачено. Осталось: " << prog.unusedPoints << std::endl;
        return true;
    }

} // namespace bunker
