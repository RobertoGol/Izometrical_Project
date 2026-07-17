#include "engine/ProceduralSoundEffects.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    // ── 2. СУЩЕСТВА ROY / RAY ──
    sf::SoundBuffer SoundEffectsBank::makeRoyStep()
    {
        int len = 400;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 320.0f * t) * 18000.0f *
                                                   (1.0f - static_cast<float>(i) / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeRoyReplication()
    {
        int len = 1800;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            samples[i] = ((i / 40) % 2 == 0) ? 22000 : -22000;
            samples[i] = static_cast<std::int16_t>(samples[i] * (1.0f - static_cast<float>(i) / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeRoySpark()
    {
        int len = 600;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            samples[i] = static_cast<std::int16_t>((((std::rand() % 100) > 80) ? 20000 : -5000) * (len - i) / len);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeRayHover()
    {
        int len = 2205;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 810.0f * t) * 12000.0f);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    // ── 3. ХОДОВАЯ ТЕХНИКА ──
    sf::SoundBuffer SoundEffectsBank::makeWalkerLegs()
    {
        int len = 450;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 280.0f * t) * 22000.0f *
                                                   (1.0f - static_cast<float>(i) / len));
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeTracksGrind()
    {
        int len = 3000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            samples[i] = static_cast<std::int16_t>(((std::rand() % 200) - 100) * 180.0f);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }

    sf::SoundBuffer SoundEffectsBank::makeWheelsHum()
    {
        int len = 2000;
        std::vector<std::int16_t> samples(len);
        for (int i = 0; i < len; ++i)
        {
            float t = static_cast<float>(i) / 11025;
            samples[i] = static_cast<std::int16_t>(std::sin(2.0f * 3.14159f * 180.0f * t) * 14000.0f);
        }
        sf::SoundBuffer b;
        (void)b.loadFromSamples(samples.data(), static_cast<std::uint64_t>(samples.size()), 1, 11025,
                                {sf::SoundChannel::Mono});
        return b;
    }


} // namespace bunker
