#pragma once

#include "gameplay/advanced/AdvancedMechanicsCommon.hpp"
namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 12) OBJ MODEL LOADER
    // ═══════════════════════════════════════════════════════════════════════════════

    struct ObjVertex
    {
        float x = 0, y = 0, z = 0;
    };
    struct ObjTexCoord
    {
        float u = 0, v = 0;
    };
    struct ObjNormal
    {
        float nx = 0, ny = 0, nz = 0;
    };
    struct ObjFace
    {
        std::array<int, 3> v{{0, 0, 0}};
        std::array<int, 3> vt{{0, 0, 0}};
        std::array<int, 3> vn{{0, 0, 0}};
    };

    struct ObjModel
    {
        std::vector<ObjVertex> vertices;
        std::vector<ObjTexCoord> texCoords;
        std::vector<ObjNormal> normals;
        std::vector<ObjFace> faces;
        bool empty() const
        {
            return vertices.empty() || faces.empty();
        }
        void computeFlatNormalsIfMissing();
    };

    class ObjModelLoader
    {
      public:
        static ObjModel load(const std::string& path);

      private:
        static int parseFaceIndex(const std::string& token);
        static void parseFullFaceTriplet(const std::string& token, int& v_idx, int& vt_idx, int& vn_idx);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 13) LANLINE SERVICES: lobby/chat/voice/delivery/local squad model
    // ═══════════════════════════════════════════════════════════════════════════════

    struct LanlinePeer
    {
        int id = 0;
        std::string name;
        bool voiceActive = false;
        Vector3D lastKnownPos;
        bool inInterestArea = true;
        unsigned int simulatedSocketFd = 0;
    };

    struct LanlineChatMessage
    {
        int fromPeer = 0;
        std::string text;
        float ttl = 10.0f;
    };

    struct LanlineDelivery
    {
        int id = 0;
        std::string payload;
        Vector3D dropPos;
        float eta = 10.0f;
        bool delivered = false;
    };

    class LanlineServices
    {
      public:
        static constexpr int MAX_COOP_PLAYERS = 20;

        int createLocalLobby(const std::string& playerName);
        int addPeer(const std::string& name);
        int addPeerWithSocket(const std::string& name, Vector3D pos, unsigned int sockFd);
        void sendChat(int fromPeer, const std::string& text);
        void setVoice(int peerId, bool active);
        int requestDelivery(const std::string& payload, Vector3D dropPos);
        void update(GameState& gs, PlayerInventory& inv, float dt);

        void cullInactiveOrDistantPeers(const Vector3D& localPlayerPos, float interestRadius);
        void simulateWinsockUdpHeartbeat(float dt);

        bool connected() const
        {
            return m_Connected;
        }
        const std::vector<LanlinePeer>& peers() const
        {
            return m_Peers;
        }
        const std::vector<LanlineChatMessage>& chat() const
        {
            return m_Chat;
        }
        const std::vector<LanlineDelivery>& deliveries() const
        {
            return m_Deliveries;
        }

      private:
        bool m_Connected = false;
        int m_LobbyId = 0;
        int m_NextLobbyId = 1000;
        int m_NextPeerId = 0;
        int m_NextDeliveryId = 0;
        unsigned int m_NextSocketFd = 100042;
        float m_HeartbeatTimer = 0.0f;
        std::vector<LanlinePeer> m_Peers;
        std::vector<LanlineChatMessage> m_Chat;
        std::vector<LanlineDelivery> m_Deliveries;

        void systemMessage(const std::string& text)
        {
            m_Chat.push_back({0, text, 8.0f});
        }
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 14) PROFILE / SESSION MIGRATION
    // ═══════════════════════════════════════════════════════════════════════════════

    struct PlayerProfile
    {
        unsigned int version = 16;
        std::string playerName = "Pilot";
        int totalKills = 0;
        int totalDeaths = 0;
        int sessionsPlayed = 0;
        float totalPlayTime = 0.0f;
        std::map<std::string, int> flags;
    };

    class ProfileSessionSystem
    {
      public:
        void startSession(PlayerProfile& p);
        void update(PlayerProfile& p, float dt);
        void registerKill(PlayerProfile& p)
        {
            p.totalKills += 1;
        }
        void registerDeath(PlayerProfile& p)
        {
            p.totalDeaths += 1;
        }
        void migrate(PlayerProfile& p);
        std::string summary(const PlayerProfile& p) const;

      private:
        bool m_Started = false;
        float m_SessionTime = 0.0f;
    };


} // namespace bunker
