#include "engine/ProceduralSoundEffects.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    // ── 6. ТАНК БТ И ОКРУЖЕНИЕ ──
    sf::SoundBuffer SoundEffectsBank::makeTitanSteam()
    {
        int len = 5000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
            samples[i] =
                static_cast<std::int16_t>(((std::rand() % 200) - 100) * 160.0f * (1.0f - static_cast<float>(i) / len));
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeCockpitSeal()
    {
        int len = 2000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] =
                static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 110.0f * t) * 25000.0f * std::exp(-5.0f * t));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeRamShield()
    {
        int len = 2500;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] =
                static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 450.0f * t) * 29000.0f * std::exp(-4.0f * t));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeGeneratorStart()
    {
        int len = 3500;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            float rev = 25.0f + 35.0f * (static_cast<float>(i) / len);
            samples[i] =
                static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * rev * t) * 24000.0f * std::min(1.0f, t * 3.0f));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeTransformerBuzz()
    {
        int len = 2000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 60.0f * t) * 15000.0f);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeAcidRain()
    {
        int len = 3000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 90.0f);
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeWind()
    {
        int len = 6000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 110.0f * std::sin(3.14159f * i / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    // ── 7. РОБКО UI ──
    sf::SoundBuffer SoundEffectsBank::makeKeyClick()
    {
        int len = 250;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
            samples[i] =
                static_cast<std::int16_t>(((std::rand() % 200) - 100) * 150.0f * (1.0f - static_cast<float>(i) / len));
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeAccessBeep()
    {
        int len = 600;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 1800.0f * t) * 20000.0f);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }


} // namespace bunker
