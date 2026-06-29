#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include <vector>

namespace bunker
{

    enum class DestructibleKind
    {
        FragileGlassPanel, // Хрупкие стёкла (Glass break)
        RustedSteamCar,    // Ржавые разрушенные машины и транспорт
        TrashDebrisPile,   // Мусор и кучи обломков
        ConcreteBarricade  // Барикады C.A.M.P.
    };

    struct DestructibleProp
    {
        Vector3D position;
        DestructibleKind kind = DestructibleKind::TrashDebrisPile;
        float health = 100.0f;
        bool isDestroyed = false;
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Реактивная физика деструктивного окружения (Приоритет P1)
    // Разлёт стёкол, сдвиг ржавого транспорта и разлёт мусора от волн Скорча
    // ═══════════════════════════════════════════════════════════════════════

    class DestructibleEnvironmentManager
    {
    private:
        std::vector<DestructibleProp> m_Props;

    public:
        DestructibleEnvironmentManager();

        void spawnDefaultProps();
        void applyBlastDisruption(GameState &gs, const Vector3D &blastOrigin, float blastRadius, float blastDamage);

        const std::vector<DestructibleProp> &allProps() const { return m_Props; }
    };

} // namespace bunker
