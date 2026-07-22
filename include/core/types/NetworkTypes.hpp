{
#pragma once
#include "core/Types.hpp"
#include <SFML/Network.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace bunker
{
    enum class NetRole
    {
        Offline,
        Host,   
        Client  
    };

    enum class PacketType : std::uint8_t
    {
        Handshake = 1,
        HandshakeAck,
        PlayerState,
        EnemyStateSync,
        ChatMessage,
        Disconnect
    };

    #pragma pack(push, 1)
    struct PlayerNetState
    {
        std::uint16_t peerId = 0;
        float x = 0, y = 0, z = 0;
        float vx = 0, vy = 0, vz = 0;
        float health = 100.0f;
        float facingAngle = 0.0f;
        std::uint8_t playerMode = 0; 
        std::uint32_t timestamp = 0;
    };
    #pragma pack(pop)

    struct RemotePlayer
    {
        int id = 0;
        std::string name = "Pilot";
        Vector3D position = {0,0,0};
        Vector3D velocity = {0,0,0};
        float health = 100.0f;
        float facingAngle = 0.0f;
        UnitMode mode = UnitMode::Scout;
        
        sf::IpAddress ip;
        unsigned short udpPort = 0;
        sf::TcpSocket* tcpSocket = nullptr; 
        float lastHeartbeat = 0.0f;
    };
}
}