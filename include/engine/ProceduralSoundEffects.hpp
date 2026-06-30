#pragma once

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <vector>
#include <string>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Полный неразреженный звуковой банк движка (SFX Omniverse Bank v.114)
    // Строго без сокращений: все 33 класса звуков имеют свои PCM алгоритмы!
    // ROY / RAY — строго механические существа. Нулевая нагрузка на ЦП.
    // ═══════════════════════════════════════════════════════════════════════

    enum class SoundEffectType
    {
        // ── 1. ПИЛОТ, ШАГИ И БЫТОВЫЕ ФОЛИ ──
        FootstepConcrete,
        FootstepWaterPuddle,
        FootstepMetalGrating,
        FootstepSludge,
        PilotSuitBreathing,
        PilotHeartbeatCrit,
        StimInjectorClick,
        RationCanOpen,

        // ── 2. МЕХАНИЧЕСКИЕ СУЩЕСТВА ROY И ДРОНЫ RAY ──
        RoyMechStep,
        RoyReplicationClank,
        RoyOpticSparkFailure,
        RayDroneHoverHum,

        // ── 3. ХОДОВАЯ ТЕХНИКА (Ноги, Гусеницы, Колёса) ──
        MechWalkerLegsShuffle,
        MechTracksGrindLoop,
        MechWheelsRollHum,

        // ── 4. ОРУЖИЕ, БАЛЛИСТИКА И РУКОПАШКА ──
        WeapXO16Shot,
        WeapShotgunRifleBlast,
        WeapScorchMortarLaunch,
        WeapGrenadeBounce,
        WeapReloadHydraulic,
        CasingsBounceConcrete,
        MeleeSlashingBlade,
        MeleePiercingThrust,

        // ── 5. ЗАХВАТЫВАЮЩАЯ МАГИЯ АНИМЕ ВЫСОКОГО УРОВНЯ ──
        MagicAnimeCastHype,

        // ── 6. ТАНК БТ-7274, ЭЛЕКТРИЧЕСТВО И ПРИРОДА ──
        TitanSteamVenting350Bar,
        TitanCockpitSealClank,
        TitanRamShieldClang,
        ElecGeneratorStartHum,
        ElecTransformerBuzz,
        NatAcidRainPatter,
        NatWindExteriorHowl,

        // ── 7. ИНТЕРФЕЙС РОБКО ──
        UiTerminalKeyClick,
        UiAccessGrantedBeep
    };

    class SoundEffectsBank
    {
    private:
        static std::unordered_map<SoundEffectType, sf::SoundBuffer> s_Buffers;
        static sf::Sound s_SoundPlayer;

    public:
        static void initializeFast();
        static void play(SoundEffectType type, float volume = 100.0f);

    private:
        static sf::SoundBuffer makeConcreteStep();
        static sf::SoundBuffer makeWaterStep();
        static sf::SoundBuffer makeGratingStep();
        static sf::SoundBuffer makeSludgeStep();
        static sf::SoundBuffer makeBreathing();
        static sf::SoundBuffer makeHeartbeat();
        static sf::SoundBuffer makeStimClick();
        static sf::SoundBuffer makeRationClick();

        static sf::SoundBuffer makeRoyStep();
        static sf::SoundBuffer makeRoyReplication();
        static sf::SoundBuffer makeRoySpark();
        static sf::SoundBuffer makeRayHover();

        static sf::SoundBuffer makeWalkerLegs();
        static sf::SoundBuffer makeTracksGrind();
        static sf::SoundBuffer makeWheelsHum();

        static sf::SoundBuffer makeXO16();
        static sf::SoundBuffer makeShotgun();
        static sf::SoundBuffer makeMortar();
        static sf::SoundBuffer makeGrenade();
        static sf::SoundBuffer makeReload();
        static sf::SoundBuffer makeCasings();
        static sf::SoundBuffer makeSlashing();
        static sf::SoundBuffer makePiercing();

        static sf::SoundBuffer makeAnimeMagicMusic();

        static sf::SoundBuffer makeTitanSteam();
        static sf::SoundBuffer makeCockpitSeal();
        static sf::SoundBuffer makeRamShield();
        static sf::SoundBuffer makeGeneratorStart();
        static sf::SoundBuffer makeTransformerBuzz();
        static sf::SoundBuffer makeAcidRain();
        static sf::SoundBuffer makeWind();

        static sf::SoundBuffer makeKeyClick();
        static sf::SoundBuffer makeAccessBeep();
    };

} // namespace bunker