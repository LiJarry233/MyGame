#pragma once

#include <string>
#include <cstdint>
#include "GameState.h"
#include "LevelConfig.h"
#include <enet/enet.h>

enum class NetworkMode {
    NONE,
    HOST,
    CLIENT
};

enum class NetworkPacketType : uint8_t {
    HOST_SNAPSHOT = 1,
    CLIENT_PADDLE = 2,
    INIT = 0x10,
    INIT_ACK = 0x11
};

#pragma pack(push, 1)
struct PowerUpData {
    float x, y;
    uint8_t type;
};

struct InitPacket {
    uint8_t type;
    uint8_t currentLevel;
    uint8_t brickCount;
    uint8_t brickTypes[66];
};

struct InitAckPacket {
    uint8_t type;
};

struct BallData {
    float x, y;
    float velX, velY;
};

struct HostSnapshotPacket {
    uint8_t type;
    uint8_t ballCount;
    BallData balls[8];
    float hostPaddleX;
    float hostPaddleY;
    float hostPaddleWidth;
    uint8_t brickCount;
    uint8_t brickDestroyed[66];
    uint8_t brickTypes[66];
    uint8_t powerupCount;
    PowerUpData powerups[10];
    int32_t score;
    int32_t lives;
    uint8_t currentLevel;
    uint8_t currentState;
    uint8_t ballLaunched;
    uint8_t loadingStatus;
    uint8_t backgroundLoaded;
};

struct ClientPaddlePacket {
    uint8_t type;
    float clientPaddleX;
    float clientPaddleY;
};
#pragma pack(pop)

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool InitializeHost(uint16_t port, GameState& state);
    bool InitializeClient(const std::string& hostName, uint16_t port, GameState& state);

    void SetLevelConfig(const LevelsConfig* cfg) { levelCfg = cfg; }

    void Service(GameState& state);
    void SendInit(const GameState& state);
    void SendHostSnapshot(const GameState& state);
    void SendClientPaddle(const GameState& state);

    bool IsConnected() const;
    bool IsClientInitialized() const { return initAckReceived; }

private:
    ENetHost* host;
    ENetPeer* peer;
    NetworkMode mode;
    bool connected;
    bool initAckReceived;
    const LevelsConfig* levelCfg = nullptr;

    void ProcessPacket(ENetPacket* packet, GameState& state);
    void SendInitAck();
    void CreateWallsFromConfig(GameState& state);
    void RebuildBricksFromConfig(GameState& state);
};
