#pragma once

#include "gameplay/advanced/AdvancedMechanicsCommon.hpp"
namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 2) RADIO / TAPE SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    struct TapeRecord
    {
        std::string id;
        std::string title;
        std::string transcript;
        bool found = false;
        bool played = false;
    };

    struct RadioMessage
    {
        float atTime = 0.0f;
        std::string channel;
        std::string text;
        bool fired = false;
    };

    class RadioTapeSystem
    {
      public:
        RadioTapeSystem();
        void update(float dt);
        void discoverTape(const std::string& id);
        std::string playNextUnplayed();

        const std::string& lastSubtitle() const
        {
            return m_LastSubtitle;
        }
        const std::deque<std::string>& log() const
        {
            return m_Log;
        }
        const std::vector<TapeRecord>& tapes() const
        {
            return m_Tapes;
        }

      private:
        float m_Time = 0.0f;
        std::string m_LastSubtitle;
        std::deque<std::string> m_Log;
        std::vector<TapeRecord> m_Tapes;
        std::vector<RadioMessage> m_Radio;
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 3) RATIONS / HEAL / RELOAD
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class RationKind
    {
        Protein,
        Stamina,
        Focus,
        AntiErosion
    };

    struct ActiveBuff
    {
        RationKind kind = RationKind::Protein;
        float timeLeft = 0.0f;
        float amount = 0.0f;
    };

    struct WeaponRuntime
    {
        int magazine = 30;
        int magazineMax = 30;
        int reserveAmmo = 120;
        float reloadTimer = 0.0f;
        bool isReloading = false;
    };

    class SurvivalSystem
    {
      public:
        static constexpr unsigned int ITEM_STIM = 303;
        static constexpr unsigned int ITEM_RATION_PROTEIN = 510;
        static constexpr unsigned int ITEM_RATION_STAMINA = 511;
        static constexpr unsigned int ITEM_RATION_FOCUS = 512;
        static constexpr unsigned int ITEM_RATION_ANTI_EROSION = 513;
        static constexpr unsigned int ITEM_AMMO_556 = 610;

        void update(GameState& gs, PlayerInventory& inv, float dt);
        bool useStim(GameState& gs, PlayerInventory& inv);
        bool eatRation(GameState& gs, PlayerInventory& inv, RationKind kind);
        void startReload(PlayerInventory& inv);
        bool consumeRound();

        void addStress(float amount)
        {
            m_Stress = advClamp(m_Stress + amount, 0.0f, 100.0f);
        }
        void calm(float amount)
        {
            m_Stress = advClamp(m_Stress - amount, 0.0f, 100.0f);
        }

        float stress() const
        {
            return m_Stress;
        }
        bool secondWindAvailable() const
        {
            return !m_SecondWindUsed;
        }
        bool soulLineAvailable() const
        {
            return !m_SoulLineUsed;
        }
        const WeaponRuntime& weapon() const
        {
            return m_Weapon;
        }
        const std::vector<ActiveBuff>& buffs() const
        {
            return m_Buffs;
        }

        float speedMultiplier() const;
        float aimPenalty() const;

      private:
        float m_Stress = 0.0f;
        bool m_SecondWindUsed = false;
        bool m_SoulLineUsed = false;
        WeaponRuntime m_Weapon;
        std::vector<ActiveBuff> m_Buffs;

        float reloadDuration() const;
        void updateStress(GameState& gs, float dt);
        void updateBuffs(GameState& gs, float dt);
        void updateReload(GameState& gs, float dt);
        void updateSecondWindAndSoulLine(GameState& gs);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 4) TANK UTILITIES / HANGAR / THERMAL LOAD / SEAT SWAP
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class TankUtilityMode
    {
        BucketRig,
        RamShield,
        TowCoupler
    };
    enum class TankSeat
    {
        Driver,
        Gunner
    };

    struct MuzzleSpark
    {
        Vector3D pos;
        Vector3D vel;
        float ttl = 0.15f;
    };

    struct TankUtilityRuntime
    {
        TankUtilityMode utility = TankUtilityMode::BucketRig;
        TankSeat seat = TankSeat::Driver;
        float utilityCooldown = 0.0f;
        float cannonThermalLoad = 0.0f; // 0..100
        bool overheated = false;
        bool towing = false;
        Vector3D towAnchor;
    };

    class TankUtilitySystem
    {
      public:
        static constexpr unsigned int ITEM_REPAIR_KIT = 401;

        void update(GameState& gs, float dt);
        void nextUtility();
        void swapSeat();
        bool useUtility(GameState& gs);
        bool registerCannonShot(GameState& gs, float heat);
        bool repairInHangar(GameState& gs, PlayerInventory& inv);

        bool isInsideHangar(const Vector3D& p) const
        {
            return p.x >= 1.5f && p.x <= 6.5f && p.y >= 1.5f && p.y <= 6.5f;
        }
        const TankUtilityRuntime& runtime() const
        {
            return m_Runtime;
        }
        const std::vector<MuzzleSpark>& sparks() const
        {
            return m_Sparks;
        }

      private:
        TankUtilityRuntime m_Runtime;
        std::vector<MuzzleSpark> m_Sparks;

        void carveFront(GameState& gs, int radius, int damage);
    };


} // namespace bunker
