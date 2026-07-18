#include "gameplay/AdvancedMechanics.hpp"

#include <algorithm>
#include <cstdlib>

namespace bunker
{

    int LanlineServices::createLocalLobby(const std::string& playerName)
    {
        m_Peers.clear();
        m_Deliveries.clear();
        m_Chat.clear();
        m_Connected = true;
        m_LobbyId = ++m_NextLobbyId;
        addPeerWithSocket(playerName.empty() ? "Solo_Pilot" : playerName, {10.0f, 10.0f, 0.0f}, ++m_NextSocketFd);
        addPeerWithSocket("Gunner_BT7274", {11.0f, 10.0f, 0.0f}, ++m_NextSocketFd);
        addPeerWithSocket("Scout_LogHorizon", {12.0f, 12.0f, 0.0f}, ++m_NextSocketFd);
        addPeerWithSocket("Vault17_Quartermaster", {15.0f, 8.0f, 0.0f}, ++m_NextSocketFd);

        const char* coopNames[] = {"Ranger_Kodiak", "Tech_Valerie",   "Heavy_Goliath",  "Medic_Mercy",
                                   "Sniper_Ghost",  "Engineer_Spark", "Recon_Viper",    "Trooper_Blaze",
                                   "Sapper_Boom",   "Sentinel_Apex",  "Commando_Rex",   "Guardian_Shield",
                                   "Warden_Frost",  "Striker_Bolt",   "Vanguard_Storm", "Overseer_Vault17"};
        for (int i = 0; i < 16 && static_cast<int>(m_Peers.size()) < MAX_COOP_PLAYERS; ++i)
        {
            const float px = 8.0f + static_cast<float>((i * 5) % 40);
            const float py = 8.0f + static_cast<float>((i * 7) % 35);
            addPeerWithSocket(coopNames[i], {px, py, 0.0f}, ++m_NextSocketFd);
        }

        systemMessage("LANLINE Winsock net #1001 connected. " + std::to_string(m_Peers.size()) +
                      " squad combatants online.");
        return m_LobbyId;
    }

    int LanlineServices::addPeer(const std::string& name)
    {
        return addPeerWithSocket(name, {10.0f, 10.0f, 0.0f}, ++m_NextSocketFd);
    }

    int LanlineServices::addPeerWithSocket(const std::string& name, Vector3D pos, unsigned int sockFd)
    {
        LanlinePeer p;
        p.id = ++m_NextPeerId;
        p.name = name;
        p.lastKnownPos = pos;
        p.simulatedSocketFd = sockFd;
        p.inInterestArea = true;
        m_Peers.push_back(p);
        return p.id;
    }

    void LanlineServices::cullInactiveOrDistantPeers(const Vector3D& localPlayerPos, float interestRadius)
    {
        const float rSq = interestRadius * interestRadius;
        for (auto& p : m_Peers)
        {
            const float dx = p.lastKnownPos.x - localPlayerPos.x;
            const float dy = p.lastKnownPos.y - localPlayerPos.y;
            p.inInterestArea = (dx * dx + dy * dy <= rSq);
        }
    }

    void LanlineServices::simulateWinsockUdpHeartbeat(float dt)
    {
        m_HeartbeatTimer += dt;
        if (m_HeartbeatTimer >= 1.0f)
        {
            m_HeartbeatTimer = 0.0f;
            for (auto& p : m_Peers)
            {
                if (p.simulatedSocketFd > 0)
                {
                    p.lastKnownPos.x += (std::rand() % 3 - 1) * 0.1f;
                    p.lastKnownPos.y += (std::rand() % 3 - 1) * 0.1f;
                }
            }
        }
    }

    void LanlineServices::sendChat(int fromPeer, const std::string& text)
    {
        if (!m_Connected)
        {
            return;
        }
        m_Chat.push_back({fromPeer, text, 10.0f});
        if (m_Chat.size() > 20)
        {
            m_Chat.erase(m_Chat.begin());
        }
    }

    void LanlineServices::setVoice(int peerId, bool active)
    {
        for (auto& p : m_Peers)
        {
            if (p.id == peerId)
            {
                p.voiceActive = active;
            }
        }
    }

    int LanlineServices::requestDelivery(const std::string& payload, Vector3D dropPos)
    {
        LanlineDelivery d;
        d.id = ++m_NextDeliveryId;
        d.payload = payload;
        d.dropPos = dropPos;
        d.eta = 8.0f + static_cast<float>((m_NextDeliveryId * 7) % 6);
        m_Deliveries.push_back(d);
        return d.id;
    }

    void LanlineServices::update(GameState& gs, PlayerInventory& inv, float dt)
    {
        simulateWinsockUdpHeartbeat(dt);
        cullInactiveOrDistantPeers(gs.playerPos, 40.0f);

        for (auto& c : m_Chat)
        {
            c.ttl -= dt;
        }
        m_Chat.erase(
            std::remove_if(m_Chat.begin(), m_Chat.end(), [](const LanlineChatMessage& c) { return c.ttl <= 0.0f; }),
            m_Chat.end());

        for (auto& d : m_Deliveries)
        {
            if (d.delivered)
            {
                continue;
            }
            d.eta -= dt;
            if (d.eta <= 0.0f)
            {
                d.delivered = true;
                if (d.payload == "ammo")
                {
                    inv.addItem(SurvivalSystem::ITEM_AMMO_556, ItemType::Ammo, 2, 0.15f, "5.56 AMMO BOX");
                }
                else if (d.payload == "med")
                {
                    inv.addItem(SurvivalSystem::ITEM_STIM, ItemType::Medicine, 1, 0.10f, "STIM INJECTOR");
                }
                else
                {
                    inv.addItem(201, ItemType::Resource, 10, 0.08f, "SCRAP METAL");
                }
                gs.score += 25;
                systemMessage("Delivery arrived: " + d.payload);
            }
        }

        static float s_ChatTimer = 15.0f;
        s_ChatTimer -= dt;
        if (s_ChatTimer <= 0.0f && m_Peers.size() > 1)
        {
            s_ChatTimer = 30.0f;
            sendChat(m_Peers[1].id, "Сенсоры бастиона в норме. Готов прикрыть огнем.");
        }
    }

} // namespace bunker
