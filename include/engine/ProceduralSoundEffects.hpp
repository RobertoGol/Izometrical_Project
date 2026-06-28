#pragma once

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <vector>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Полный исчерпывающий словарь всех 40 звуков Убежища 17 (Волна 7 / Wave 10)
    // Включает: природу, аномалии, шаги по 4 лужам, дыхание маски, гильзы и БТ
    // ═══════════════════════════════════════════════════════════════════════

    enum class SoundEffectType
    {
        // ── 1. Природа и Окружение Убежища 17 ──
        AmbBunkerVentilationLoop,
        AmbEtherGasHiss,
        AmbCaveWaterDrip,
        AmbAcidRainMetalRoof,
        AmbAshStormHowl,
        AmbDeepThunderRumble,
        AmbPipesCreakingMetal,

        // ── 2. Движение Пилота и Фоли ──
        PilotSuitBreathingMask,
        PilotHeartbeatSecondWind,
        FootstepConcrete1,
        FootstepConcrete2,
        FootstepWaterPuddle1,
        FootstepWaterPuddle2,
        FootstepMetalGrating1,
        FootstepMetalGrating2,
        FootstepSludgeErosion1,
        FoleyStimInjectorHiss,
        FoleyRationCanOpenClick,

        // ── 3. Оружие и Разрушения ──
        WeapXO16ShotSingle,
        WeapXO16ShotBurst,
        WeapScorchMortarLaunch,
        WeapThermiteBurnCrackle,
        WeapIonLaserHum,
        WeapCasingsClinkConcrete,
        WeapReloadHydraulicLatch,
        HitRicochetMetalWhine,
        GlassShatterSpike,
        CrateBreakThud,

        // ── 4. Танк Титан БТ-7274 ──
        TitanSteamHiss350Bar,
        CockpitSealClank,
        RamShieldHeavyClang,
        BucketRigScoopScrape,
        TowCouplerLatchClick,

        // ── 5. Рой RAY и Противники ──
        SwarmVerminChitterGroup,
        SwarmEggHatchSquelch,
        GhoulScreechAggro,
        RayDroneHoverHum,
        RayOpticSparkBlind,

        // ── 6. Терминалы и РобКо ──
        UiRobCoKeyClickAmber,
        UiAccessGrantedBeep
    };

    class ProceduralSoundEffects
    {
    private:
        static std::unordered_map<SoundEffectType, sf::SoundBuffer> s_Buffers;
        static sf::Sound s_SoundPlayer;

    public:
        static void initializeDefaults();
        static void play(SoundEffectType type, float volume = 100.0f);

    private:
        static sf::SoundBuffer makeSineSweepBuffer(float freqStart, float freqEnd, float durSec, float vol);
        static sf::SoundBuffer makeNoiseDecayBuffer(float durSec, float decayRate, float vol);
        static sf::SoundBuffer makeThudBuffer(float freq, float durSec, float vol);
        static sf::SoundBuffer makeBreathingBuffer(float durSec, float vol);
        static sf::SoundBuffer makeChitterBuffer(float durSec, float vol);
    };

} // namespace bunker
