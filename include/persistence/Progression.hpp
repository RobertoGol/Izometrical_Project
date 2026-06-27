#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "core/Constants.hpp"
#include <string>

namespace bunker
{

    class Progression
    {
    public:
        enum class StatType
        {
            MaxHP,
            MaxMP,
            MoveSpeed,
            DamageBonus,
            ErosionResist
        };

        Progression() = default;

        static bool awardXP(GameState &gs, int amount, std::string *eventText = nullptr);
        static void onEnemyKilled(GameState &gs);
        static void onContainerOpened(GameState &gs);
        static void onPipPadFound(GameState &gs);
        static void onBaseCleared(GameState &gs);
        static int xpRequiredForLevel(int level);
        static float xpPercent(const GameState &gs);
        static bool spendPoint(GameState &gs, StatType stat);

        static int getLevel(const GameState &gs) { return gs.characterProg.level; }
        static int getXP(const GameState &gs) { return gs.characterProg.experience; }
        static int getXPRequired(const GameState &gs) { return xpRequiredForLevel(gs.characterProg.level); }
        static int getUnusedPoints(const GameState &gs) { return gs.characterProg.unusedPoints; }
        static float getHP(const GameState &gs) { return gs.characterProg.hp; }
        static float getMP(const GameState &gs) { return gs.characterProg.mp; }
    };

} // namespace bunker
