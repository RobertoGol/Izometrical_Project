#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <string>

namespace bunker
{

    enum class VoiceEvent
    {
        PipBoyBoot,
        PipPadBoot,
        TitanActivation
    };

    // ── Языковые пакеты озвучки и титров (Волна 7 / Settings Localization) ──
    enum class VoiceLanguage
    {
        EnglishUS, // 1. Английская (США)
        Russian,   // 2. Русская (Россия)
        German,    // 3. Немецкая
        EnglishAU, // 4. Английская с австралийским акцентом (не сельским)
        EnglishGB  // 5. Английская (Британия)
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Звуковой контроллер движка и локализованных субтитров (Волна 7)
    // Поддерживает переключение языка озвучки и языка субтитров из меню!
    // ═══════════════════════════════════════════════════════════════════════

    class AudioController
    {
    private:
        sf::Music m_VoiceStream;
        std::string m_CurrentSubtitle = "";
        float m_SubtitleTimer = 0.0f;
        bool m_IsMuted = false;
        VoiceLanguage m_VoiceLang = VoiceLanguage::Russian;
        VoiceLanguage m_SubLang = VoiceLanguage::Russian;

    public:
        AudioController() = default;

        bool playVoiceEvent(VoiceEvent ev);
        void update(float dt);
        void renderSubtitlesHUD(sf::RenderWindow &window, const sf::Font *font) const;

        void setVoiceLanguage(VoiceLanguage l) { m_VoiceLang = l; }
        void setSubtitleLanguage(VoiceLanguage l) { m_SubLang = l; }
        VoiceLanguage voiceLanguage() const { return m_VoiceLang; }
        VoiceLanguage subtitleLanguage() const { return m_SubLang; }

        void setMuted(bool mute)
        {
            m_IsMuted = mute;
            if (mute)
                m_VoiceStream.stop();
        }
        void toggleMute() { setMuted(!m_IsMuted); }
        bool isMuted() const { return m_IsMuted; }
        const std::string &currentSubtitle() const { return m_CurrentSubtitle; }

    private:
        std::string getAssetPath(VoiceEvent ev) const;
        std::string getSubtitleText(VoiceEvent ev) const;
    };

} // namespace bunker
