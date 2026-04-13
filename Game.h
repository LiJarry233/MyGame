#pragma once
#include "raylib.h"
#include "GameState.h"
#include "InputSystem.h"
#include "PhysicsSystem.h"
#include "RenderSystem.h"
#include "Config.h"
#include <vector>
#include "Particle.h"
#include <algorithm>

class Game {
private:
    Config config;

    GameState state;
    InputSystem input;
    PhysicsSystem* physics;
    RenderSystem render;

public:
    Game() {

        // 读取配置
        config.Load("config.json");

        // 初始化窗口
        InitWindow(config.screenWidth, config.screenHeight, config.title.c_str());

        // 初始化 paddle（用配置）
        state.paddle = Paddle(
            350.0f,
            550.0f,
            config.paddleWidth,
            config.paddleHeight
        );

        // 初始化游戏参数
        state.lives = config.lives;
        state.currentLevel = 1;  // 默认从level1开始
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

        // 物理系统
        physics = new PhysicsSystem(config.screenWidth, config.screenHeight);

        physics->ResetBricks(state);
        physics->ResetBall(state);

        // 重置道具工厂的生成跟踪
        state.powerUpFactory.ResetGenerationTracking();

        SetTargetFPS(60);
    }

    ~Game() {
        delete physics;
        CloseWindow();
    }

    void Run() {
        GameStatus previousState = state.currentState;
        
        while (!WindowShouldClose()) {
            input.Handle(state);
            
            // 检测状态变化
            if (state.currentState != previousState) {
                if (state.currentState == GameStatus::PLAYING && previousState == GameStatus::MENU) {
                    // 从菜单进入游戏，重新生成砖块和小球
                    physics->ResetBricks(state);
                    physics->ResetBall(state);
                    state.powerUpFactory.ResetGenerationTracking();
                }
                previousState = state.currentState;
            }
            
            physics->Update(state);
            render.Draw(state);
        }
    }
};