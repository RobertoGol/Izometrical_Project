#pragma once

#include "ai/HostileAISystem.hpp"
#include <vector>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Внешняя дата-таблица параметров боевых архетипов ИИ (Пакет 3 / Фаза 3)
    // Выносит хардкод из конструктора движка в централизованный реестр
    // ═══════════════════════════════════════════════════════════════════════

    class EnemyArchetypeRegistry
    {
    private:
        static std::vector<HostileProfile> s_Profiles;

    public:
        static void initializeRegistry();
        static const HostileProfile &getProfile(HostileKind kind);
    };

} // namespace bunker
