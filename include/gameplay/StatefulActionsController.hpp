#pragma once

#include "gameplay/GameState.hpp"
#include <iostream>
#include <cmath>

namespace bunker
{

    enum class PilotActionState
    {
        Ready,
        HealingStim,
        ReloadingWeapon
    };

    struct ActionTimer
    {
        PilotActionState state = PilotActionState::Ready;
        float timeRemaining = 0.0f;
        float duration = 0.0f;
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Тайминговые стейт-машины действий Пилота (Приоритет P1)
    // Математическая процедурная анимация перезарядки и лечения. Срыв при уроне!
    // ═══════════════════════════════════════════════════════════════════════

    class StatefulActionsController
    {
    private:
        ActionTimer m_Timer;

    public:
        StatefulActionsController() = default;

        bool startAction(PilotActionState st, float durSec);
        void interruptAction(const char *reason);
        void update(GameState &gs, float dt);

        bool isBusy() const { return m_Timer.state != PilotActionState::Ready; }
        PilotActionState currentState() const { return m_Timer.state; }
        float getProceduralAnimProgress() const { return (m_Timer.duration > 0.0f) ? (1.0f - m_Timer.timeRemaining / m_Timer.duration) : 0.0f; }
    };

} // namespace bunker
