#pragma once

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <vector>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Исчерпывающий звуковой банк движка (SFX Omniverse Bank v.115)
    // Учтены абсолютно все классы: ROY механические, шагоходы, рукопашка, аниме-магия!
    // ═══════════════════════════════════════════════════════════════════════

    enum class SoundEffectType
    {
        // ── 1. ШАГИ ПИЛОТА И ФОЛИ ──
        FootstepConcrete,
        FootstepWaterPuddle,
        FootstepMetalGrating,
        FootstepSludge,
        PilotSuitBreathing,
        PilotHeartbeatCrit,
        StimInjectorClick,
        RationCanOpen,

        // ── 2. МЕХАНИЧЕСКИЕ СУЩЕСТВА ROY И ДРОНЫ RAY ──
        RoyMechStep,          // Механическое цоканье робота ROY
        RoyReplicationClank,  // Лязг самосборки дрона ROY
        RoyOpticSparkFailure, // Короткое замыкание оптики при ранении
        RayDroneHoverHum,     // Электрический гул левитации RAY

        // ── 3. ХОДОВАЯ ТЕХНИКА (Ноги, Гусеницы, Колёса) ──
        MechWalkerLegsShuffle, // Быстрый перебор шагоходовых ног роботов-пауков
        MechTracksGrindLoop,   // Тяжёлый металлический лязг гусениц
        MechWheelsRollHum,     // Шум катящихся колёс паровых машин

        // ── 4. БАЛЛИСТИКА И РУКОПАШНЫЙ БОЙ (Холодное оружие) ──
        WeapXO16Shot,
        WeapShotgunRifleBlast, // Выстрел крупнокалиберного ружья
        WeapScorchMortarLaunch,
        WeapGrenadeBounce, // Металлический отскок гранаты о бетон
        WeapReloadHydraulic,
        CasingsBounceConcrete,
        MeleeSlashingBlade,  // Свист режущего взмаха клинка / катаны
        MeleePiercingThrust, // Пробивающий колющий удар клинка в броню

        // ── 5. ЗАЖИГАЮЩАЯ МАГИЯ ВЫСОКОГО УРОВНЯ (Anime Hype Magic) ──
        MagicAnimeCastHype, // Восходящий захватывающий перелив заряда магии!

        // ── 6. ТАНК БТ-7274 И ОКРУЖЕНИЕ ──
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
        // Полный физический спектр генераторов без единого удаления логики
        static sf::SoundBuffer makeFastPulse(int lengthSamples, sf::Int16 amp);
        static sf::SoundBuffer makeFastNoise(int lengthSamples, sf::Int16 amp);
        static sf::SoundBuffer makeSineSweepFast(float fStart, float fEnd, float durSec, float vol);
        static sf::SoundBuffer makeNoiseDecayBuffer(float durSec, float decayRate, float vol);
        static sf::SoundBuffer makeThudBuffer(float freq, float durSec, float vol);
        static sf::SoundBuffer makeBreathingBuffer(float durSec, float vol);
        static sf::SoundBuffer makeChitterBuffer(float durSec, float vol);
        static sf::SoundBuffer makeGeneratorRevBuffer(float durSec, float vol);
        static sf::SoundBuffer makeSparkBuffer(float durSec, float vol);
        static sf::SoundBuffer makeAnimeMagicMusicFast();
    };

} // namespace bunker
