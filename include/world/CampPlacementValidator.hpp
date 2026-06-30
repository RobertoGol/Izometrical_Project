#pragma once

#include "gameplay/GameState.hpp"
#include <cmath>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Валидатор строительства C.A.M.P. (Волна 8)
    // Магнитный прилип стен (snap) с кнопкой отключения автомата прикрепления
    // ═══════════════════════════════════════════════════════════════════════

    class CampPlacementValidator
    {
    private:
        bool m_AutoSnapEnabled = true; // Кнопка отключения автомата прикрепления

    public:
        CampPlacementValidator() = default;

        void toggleAutoSnap() { m_AutoSnapEnabled = !m_AutoSnapEnabled; }
        bool isSnapEnabled() const { return m_AutoSnapEnabled; }

        bool validateAndSnap(const WorldGridState &grid, int &tileX, int &tileY) const;
    };

} // namespace bunker