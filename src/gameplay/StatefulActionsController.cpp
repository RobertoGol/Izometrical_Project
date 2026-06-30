#include "gameplay/StatefulActionsController.hpp"
#include <algorithm>

namespace bunker
{

    bool StatefulActionsController::startAction(PilotActionState st, float durSec)
    {
        if (isBusy())
            return false;

        m_Timer.state = st;
        m_Timer.duration = durSec;
        m_Timer.timeRemaining = durSec;
        std::cout << "[ACTION] Начато прерываемое действие (" << durSec << "с): "
                  << ((st == PilotActionState::HealingStim) ? "ВВОД СТИМ-ИНЖЕКТОРА" : "ПЕРЕЗАРЯДКА ОРУЖИЯ")
                  << std::endl;
        return true;
    }

    void StatefulActionsController::interruptAction(const char *reason)
    {
        if (!isBusy())
            return;

        std::cout << "[ACTION INTERRUPTED] Действие сорвано! Причина: " << reason << std::endl;
        m_Timer.state = PilotActionState::Ready;
        m_Timer.timeRemaining = 0.0f;
    }

    void StatefulActionsController::update(GameState &gs, float dt)
    {
        if (!isBusy())
            return;

        m_Timer.timeRemaining -= dt;

        // Математическая процедурная анимация покачивания оружия Пилота при действии
        float progress = getProceduralAnimProgress();
        float bobAnimOffset = std::sin(progress * 3.14159265f) * 0.15f;
        gs.playerPos.z = bobAnimOffset; // Визуальный подъем/опускание рук в изометрии

        if (m_Timer.timeRemaining <= 0.0f)
        {
            if (m_Timer.state == PilotActionState::HealingStim)
            {
                gs.playerHealth = std::min(gs.playerMaxHealth, gs.playerHealth + 45.0f);
                std::cout << "[ACTION COMPLETE] Здоровье Пилота восстановлено!" << std::endl;
            }
            else if (m_Timer.state == PilotActionState::ReloadingWeapon)
            {
                std::cout << "[ACTION COMPLETE] Боекомплект успешно перезаряжен!" << std::endl;
            }
            gs.playerPos.z = 0.0f;
            m_Timer.state = PilotActionState::Ready;
        }
    }

} // namespace bunker