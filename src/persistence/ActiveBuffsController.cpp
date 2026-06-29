#include "persistence/ActiveBuffsController.hpp"
#include <iostream>
#include <algorithm>

namespace bunker
{

    void ActiveBuffsController::consumeRation(unsigned int itemId, const std::string &name)
    {
        ConsumableBuff b;
        b.sourceItemId = itemId;
        b.buffName = name;
        b.timeRemaining = 120.0f; // Баффы рационов РобКо действуют 2 минуты реального времени

        if (name.find("STAMINA") != std::string::npos || name.find("NUKA") != std::string::npos)
        {
            b.speedBonusMultiplier = 1.35f; // Ускорение бега Пилота
        }
        else if (name.find("PROTEIN") != std::string::npos || name.find("FOCUS") != std::string::npos)
        {
            b.erosionResistBonus = 25.0f; // Бонус к сопротивлению Эфирной Эрозии
        }

        m_ActiveBuffs.push_back(b);
        std::cout << "[BUFF MANAGER] Активирован рацион РобКо: " << name << " (+120с действия)" << std::endl;
    }

    void ActiveBuffsController::update(GameState &gs, float dt)
    {
        float speedMult = 1.0f;
        for (auto &b : m_ActiveBuffs)
        {
            b.timeRemaining -= dt;
            speedMult = std::max(speedMult, b.speedBonusMultiplier);
        }

        m_ActiveBuffs.erase(
            std::remove_if(m_ActiveBuffs.begin(), m_ActiveBuffs.end(), [](const ConsumableBuff &b)
                           { return b.timeRemaining <= 0.0f; }),
            m_ActiveBuffs.end());

        gs.playerSpeed = Config::PLAYER_WALK_SPEED * speedMult;
    }

    float ActiveBuffsController::getCurrentSpeedBonus() const
    {
        float m = 1.0f;
        for (const auto &b : m_ActiveBuffs)
            m = std::max(m, b.speedBonusMultiplier);
        return m;
    }

    float ActiveBuffsController::getCurrentErosionResist() const
    {
        float r = 0.0f;
        for (const auto &b : m_ActiveBuffs)
            r += b.erosionResistBonus;
        return r;
    }

} // namespace bunker
