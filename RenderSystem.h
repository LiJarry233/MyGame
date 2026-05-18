#pragma once
#include "GameState.h"
#include "raylib.h"
#include <mutex>

static void DrawButton(Rectangle r, const char* text, bool hover, Color base, Color hoverColor) {
    Color fill = hover ? hoverColor : base;
    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 2, DARKGRAY);
    int tw = MeasureText(text, 20);
    DrawText(text, (int)(r.x + r.width / 2 - tw / 2),
             (int)(r.y + r.height / 2 - 10), 20, WHITE);
}

class RenderSystem {
public:
    void Draw(GameState& state) {
        BeginDrawing();
        DrawScene(state);
        EndDrawing();
    }

    void DrawScene(GameState& state) {
        bool bgLoaded;
        bool loadingOverlay;
        {
            std::lock_guard<std::mutex> lock(state.loadingMutex);
            bgLoaded = state.backgroundLoaded;
            loadingOverlay = (state.loadingStatus == LoadingStatus::LOADING);
        }

        if (bgLoaded) {
            ClearBackground(Color{173, 216, 230, 255});
        } else {
            ClearBackground(RAYWHITE);
        }

        // MAIN_MENU
        if (state.currentState == GameStatus::MAIN_MENU) {
            DrawMainMenu(state);
            return;
        }

        // LOAD_MENU
        if (state.currentState == GameStatus::LOAD_MENU) {
            DrawLoadMenu();
            return;
        }

        // LEVEL_SELECT
        if (state.currentState == GameStatus::LEVEL_SELECT) {
            DrawLevelSelect(state);
            return;
        }

        // BRICK_EDITOR
        if (state.currentState == GameStatus::BRICK_EDITOR) {
            DrawBrickEditor(state);
            return;
        }

        // CLIENT waiting for host
        if (state.networkClient && !state.clientInitialized) {
            if (state.networkConnected) {
                DrawText("Connected! Waiting for host to start game...", 160, 280, 22, DARKGRAY);
                DrawText("Host is selecting save data", 240, 320, 16, GRAY);
            } else {
                DrawText("Connecting to host...", 250, 280, 24, DARKGRAY);
            }
            return;
        }

        state.paddle.Draw();

        if (state.networkHost || state.networkClient) {
            DrawRectangleRec(state.remotePaddle.GetRect(), MAROON);
            DrawText((state.networkHost ? "Guest Paddle" : "Host Paddle"),
                     state.remotePaddle.GetRect().x,
                     state.remotePaddle.GetRect().y - 20, 12, DARKGRAY);
        }

        for (auto& ball : state.balls)
            ball.Draw();

        // Draw walls (indestructible, brown)
        for (auto& wall : state.walls)
            wall.Draw();

        for (auto& brick : state.bricks) {
            if (brick.IsDestroyed()) continue;
            brick.Draw();
        }

        for (auto& powerup : state.powerups)
            powerup.Draw();

        // HUD
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
            }
        }

        if (state.doubleScoreActive) {
            DrawText(TextFormat("2x SCORE: %.1f s", state.doubleScoreTimer), 350, 40, 16, PURPLE);
        }
        if (state.enlargePaddleActive) {
            DrawText(TextFormat("BIG PADDLE: %.1f s", state.enlargePaddleTimer), 350, 60, 16, PINK);
        }

        switch (state.currentState) {

        case GameStatus::MENU:
            DrawText("PRESS ENTER TO START", 200, 290, 20, BLACK);
            DrawText("Q: SELECT LEVEL  |  E: EDIT  |  P: MAIN MENU", 155, 320, 14, DARKGRAY);
            DrawText(TextFormat("CURRENT LEVEL: %d", state.currentLevel), 280, 350, 16, BLUE);
            break;

        case GameStatus::PAUSED:
            DrawText("PAUSED", 340, 250, 40, DARKGRAY);
            break;

        case GameStatus::GAMEOVER:
            DrawText("GAME OVER", 280, 250, 40, RED);
            DrawText("PRESS R TO CONTINUE", 250, 310, 20, DARKGRAY);
            break;

        case GameStatus::VICTORY:
            DrawText(" VICTORY! ", 250, 200, 50, GOLD);
            DrawText(" VICTORY! ", 252, 202, 50, YELLOW);
            DrawText(TextFormat("FINAL SCORE: %d", state.score), 280, 280, 30, GREEN);
            DrawText("PRESS R TO CONTINUE", 250, 330, 20, DARKGRAY);
            if (state.currentLevel < 4 && state.maxUnlockedLevel > state.currentLevel) {
                DrawText(TextFormat("LEVEL %d UNLOCKED!", state.currentLevel + 1),
                         240, 370, 22, GREEN);
            }
            DrawRectangleLines(200, 180, 400, 220, GOLD);
            DrawRectangleLines(198, 178, 404, 224, YELLOW);
            break;

        default:
            break;
        }

        for (auto& p : state.particles)
            p.Draw();

        if (loadingOverlay) {
            DrawRectangle(0, 0, 800, 600, Color{0, 0, 0, 150});
            DrawText("Loading...", 320, 280, 30, WHITE);
        }
    }

private:
    void DrawMainMenu(GameState& state) {
        DrawText("BRICK BREAKER", 180, 100, 50, DARKBLUE);
        DrawText("Single-player / Host", 250, 160, 20, GRAY);

        bool canStart = !state.networkHost || state.networkConnected;

        if (state.networkHost && !state.networkConnected) {
            DrawText("Waiting for client to connect...", 210, 240, 22, RED);
            DrawText("Start a client on another window:", 200, 380, 16, DARKGRAY);
            DrawText("./MyRaylibGame client", 260, 410, 18, DARKGRAY);
        } else {
            Rectangle startBtn = {300, 280, 200, 60};
            Vector2 m = GetMousePosition();
            bool hover = m.x >= startBtn.x && m.x <= startBtn.x + startBtn.width &&
                         m.y >= startBtn.y && m.y <= startBtn.y + startBtn.height;
            DrawButton(startBtn, "START", hover, DARKBLUE, BLUE);
        }

        DrawText("Press ESC to quit", 290, 540, 16, LIGHTGRAY);
    }

    void DrawLoadMenu() {
        DrawText("SELECT SAVE SLOT", 230, 40, 36, DARKBLUE);
        Vector2 m = GetMousePosition();

        for (int slot = 1; slot <= 3; slot++) {
            float yBase = 110.0f + (slot - 1) * 145.0f;

            DrawRectangle(60, (int)yBase - 5, 660, 130,
                          Color{240, 240, 250, 255});
            DrawRectangleLines(60, (int)yBase - 5, 660, 130, LIGHTGRAY);

            DrawText(TextFormat("SAVE SLOT %d", slot), 80, (int)yBase + 5, 24, DARKBLUE);

            if (SaveManager::SlotExists(slot)) {
                SaveData data = SaveManager::Load(slot);
                DrawText(TextFormat("Total Score: %d", data.score),
                         80, (int)yBase + 38, 20, BLACK);
                DrawText(TextFormat("Lives: %d  |  Level: %d  |  Unlocked: 1-%d",
                         data.lives, data.currentLevel, data.maxUnlockedLevel),
                         80, (int)yBase + 65, 16, DARKGRAY);
            } else {
                DrawText("Empty", 80, (int)yBase + 50, 20, GRAY);
            }

            Rectangle loadBtn = {560, yBase + 10, 100, 35};
            bool loadHover = m.x >= loadBtn.x && m.x <= loadBtn.x + loadBtn.width &&
                             m.y >= loadBtn.y && m.y <= loadBtn.y + loadBtn.height;
            DrawButton(loadBtn, "LOAD", loadHover, DARKGREEN, GREEN);

            Rectangle delBtn = {560, yBase + 55, 100, 35};
            bool delHover = m.x >= delBtn.x && m.x <= delBtn.x + delBtn.width &&
                            m.y >= delBtn.y && m.y <= delBtn.y + delBtn.height;
            DrawButton(delBtn, "DELETE", delHover, MAROON, RED);
        }
    }

    void DrawLevelSelect(GameState& state) {
        DrawRectangle(0, 0, 800, 600, Color{0, 0, 0, 120});
        DrawText("SELECT LEVEL", 250, 30, 36, DARKBLUE);
        DrawText("Q / ESC to return | Level 4 always available", 200, 540, 14, GRAY);

        Vector2 m = GetMousePosition();

        for (int lv = 1; lv <= 4; lv++) {
            float yBase = 100.0f + (lv - 1) * 105.0f;
            Rectangle btn = {250, yBase, 300, 55};
            bool unlocked = (lv <= state.maxUnlockedLevel) || (lv == 4);
            bool hover = m.x >= btn.x && m.x <= btn.x + btn.width &&
                         m.y >= btn.y && m.y <= btn.y + btn.height;
            bool selected = lv == state.currentLevel;

            const char* name = "";
            switch (lv) {
                case 1: name = "LEVEL 1 - Classic"; break;
                case 2: name = "LEVEL 2 - Advanced"; break;
                case 3: name = "LEVEL 3 - Expert"; break;
                case 4: name = "LEVEL 4 - Hui (always open)"; break;
            }

            if (unlocked) {
                Color base = selected ? BLUE : DARKBLUE;
                Color hov  = selected ? BLUE : Color{50, 50, 200, 255};
                DrawButton(btn, name, hover, base, hov);
            } else {
                DrawRectangleRec(btn, Color{80, 80, 80, 200});
                DrawRectangleLinesEx(btn, 2, Color{120, 120, 120, 255});
                int tw = MeasureText(TextFormat("%s (LOCKED)", name), 18);
                DrawText(TextFormat("%s (LOCKED)", name),
                         (int)(btn.x + btn.width / 2 - tw / 2),
                         (int)(btn.y + btn.height / 2 - 9), 18,
                         Color{160, 160, 160, 255});
            }
        }
    }

    void DrawBrickEditor(GameState& state) {
        ClearBackground(Color{30, 30, 50, 255});

        // Draw grid lines
        for (int x = 0; x <= 800; x += 40)
            DrawLine(x, 0, x, 600, Color{50, 50, 70, 255});
        for (int y = 0; y <= 600; y += 40)
            DrawLine(0, y, 800, y, Color{50, 50, 70, 255});

        // Draw bricks
        for (size_t i = 0; i < state.bricks.size(); i++) {
            if (state.bricks[i].IsDestroyed()) continue;
            state.bricks[i].Draw();
            // Selection highlight
            if ((int)i == state.editorSelectedIdx) {
                Rectangle r = state.bricks[i].GetRect();
                DrawRectangleLinesEx(r, 3, WHITE);
            }
        }

        // Hover highlight
        Vector2 m = GetMousePosition();
        for (auto& b : state.bricks) {
            if (b.IsDestroyed()) continue;
            if (CheckCollisionPointRec(m, b.GetRect())) {
                DrawRectangleLinesEx(b.GetRect(), 1, YELLOW);
                break;
            }
        }

        // UI panel
        DrawRectangle(0, 0, 800, 36, Color{0, 0, 0, 200});
        DrawText("BRICK EDITOR | E/ESC: Exit | LClick: Select | RClick: Add | D: Delete",
                 10, 10, 14, LIGHTGRAY);
        DrawText("1:Nrm  2:Split  3:DblScore  4:Enlarge",
                 500, 10, 14, LIGHTGRAY);

        if (state.editorSelectedIdx >= 0 && state.editorSelectedIdx < (int)state.bricks.size()) {
            Rectangle r = state.bricks[state.editorSelectedIdx].GetRect();
            DrawText(TextFormat("Selected: (%.0f, %.0f) Type=%d",
                     r.x, r.y, (int)state.bricks[state.editorSelectedIdx].GetType()),
                     10, 580, 16, YELLOW);
        }
    }
};
