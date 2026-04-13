#pragma once
#include "raylib.h"
#include "GameState.h"

class InputSystem {
public:
    void Handle(GameState& state) {

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