#include "NetworkManager.h"
#include <cstring>
#include "raylib.h"
#include "GameState.h"

NetworkManager::NetworkManager()
    : host(nullptr), peer(nullptr), mode(NetworkMode::NONE),
      connected(false), initAckReceived(false) {
    if (enet_initialize() != 0) {
        TraceLog(LOG_ERROR, "ENet initialization failed.");
    } else {
        atexit(enet_deinitialize);
    }
}

NetworkManager::~NetworkManager() {
    if (peer) {
        enet_peer_disconnect(peer, 0);
    }
    if (host) {
        enet_host_destroy(host);
    }
}

bool NetworkManager::InitializeHost(uint16_t port, GameState& state) {
    if (host) return false;

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, 2, 2, 0, 0);
    if (!host) {
        TraceLog(LOG_ERROR, "Failed to create ENet host.");
        return false;
    }

    mode = NetworkMode::HOST;
    state.networkHost = true;
    state.networkClient = false;
    state.networkConnected = false;
    return true;
}

bool NetworkManager::InitializeClient(const std::string& hostName, uint16_t port, GameState& state) {
    state.networkConnected = false;
    if (host) return false;

    host = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!host) {
        TraceLog(LOG_ERROR, "Failed to create ENet client host.");
        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, hostName.c_str());
    address.port = port;

    peer = enet_host_connect(host, &address, 2, 0);
    if (!peer) {
        TraceLog(LOG_ERROR, "Failed to create ENet connection to host.");
        return false;
    }

    mode = NetworkMode::CLIENT;
    state.networkClient = true;
    state.networkHost = false;
    return true;
}

void NetworkManager::Service(GameState& state) {
    if (!host) return;

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            peer = event.peer;
            connected = true;
            state.networkConnected = true;
            TraceLog(LOG_INFO, "ENet peer connected.");
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            if (!connected) {
                connected = true;
                state.networkConnected = true;
                TraceLog(LOG_INFO, "ENet peer connected (via data packet).");
            }
            ProcessPacket(event.packet, state);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            TraceLog(LOG_WARNING, "ENet peer disconnected.");
            connected = false;
            state.networkConnected = false;
            initAckReceived = false;
            peer = nullptr;
            break;

        default:
            break;
        }
    }
}

void NetworkManager::SendInit(const GameState& state) {
    if (!connected || !peer) return;

    InitPacket packetData;
    memset(&packetData, 0, sizeof(packetData));
    packetData.type = static_cast<uint8_t>(NetworkPacketType::INIT);
    packetData.currentLevel = static_cast<uint8_t>(state.currentLevel);
    packetData.brickCount = static_cast<uint8_t>(std::min((size_t)66, state.bricks.size()));

    for (size_t i = 0; i < packetData.brickCount; ++i) {
        packetData.brickTypes[i] = static_cast<uint8_t>(state.bricks[i].GetType());
    }

    ENetPacket* packet = enet_packet_create(&packetData, sizeof(packetData), 0);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}

void NetworkManager::SendInitAck() {
    if (!connected || !peer) return;

    InitAckPacket packetData;
    packetData.type = static_cast<uint8_t>(NetworkPacketType::INIT_ACK);

    ENetPacket* packet = enet_packet_create(&packetData, sizeof(packetData), 0);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}

void NetworkManager::SendHostSnapshot(const GameState& state) {
    if (!connected || !peer) return;

    HostSnapshotPacket packetData;
    memset(&packetData, 0, sizeof(packetData));
    packetData.type = static_cast<uint8_t>(NetworkPacketType::HOST_SNAPSHOT);

    packetData.ballCount = static_cast<uint8_t>(std::min((size_t)8, state.balls.size()));
    for (uint8_t i = 0; i < packetData.ballCount; ++i) {
        packetData.balls[i].x = state.balls[i].GetPosition().x;
        packetData.balls[i].y = state.balls[i].GetPosition().y;
        packetData.balls[i].velX = state.balls[i].GetVelocity().x;
        packetData.balls[i].velY = state.balls[i].GetVelocity().y;
    }

    packetData.hostPaddleX = state.paddle.GetRect().x;
    packetData.hostPaddleY = state.paddle.GetRect().y;
    packetData.hostPaddleWidth = state.paddle.GetWidth();

    packetData.brickCount = static_cast<uint8_t>(std::min((size_t)66, state.bricks.size()));
    for (size_t i = 0; i < packetData.brickCount; ++i) {
        packetData.brickDestroyed[i] = state.bricks[i].IsDestroyed() ? 1 : 0;
        packetData.brickTypes[i] = static_cast<uint8_t>(state.bricks[i].GetType());
    }

    packetData.powerupCount = static_cast<uint8_t>(std::min((size_t)10, state.powerups.size()));
    for (size_t i = 0; i < packetData.powerupCount; ++i) {
        packetData.powerups[i].x = state.powerups[i].GetPosition().x;
        packetData.powerups[i].y = state.powerups[i].GetPosition().y;
        packetData.powerups[i].type = static_cast<uint8_t>(state.powerups[i].GetType());
    }

    packetData.score = state.score;
    packetData.lives = state.lives;
    packetData.currentLevel = static_cast<uint8_t>(state.currentLevel);
    packetData.currentState = static_cast<uint8_t>(state.currentState);
    packetData.ballLaunched = state.ballLaunched ? 1 : 0;
    {
        std::lock_guard<std::mutex> lock(state.loadingMutex);
        packetData.loadingStatus = static_cast<uint8_t>(state.loadingStatus);
        packetData.backgroundLoaded = state.backgroundLoaded ? 1 : 0;
    }

    ENetPacket* packet = enet_packet_create(&packetData, sizeof(packetData), 0);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}

void NetworkManager::SendClientPaddle(const GameState& state) {
    if (!connected || !peer) return;

    ClientPaddlePacket packetData;
    packetData.type = static_cast<uint8_t>(NetworkPacketType::CLIENT_PADDLE);
    packetData.clientPaddleX = state.paddle.GetRect().x;
    packetData.clientPaddleY = state.paddle.GetRect().y;

    ENetPacket* packet = enet_packet_create(&packetData, sizeof(packetData), 0);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}

bool NetworkManager::IsConnected() const {
    return connected;
}

void NetworkManager::ProcessPacket(ENetPacket* packet, GameState& state) {
    if (!packet || packet->dataLength < 1) return;

    uint8_t packetType = packet->data[0];

    if (packetType == static_cast<uint8_t>(NetworkPacketType::CLIENT_PADDLE)) {
        if (packet->dataLength < sizeof(ClientPaddlePacket)) return;
        ClientPaddlePacket packetData;
        std::memcpy(&packetData, packet->data, sizeof(packetData));
        state.remotePaddle.SetPosition({packetData.clientPaddleX, packetData.clientPaddleY});
    }
    else if (packetType == static_cast<uint8_t>(NetworkPacketType::INIT_ACK)) {
        initAckReceived = true;
        TraceLog(LOG_INFO, "Received INIT_ACK from client. Starting snapshots.");
    }
    else if (packetType == static_cast<uint8_t>(NetworkPacketType::INIT)) {
        if (packet->dataLength < sizeof(InitPacket)) return;
        InitPacket packetData;
        std::memcpy(&packetData, packet->data, sizeof(packetData));

        state.currentLevel = packetData.currentLevel;
        state.clientInitialized = true;

        // Rebuild bricks at deterministic positions for this level
        // We need to set level before calling ResetBricksPlain externally
        // For now, mark as initialized - Game.h will handle brick creation
        // Apply brick types after bricks are created
        TraceLog(LOG_INFO, "Received INIT: level=%d, bricks=%d", packetData.currentLevel, packetData.brickCount);

        // Store brick types temporarily - they'll be applied after bricks are created
        // Since bricks may not exist yet, we trigger rebuild in the state
        state.bricks.clear();

        const int BASE_ROWS = 5;
        const int BASE_COLS = 8;
        const float BASE_WIDTH = 80.0f;
        const float BASE_HEIGHT = 25.0f;

        if (state.currentLevel == 2 || state.currentLevel == 3) {
            state.rows = BASE_ROWS + 1;
            state.cols = BASE_COLS + 3;
            state.brickWidth = BASE_WIDTH * 0.8f;
            state.brickHeight = BASE_HEIGHT * 0.8f;
        } else {
            state.rows = BASE_ROWS;
            state.cols = BASE_COLS;
            state.brickWidth = BASE_WIDTH;
            state.brickHeight = BASE_HEIGHT;
        }

        int screenWidth = 800;
        for (int r = 0; r < state.rows; r++) {
            for (int c = 0; c < state.cols; c++) {
                float totalWidth = state.cols * state.brickWidth;
                float totalGapWidth = (state.cols - 1) * 10.0f;
                float availableWidth = screenWidth - 80.0f;
                float gap = 10.0f;
                if (totalWidth + totalGapWidth > availableWidth) {
                    gap = (availableWidth - totalWidth) / (state.cols - 1);
                    if (gap < 1.0f) gap = 1.0f;
                }
                float x = 40.0f + c * (state.brickWidth + gap);
                float y = 80.0f + r * (state.brickHeight + 5.0f);
                state.bricks.emplace_back(x, y, state.brickWidth, state.brickHeight, NORMAL);
            }
        }

        // Apply brick types from INIT packet
        for (size_t i = 0; i < packetData.brickCount && i < state.bricks.size(); ++i) {
            state.bricks[i].SetType(static_cast<BrickType>(packetData.brickTypes[i]));
        }

        // Create a ball for rendering
        if (state.balls.empty()) {
            state.balls.emplace_back(Vector2{400, 300}, 4.0f, 90.0f, state.ballRadius);
        }

        SendInitAck();
    }
    else if (packetType == static_cast<uint8_t>(NetworkPacketType::HOST_SNAPSHOT)) {
        if (packet->dataLength < sizeof(HostSnapshotPacket)) return;
        HostSnapshotPacket packetData;
        std::memcpy(&packetData, packet->data, sizeof(packetData));

        // Sync balls: resize to match HOST's ball count
        while (state.balls.size() > packetData.ballCount) {
            state.balls.pop_back();
        }
        while (state.balls.size() < packetData.ballCount) {
            state.balls.emplace_back(Vector2{400, 300}, 4.0f, 90.0f, state.ballRadius);
        }
        for (uint8_t i = 0; i < packetData.ballCount; ++i) {
            state.balls[i].SetPosition({packetData.balls[i].x, packetData.balls[i].y});
            state.balls[i].SetVelocity({packetData.balls[i].velX, packetData.balls[i].velY});
            state.balls[i].UpdateTrail();
        }

        // Sync host paddle position and width
        state.remotePaddle.SetPosition({packetData.hostPaddleX, packetData.hostPaddleY});
        state.remotePaddle.SetWidth(packetData.hostPaddleWidth);

        state.score = packetData.score;
        state.lives = packetData.lives;
        state.currentState = static_cast<GameStatus>(packetData.currentState);
        state.ballLaunched = packetData.ballLaunched != 0;

        state.loadingStatus = static_cast<LoadingStatus>(packetData.loadingStatus);
        state.backgroundLoaded = packetData.backgroundLoaded != 0;

        // Detect level change - rebuild bricks
        if (packetData.currentLevel != state.currentLevel) {
            state.currentLevel = packetData.currentLevel;
            state.bricks.clear();

            const int BASE_ROWS = 5;
            const int BASE_COLS = 8;
            const float BASE_WIDTH = 80.0f;
            const float BASE_HEIGHT = 25.0f;

            if (state.currentLevel == 2 || state.currentLevel == 3) {
                state.rows = BASE_ROWS + 1;
                state.cols = BASE_COLS + 3;
                state.brickWidth = BASE_WIDTH * 0.8f;
                state.brickHeight = BASE_HEIGHT * 0.8f;
            } else {
                state.rows = BASE_ROWS;
                state.cols = BASE_COLS;
                state.brickWidth = BASE_WIDTH;
                state.brickHeight = BASE_HEIGHT;
            }

            int screenWidth = 800;
            for (int r = 0; r < state.rows; r++) {
                for (int c = 0; c < state.cols; c++) {
                    float totalWidth = state.cols * state.brickWidth;
                    float totalGapWidth = (state.cols - 1) * 10.0f;
                    float availableWidth = screenWidth - 80.0f;
                    float gap = 10.0f;
                    if (totalWidth + totalGapWidth > availableWidth) {
                        gap = (availableWidth - totalWidth) / (state.cols - 1);
                        if (gap < 1.0f) gap = 1.0f;
                    }
                    float x = 40.0f + c * (state.brickWidth + gap);
                    float y = 80.0f + r * (state.brickHeight + 5.0f);
                    state.bricks.emplace_back(x, y, state.brickWidth, state.brickHeight, NORMAL);
                }
            }
        }

        // Sync brick states and types
        for (size_t i = 0; i < packetData.brickCount && i < state.bricks.size(); ++i) {
            // Generate particles when a brick is newly destroyed
            if (packetData.brickDestroyed[i] && !state.bricks[i].IsDestroyed()) {
                Rectangle rect = state.bricks[i].GetRect();
                Vector2 center = {rect.x + rect.width / 2, rect.y + rect.height / 2};
                for (int p = 0; p < 20; p++) {
                    state.particles.emplace_back(center);
                }
            }
            state.bricks[i].SetDestroyed(packetData.brickDestroyed[i] != 0);
            state.bricks[i].SetType(static_cast<BrickType>(packetData.brickTypes[i]));
        }

        // Sync powerups
        state.powerups.clear();
        for (uint8_t i = 0; i < packetData.powerupCount; ++i) {
            PowerUpTypeEnum ptype = static_cast<PowerUpTypeEnum>(packetData.powerups[i].type);
            state.powerups.emplace_back(Vector2{packetData.powerups[i].x, packetData.powerups[i].y}, ptype);
        }

        // Update particles locally (visual only)
        for (auto& p : state.particles)
            p.Update();
        state.particles.erase(
            std::remove_if(state.particles.begin(), state.particles.end(),
            [](Particle& p) { return p.IsDead(); }),
            state.particles.end()
        );
    }
}
