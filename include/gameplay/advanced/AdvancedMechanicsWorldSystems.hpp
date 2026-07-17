#pragma once

#include "gameplay/advanced/AdvancedMechanicsCommon.hpp"
namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 5) REACTIVE BREAKABLES + SHOCK WAVES
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class BreakableKind
    {
        Glass,
        Vegetation,
        Crate,
        Barrel,
        Console
    };

    struct BreakableObject
    {
        int id = 0;
        BreakableKind kind = BreakableKind::Crate;
        Vector3D position;
        float health = 30.0f;
        float radius = 0.35f;
        bool broken = false;
        Vector3D velocity;
    };

    struct ShockWave
    {
        Vector3D origin;
        float radius = 0.1f;
        float maxRadius = 4.0f;
        float force = 1.0f;
        float ttl = 0.45f;
    };

    class ReactiveWorldSystem
    {
      public:
        int add(BreakableKind kind, Vector3D pos);
        void seedDefault();
        void update(GameState& gs, float dt);
        void damageAt(GameState& gs, Vector3D pos, float radius, float damage, float impulse);

        const std::vector<BreakableObject>& breakables() const
        {
            return m_Breakables;
        }
        const std::vector<ShockWave>& waves() const
        {
            return m_Waves;
        }

      private:
        int m_NextId = 0;
        std::vector<BreakableObject> m_Breakables;
        std::vector<ShockWave> m_Waves;

        void explodeBarrel(GameState& gs, BreakableObject& b);
        void applyWave(GameState& gs, const ShockWave& wave, float dt);
    };


} // namespace bunker
