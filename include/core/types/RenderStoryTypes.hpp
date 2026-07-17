#pragma once

#include "core/Constants.hpp"
#include "core/types/GameplayTypes.hpp"
#include <functional>
#include <string>
#include <vector>

namespace bunker
{
    // ══════════════════════════════════════════════════════════════════════
    // Рендер-очередь (для Z-сортировки)
    // ══════════════════════════════════════════════════════════════════════

    struct RenderObject
    {
        float depth;
        std::function<void()> drawFunc;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Прогрессия персонажа (из GMyGameDoNotTouch)
    // ══════════════════════════════════════════════════════════════════════

    struct CharacterProgression
    {
        int level = 1;
        int experience = 0;
        int unusedPoints = 0;
        float hp = Config::PLAYER_START_HP;
        float maxHp = Config::PLAYER_START_HP;
        float mp = 50.0f;
        float maxMp = 50.0f;
        float sanityLine = 100.0f; // Линия Разума (Резервный предохранитель сознания)
        float soulLine = 100.0f;   // Линия Души (Система безопасности воспоминаний)
        std::vector<InventoryItem> inventory;
    };

    // ══════════════════════════════════════════════════════════════════════
    // Сюжет
    // ══════════════════════════════════════════════════════════════════════

    struct StoryState
    {
        bool pipPadRecovered = false;
        bool archiveRecovered = false;
        bool tankLinked = false;
        bool bucketRecovered = false;
        bool exitedBunker = false;
    };

    struct StoryRouteEntry
    {
        std::string text;
        bool completed = false;
    };


} // namespace bunker
