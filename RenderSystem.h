#pragma once
#include "GameState.h"
#include "raylib.h"
#include <mutex>

class RenderSystem {
public:
    void Draw(GameState& state) {

        BeginDrawing();

        // 根据加载状态决定背景颜色
        {
            std::lock_guard<std::mutex> lock(state.loadingMutex);
            if (state.backgroundLoaded) {
                ClearBackground(Color{173, 216, 230, 255}); // 淡蓝色
            } else {
                ClearBackground(RAYWHITE);
            }
        }

        // CLIENT waiting for connection/initialization
        if (state.networkClient && !state.clientInitialized) {
            if (state.networkConnected) {
                DrawText("Connected! Waiting for game data...", 200, 280, 20, DARKGRAY);
            } else {
                DrawText("Connecting to host...", 250, 280, 24, DARKGRAY);
                DrawText("Waiting for game data", 240, 320, 18, GRAY);
            }
            EndDrawing();
            return;
        }

        // 游戏对象
        state.paddle.Draw();

        if (state.networkHost || state.networkClient) {
            DrawRectangleRec(state.remotePaddle.GetRect(), MAROON);
            DrawText((state.networkHost ? "Guest Paddle" : "Host Paddle"),
                     state.remotePaddle.GetRect().x,
                     state.remotePaddle.GetRect().y - 20,
                     12,
                     DARKGRAY);
        }

        for (auto& ball : state.balls)
            ball.Draw();

        for (auto& brick : state.bricks)
            brick.Draw();

        // 绘制道具
        for (auto& powerup : state.powerups)
            powerup.Draw();

        // UI
        DrawText(TextFormat("Score: %d", state.score), 10, 10, 20, BLACK);
        DrawText(TextFormat("Lives: %d", state.lives), 650, 10, 20, BLACK);
        DrawText(TextFormat("Level: %d", state.currentLevel), 350, 10, 20, BLUE);

        if (state.networkHost) {
            if (!state.networkConnected) {
                DrawText("HOST MODE - waiting for client...", 10, 40, 18, RED);
            } else {
                DrawText("HOST MODE - client connected", 10, 40, 18, DARKGREEN);
            }
        } else if (state.networkClient) {
            if (state.networkConnected) {
                DrawText("CLIENT MODE - connected to host", 10, 40, 18, DARKGREEN);
            } else {
                DrawText("CLIENT MODE - connecting...", 10, 40, 18, RED);
            }
        }

        // 显示当前活跃道具效果
        if (state.doubleScoreActive) {
            DrawText(TextFormat("2x SCORE: %.1f s", state.doubleScoreTimer), 350, 40, 16, PURPLE);
        }
        if (state.enlargePaddleActive) {
            DrawText(TextFormat("BIG PADDLE: %.1f s", state.enlargePaddleTimer), 350, 60, 16, PINK);
        }

        // 状态机显示
        switch (state.currentState) {

        case GameStatus::MENU:
            DrawText("PRESS ENTER TO START", 200, 300, 20, BLACK);
            DrawText("PRESS 1, 2, 3 TO SELECT LEVEL", 180, 330, 16, DARKGRAY);
            DrawText(TextFormat("CURRENT LEVEL: %d", state.currentLevel), 250, 360, 16, BLUE);
            break;

        case GameStatus::PAUSED:
            DrawText("PAUSED", 340, 250, 40, DARKGRAY);
            break;

        case GameStatus::GAMEOVER:
            DrawText("GAME OVER", 280, 250, 40, RED);
            break;

        case GameStatus::VICTORY:
            // 炫彩胜利字幕
            DrawText(" VICTORY! ", 250, 200, 50, GOLD);
            DrawText(" VICTORY! ", 252, 202, 50, YELLOW);  // 阴影效果
            
            // 显示最终得分
            DrawText(TextFormat("FINAL SCORE: %d", state.score), 280, 280, 30, GREEN);
            
            // 炫彩边框
            DrawRectangleLines(200, 180, 400, 140, GOLD);
            DrawRectangleLines(198, 178, 404, 144, YELLOW);
            break;

        default:
            break;
        }
        
        for (auto& p : state.particles)
            p.Draw();

        // 异步加载中显示Loading覆盖层
        {
            std::lock_guard<std::mutex> lock(state.loadingMutex);
            if (state.loadingStatus == LoadingStatus::LOADING) {
                DrawRectangle(0, 0, 800, 600, Color{0, 0, 0, 150});
                DrawText("Loading...", 320, 280, 30, WHITE);
            }
        }

        EndDrawing();
    }
};