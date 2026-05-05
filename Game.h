#pragma once
#include "raylib.h"
#include "GameState.h"
#include "InputSystem.h"
#include "PhysicsSystem.h"
#include "RenderSystem.h"
#include "NetworkManager.h"
#include "Config.h"
#include <vector>
#include "Particle.h"
#include <algorithm>
#include "ScoreCalculator.h"
#include "GameObject.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"

class Game {
private:
    Config config;

    GameState state;
    InputSystem input;
    PhysicsSystem* physics;
    RenderSystem render;
    NetworkManager network;
    NetworkMode netMode;

    ScoreCalculator scoreCalculator;
    std::vector<GameObject*> gameObjects;

public:
    Game(NetworkMode mode = NetworkMode::NONE) : netMode(mode) {

        config.Load("config.json");

        InitWindow(config.screenWidth, config.screenHeight, config.title.c_str());

        float paddleY = (netMode == NetworkMode::CLIENT) ? 520.0f : 550.0f;
        state.paddle = Paddle(
            350.0f,
            paddleY,
            config.paddleWidth,
            config.paddleHeight
        );

        float remotePaddleY = (netMode == NetworkMode::CLIENT) ? 550.0f : 520.0f;
        state.remotePaddle = Paddle(
            350.0f,
            remotePaddleY,
            config.paddleWidth,
            config.paddleHeight
        );

        state.lives = config.lives;
        state.currentLevel = 1;
        state.rows = config.brickRows;
        state.cols = config.brickCols;
        state.brickWidth = config.brickWidth;
        state.brickHeight = config.brickHeight;
        state.ballRadius = config.ballRadius;
        state.ballSpeed = config.ballSpeed;
        state.paddleSpeed = config.paddleSpeed;
        state.scoreNormal = config.scoreNormal;
        state.scoreSpecial = config.scoreSpecial;
        state.specialChanceSplit = config.specialChanceSplit;
        state.specialChanceDoubleScore = config.specialChanceDoubleScore;
        state.maxSpecialBrickRate = config.maxSpecialBrickRate;
        state.enlargePaddleDuration = config.enlargePaddleDuration;
        state.originalPaddleWidth = config.paddleWidth;

        physics = new PhysicsSystem(config.screenWidth, config.screenHeight);

        if (netMode == NetworkMode::CLIENT) {
            // CLIENT: no bricks, no ball - wait for INIT from HOST
            state.networkHost = false;
            state.networkClient = true;
            state.isClient = true;
            network.InitializeClient("127.0.0.1", 1234, state);
        } else {
            // HOST or single-player: normal initialization
            physics->ResetBricks(state);
            physics->ResetBall(state);
            state.powerUpFactory.ResetGenerationTracking();

            if (netMode == NetworkMode::HOST) {
                state.networkHost = true;
                state.networkClient = false;
                network.InitializeHost(1234, state);
            }
        }

        SetTargetFPS(60);
    }

    ~Game() {
        delete physics;
        for (auto obj : gameObjects) {
            delete obj;
        }
        CloseWindow();
    }

    void Run() {
        GameStatus previousState = state.currentState;

        while (!WindowShouldClose()) {
            if (netMode == NetworkMode::CLIENT) {
                RunClientFrame();
            } else {
                RunHostFrame(previousState);
            }
        }
    }

private:
    void RunClientFrame() {
        input.Handle(state);
        network.Service(state);

        if (network.IsConnected()) {
            network.SendClientPaddle(state);
        }

        render.Draw(state);
    }

    void RunHostFrame(GameStatus& previousState) {
        input.Handle(state);
        network.Service(state);

        // State transition handling
        if (state.currentState != previousState) {
            if (state.currentState == GameStatus::PLAYING && previousState == GameStatus::MENU) {
                physics->ResetBricks(state);
                physics->ResetBall(state);
                state.powerUpFactory.ResetGenerationTracking();
            }
            previousState = state.currentState;
        }

        physics->Update(state);

        // Network sending
        if (netMode == NetworkMode::HOST && network.IsConnected()) {
            if (!network.IsClientInitialized()) {
                network.SendInit(state);
            } else {
                network.SendHostSnapshot(state);
            }
        }

        render.Draw(state);
    }
};
