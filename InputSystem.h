#pragma once
#include "raylib.h"
#include "GameState.h"
#include <thread>
#include <chrono>

class InputSystem {
public:
    void Handle(GameState& state) {

        // CLIENT只处理板子移动
        if (state.isClient) {
            if (IsKeyDown(KEY_LEFT)) state.paddle.MoveLeft(state.paddleSpeed);
            if (IsKeyDown(KEY_RIGHT)) state.paddle.MoveRight(state.paddleSpeed);
            return;
        }

        // HOST按L键触发异步加载
        if (IsKeyPressed(KEY_L)) {
            std::lock_guard<std::mutex> lock(state.loadingMutex);
            if (state.loadingStatus == LoadingStatus::IDLE) {
                state.loadingStatus = LoadingStatus::LOADING;
                state.loadingFuture = std::async(std::launch::async, [&state]() {
                    // 模拟加载大型纹理（3秒）
                    std::this_thread::sleep_for(std::chrono::seconds(3));

                    std::lock_guard<std::mutex> lock(state.loadingMutex);
                    state.loadingStatus = LoadingStatus::DONE;
                    state.backgroundLoaded = true;
                });
            }
        }

        switch (state.currentState) {

        case GameStatus::MENU:
            if (IsKeyPressed(KEY_ENTER)) {
                state.currentState = GameStatus::PLAYING;
                state.score = 0;
                state.lives = 3;
            }
            break;

        case GameStatus::PLAYING:

            if (IsKeyPressed(KEY_P))
                state.currentState = GameStatus::PAUSED;

            // R键重开游戏
            if (IsKeyPressed(KEY_R)) {
                state.currentState = GameStatus::MENU;
                state.balls.clear();
                state.bricks.clear();
                state.powerups.clear();
                state.particles.clear();
                state.score = 0;
                state.lives = 3;
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
            }

            // 关卡切换按键
            if (IsKeyPressed(KEY_ONE)) {
                state.currentLevel = 1;
                state.currentState = GameStatus::MENU;
                state.balls.clear();
                state.bricks.clear();
                state.powerups.clear();
                state.particles.clear();
                state.score = 0;
                state.lives = 3;
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
            }
            if (IsKeyPressed(KEY_TWO)) {
                state.currentLevel = 2;
                state.currentState = GameStatus::MENU;
                state.balls.clear();
                state.bricks.clear();
                state.powerups.clear();
                state.particles.clear();
                state.score = 0;
                state.lives = 3;
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
            }
            if (IsKeyPressed(KEY_THREE)) {
                state.currentLevel = 3;
                state.currentState = GameStatus::MENU;
                state.balls.clear();
                state.bricks.clear();
                state.powerups.clear();
                state.particles.clear();
                state.score = 0;
                state.lives = 3;
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
            }

            if (!state.ballLaunched && IsKeyPressed(KEY_SPACE))
                state.ballLaunched = true;

            if (IsKeyDown(KEY_LEFT)) state.paddle.MoveLeft(state.paddleSpeed);
            if (IsKeyDown(KEY_RIGHT)) state.paddle.MoveRight(state.paddleSpeed);

            break;

        case GameStatus::PAUSED:
            if (IsKeyPressed(KEY_P))
                state.currentState = GameStatus::PLAYING;
            break;

        case GameStatus::GAMEOVER:
        case GameStatus::VICTORY:

            if (IsKeyPressed(KEY_R)) {
                state.currentState = GameStatus::MENU;
                state.balls.clear();
                state.bricks.clear();
                state.powerups.clear();
                state.particles.clear();
                state.score = 0;
                state.lives = 3;
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
                // 注意：不重置currentLevel，保持当前关卡
            }

            break;

        default:
            break;
        }
    }
};