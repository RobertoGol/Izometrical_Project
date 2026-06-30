#pragma once

#include "gameplay/GameState.hpp"
#include <vector>

namespace bunker
{

    struct ConsumableBuff
    {
        unsigned int sourceItemId = 0;
        std::string buffName = "";
        float timeRemaining = 0.0f;
        float speedBonusMultiplier = 1.0f;
        float erosionResistBonus = 0.0f;
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Менеджер баффов от рационов РобКо (Приоритет P1)
    // Управляет временными эффектами ускорения бега и резистами эрозии
    // ═══════════════════════════════════════════════════════════════════════

    class ActiveBuffsController
    {
    private:
        std::vector<ConsumableBuff> m_ActiveBuffs;

    public:
        ActiveBuffsController() = default;

        void consumeRation(unsigned int itemId, const std::string &name);
        void update(GameState &gs, float dt);

        float getCurrentSpeedBonus() const;
        float getCurrentErosionResist() const;
        const std::vector<ConsumableBuff> &activeBuffs() const { return m_ActiveBuffs; }
    };

} // namespace bunker