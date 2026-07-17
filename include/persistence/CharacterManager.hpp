#pragma once

#include "persistence/Inventory.hpp"
#include "persistence/Progression.hpp"
#include "gameplay/GameState.hpp"

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Фасад контроллеров персонажа (Пакет 4 / Фаза 2)
    // Объединяет инвентарь, прогрессию и аварийную систему Разум/Душа
    // ═══════════════════════════════════════════════════════════════════════

    class CharacterManager
    {
    private:
        PlayerInventory &m_Inventory;

    public:
        explicit CharacterManager(PlayerInventory &inv) : m_Inventory(inv) {}

        // ── Начисление опыта и синхронизация в структуру прогрессии ──
        bool awardExperience(GameState &gs, int amount);

        // ── Восстановление линий безопасности Разума и Души ──
        void restoreSanityAndSoul(GameState &gs, float amount);

        // ── Аварийная проверка деградации Разума при критической эрозии ──
        void checkConsciousnessTrappingFailSafe(GameState &gs);

        PlayerInventory &inventory() { return m_Inventory; }
        const PlayerInventory &inventory() const { return m_Inventory; }
    };

} // namespace bunker