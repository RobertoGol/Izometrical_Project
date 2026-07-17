#pragma once

#include "gameplay/GameState.hpp"
#include <cmath>

namespace bunker
{

    enum class PilotLocomotionMode
    {
        GroundRunning,
        FloorSlideHop,   // Скольжение по полу (Knee slide / Slide hopping)
        WallRunningLeft, // Ускорение и скольжение по левой стене
        WallRunningRight,
        GrappleZipline // Механика тарзанки / крюка-кошки Titanfall 2
    };

    struct GrappleCable
    {
        bool isAttached = false;
        Vector3D anchorPoint = {0.0f, 0.0f, 0.0f};
        float cableLength = 0.0f;
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Тактическая акробатика Пилота Убежища 17 (Titanfall 2 Locomotion)
    // Ускорение бега, скольжение по стенам/полу и крюк-кошка (тарзанка)
    // ═══════════════════════════════════════════════════════════════════════

    class AdvancedPilotMovementController
    {
    private:
        PilotLocomotionMode m_Mode = PilotLocomotionMode::GroundRunning;
        GrappleCable m_Grapple;
        float m_SlideTimer = 0.0f;

    public:
        AdvancedPilotMovementController() = default;

        void triggerKneeSlide(GameState &gs);
        void fireGrappleZipline(GameState &gs, const Vector3D &targetAnchor);
        void detachGrapple();
        void updateLocomotionPhysics(GameState &gs, const WorldGridState &grid, float dt);

        PilotLocomotionMode activeLocomotion() const { return m_Mode; }
        bool isGrappling() const { return m_Grapple.isAttached; }
    };

} // namespace bunker