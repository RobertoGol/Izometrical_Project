#include "persistence/CharacterManager.hpp"
#include <iostream>
#include <algorithm>

namespace bunker
{

    bool CharacterManager::awardExperience(GameState &gs, int amount)
    {
        return Progression::awardXP(gs, amount);
    }

    void CharacterManager::restoreSanityAndSoul(GameState &gs, float amount)
    {
        gs.characterProg.sanityLine = std::min(100.0f, gs.characterProg.sanityLine + amount);
        gs.characterProg.soulLine = std::min(100.0f, gs.characterProg.soulLine + amount);
        std::cout << "[SANITY & SOUL] Линии Разума и Души стабилизированы: "
                  << gs.characterProg.sanityLine << "% / " << gs.characterProg.soulLine << "%" << std::endl;
    }

    void CharacterManager::checkConsciousnessTrappingFailSafe(GameState &gs)
    {
        // Система безопасности воспоминаний игроков на крайний случай багов изоляции:
        if (gs.playerErosionLevel >= 95.0f)
        {
            gs.characterProg.sanityLine = std::max(0.0f, gs.characterProg.sanityLine - gs.deltaTime * 5.0f);
            if (gs.characterProg.sanityLine <= 0.0f && gs.characterProg.soulLine > 0.0f)
            {
                gs.characterProg.soulLine = std::max(0.0f, gs.characterProg.soulLine - 25.0f);
                gs.characterProg.sanityLine = 50.0f; // Душа удерживает Разум от распада!
                std::cout << "[META FAIL-SAFE] !! ПРЕДОХРАНИТЕЛЬ СОЗНАНИЯ !! Линия Души компенсирует распад воспоминаний!" << std::endl;
            }
        }
    }

} // namespace bunker