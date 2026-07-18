#include "gameplay/AdvancedMechanics.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 2) RADIO / TAPE SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    void SurvivalSystem::update(GameState& gs, PlayerInventory& inv, float dt)
    {
        (void)inv;
        updateStress(gs, dt);
        updateBuffs(gs, dt);
        updateReload(gs, dt);
        updateSecondWindAndSoulLine(gs);
    }

    bool SurvivalSystem::useStim(GameState& gs, PlayerInventory& inv)
    {
        if (!inv.hasItem(ITEM_STIM) || gs.playerHealth >= gs.playerMaxHealth)
        {
            return false;
        }
        inv.removeItem(ITEM_STIM, 1);
        gs.playerHealth = advClamp(gs.playerHealth + 45.0f, 0.0f, gs.playerMaxHealth);
        m_Stress = advClamp(m_Stress - 10.0f, 0.0f, 100.0f);
        return true;
    }

    bool SurvivalSystem::eatRation(GameState& gs, PlayerInventory& inv, RationKind kind)
    {
        unsigned int id = ITEM_RATION_PROTEIN;
        float amount = 1.0f;
        float duration = 45.0f;
        switch (kind)
        {
        case RationKind::Protein:
            id = ITEM_RATION_PROTEIN;
            amount = 12.0f;
            duration = 60.0f;
            break;
        case RationKind::Stamina:
            id = ITEM_RATION_STAMINA;
            amount = 0.35f;
            duration = 50.0f;
            break;
        case RationKind::Focus:
            id = ITEM_RATION_FOCUS;
            amount = 0.20f;
            duration = 35.0f;
            break;
        case RationKind::AntiErosion:
            id = ITEM_RATION_ANTI_EROSION;
            amount = 25.0f;
            duration = 40.0f;
            break;
        default:
            break;
        }
        if (!inv.hasItem(id))
        {
            return false;
        }
        inv.removeItem(id, 1);
        m_Buffs.push_back({kind, duration, amount});
        if (kind == RationKind::Protein)
        {
            gs.playerMaxHealth += amount;
            gs.playerHealth = advClamp(gs.playerHealth + amount, 0.0f, gs.playerMaxHealth);
        }
        return true;
    }

    void SurvivalSystem::startReload(PlayerInventory& inv)
    {
        if (m_Weapon.isReloading || m_Weapon.magazine >= m_Weapon.magazineMax)
        {
            return;
        }
        if (m_Weapon.reserveAmmo <= 0 && !inv.hasItem(ITEM_AMMO_556))
        {
            return;
        }
        if (m_Weapon.reserveAmmo <= 0 && inv.removeItem(ITEM_AMMO_556, 1))
        {
            m_Weapon.reserveAmmo += 30;
        }
        m_Weapon.isReloading = true;
        m_Weapon.reloadTimer = reloadDuration();
    }

    bool SurvivalSystem::consumeRound()
    {
        if (m_Weapon.isReloading || m_Weapon.magazine <= 0)
        {
            return false;
        }
        --m_Weapon.magazine;
        return true;
    }

    float SurvivalSystem::speedMultiplier() const
    {
        float result = 1.0f;
        for (const auto& b : m_Buffs)
        {
            if (b.kind == RationKind::Stamina)
            {
                result += b.amount;
            }
        }
        if (m_Stress > 70.0f)
        {
            result -= 0.15f;
        }
        return advClamp(result, 0.55f, 1.75f);
    }

    float SurvivalSystem::aimPenalty() const
    {
        float penalty = (m_Stress > 35.0f) ? (m_Stress - 35.0f) / 650.0f : 0.0f;
        for (const auto& b : m_Buffs)
        {
            if (b.kind == RationKind::Focus)
            {
                penalty -= b.amount;
            }
        }
        return advClamp(penalty, 0.0f, 0.35f);
    }

    float SurvivalSystem::reloadDuration() const
    {
        float d = 1.45f;
        for (const auto& b : m_Buffs)
        {
            if (b.kind == RationKind::Focus)
            {
                d *= 0.82f;
            }
        }
        return d;
    }

    void SurvivalSystem::updateStress(GameState& gs, float dt)
    {
        bool nearEnemy = false;
        for (const auto& e : gs.enemies)
        {
            if (e.isAlive && advDistSq(e.position, gs.playerPos) < 5.5f * 5.5f)
            {
                nearEnemy = true;
                break;
            }
        }
        if (nearEnemy || gs.playerHealth < gs.playerMaxHealth * 0.35f)
        {
            addStress((nearEnemy ? 4.0f : 2.0f) * dt);
        }
        else
        {
            calm(2.5f * dt);
        }
    }

    void SurvivalSystem::updateBuffs(GameState& gs, float dt)
    {
        for (auto& b : m_Buffs)
        {
            b.timeLeft -= dt;
            if (b.kind == RationKind::AntiErosion)
            {
                gs.playerErosionLevel = advClamp(gs.playerErosionLevel - b.amount * 0.03f * dt, 0.0f, 100.0f);
            }
        }
        m_Buffs.erase(
            std::remove_if(m_Buffs.begin(), m_Buffs.end(), [](const ActiveBuff& b) { return b.timeLeft <= 0.0f; }),
            m_Buffs.end());
    }

    void SurvivalSystem::updateReload(GameState& gs, float dt)
    {
        gs.fireCooldown = std::max(0.0f, gs.fireCooldown - dt);
        if (!m_Weapon.isReloading)
        {
            return;
        }
        m_Weapon.reloadTimer -= dt;
        if (m_Weapon.reloadTimer <= 0.0f)
        {
            const int need = m_Weapon.magazineMax - m_Weapon.magazine;
            const int take = std::min(need, m_Weapon.reserveAmmo);
            m_Weapon.magazine += take;
            m_Weapon.reserveAmmo -= take;
            m_Weapon.isReloading = false;
        }
    }

    void SurvivalSystem::updateSecondWindAndSoulLine(GameState& gs)
    {
        if (gs.playerHealth <= 0.0f && !m_SoulLineUsed)
        {
            m_SoulLineUsed = true;
            gs.playerHealth = 1.0f;
            m_Stress = 100.0f;
            bunker::logInfo() << "[SURVIVAL] !! SOUL LINE !! Срыв смерти! Нить души удержала Пилота в Убежище 17!"
                              << std::endl;
            return;
        }
        if (gs.playerHealth > 0.0f && gs.playerHealth < gs.playerMaxHealth * 0.16f && !m_SecondWindUsed &&
            m_Stress >= 55.0f)
        {
            m_SecondWindUsed = true;
            gs.playerHealth = std::min(gs.playerMaxHealth, gs.playerHealth + 28.0f);
            m_Stress = advClamp(m_Stress - 25.0f, 0.0f, 100.0f);
        }
    }

} // namespace bunker
