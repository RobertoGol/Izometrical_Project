#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Constants.hpp"
#include <string>
#include <algorithm>
#include <iostream>

namespace bunker {

class Progression {
public:
    Progression() = default;

    // ═══════════════════════════════════════════════
    // Начисление XP. Автоматически повышает уровень
    // при достижении порога. Возвращает true если был
    // level up.
    // ═══════════════════════════════════════════════
    static bool awardXP(GameState& gs, int amount, std::string* eventText = nullptr) {
        if (amount <= 0) return false;

        auto& prog = gs.characterProg;
        prog.experience += amount;
        bool leveledUp = false;

        while (prog.experience >= xpRequiredForLevel(prog.level)) {
            prog.experience -= xpRequiredForLevel(prog.level);
            prog.level += 1;
            prog.unusedPoints += Config::POINTS_PER_LEVEL;

            // Повышение максимального HP и MP
            prog.maxHp += Config::HP_PER_LEVEL;
            prog.maxMp += Config::MP_PER_LEVEL;

            // Полное восстановление при левел-апе
            prog.hp = prog.maxHp;
            prog.mp = prog.maxMp;

            // Синхронизация с GameState
            gs.playerMaxHealth = prog.maxHp;
            gs.playerHealth    = prog.hp;

            leveledUp = true;

            std::cout << "[LEVEL UP] Уровень " << prog.level
                      << "! HP:" << prog.maxHp
                      << " MP:" << prog.maxMp
                      << " Очков:" << prog.unusedPoints << std::endl;
        }

        if (eventText != nullptr) {
            if (leveledUp) {
                *eventText = "LEVEL UP! Lv." + std::to_string(prog.level)
                           + " | +" + std::to_string(Config::POINTS_PER_LEVEL) + " attribute points";
            } else {
                *eventText = "+" + std::to_string(amount) + " XP";
            }
        }

        return leveledUp;
    }

    // ═══════════════════════════════════════════════
    // XP за убийство врага (вызывать при score += 150)
    // ═══════════════════════════════════════════════
    static void onEnemyKilled(GameState& gs) {
        awardXP(gs, 25);
    }

    // XP за вскрытие контейнера
    static void onContainerOpened(GameState& gs) {
        awardXP(gs, 10);
    }

    // XP за подбор Pip-Pad (сюжетное)
    static void onPipPadFound(GameState& gs) {
        awardXP(gs, 100);
    }

    // XP за зачистку базы
    static void onBaseCleared(GameState& gs) {
        awardXP(gs, 500);
    }

    // ═══════════════════════════════════════════════
    // Расчёт XP для следующего уровня
    // Формула: 100 + (level - 1) * 75
    // ═══════════════════════════════════════════════
    static int xpRequiredForLevel(int level) {
        int safeLevel = std::max(1, level);
        return Config::BASE_XP_PER_LEVEL + (safeLevel - 1) * Config::XP_INCREMENT;
    }

    // ═══════════════════════════════════════════════
    // Процент до следующего уровня (для XP бара)
    // ═══════════════════════════════════════════════
    static float xpPercent(const GameState& gs) {
        int required = xpRequiredForLevel(gs.characterProg.level);
        if (required <= 0) return 0.0f;
        return static_cast<float>(gs.characterProg.experience) / static_cast<float>(required);
    }

    // ═══════════════════════════════════════════════
    // Потратить очко навыка
    // ═══════════════════════════════════════════════
    enum class StatType { MaxHP, MaxMP, MoveSpeed, DamageBonus, ErosionResist };

    static bool spendPoint(GameState& gs, StatType stat) {
        auto& prog = gs.characterProg;
        if (prog.unusedPoints <= 0) return false;

        prog.unusedPoints--;

        switch (stat) {
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
            // Будет применяться в BulletSystem
            break;
        case StatType::ErosionResist:
            // Снижает накопление эрозии
            break;
        }

        std::cout << "[SKILL] Очко потрачено. Осталось: " << prog.unusedPoints << std::endl;
        return true;
    }

    // ═══════════════════════════════════════════════
    // Геттеры для HUD
    // ═══════════════════════════════════════════════
    static int   getLevel(const GameState& gs)        { return gs.characterProg.level; }
    static int   getXP(const GameState& gs)           { return gs.characterProg.experience; }
    static int   getXPRequired(const GameState& gs)   { return xpRequiredForLevel(gs.characterProg.level); }
    static int   getUnusedPoints(const GameState& gs) { return gs.characterProg.unusedPoints; }
    static float getHP(const GameState& gs)           { return gs.characterProg.hp; }
    static float getMP(const GameState& gs)           { return gs.characterProg.mp; }
};

}  // namespace bunker
