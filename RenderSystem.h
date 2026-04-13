#pragma once
#include "GameState.h"
#include "raylib.h"

class RenderSystem {
public:
    void Draw(GameState& state) {

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 游戏对象
        state.paddle.Draw();

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

        EndDrawing();
    }
};