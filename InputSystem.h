#pragma once
#include "raylib.h"
#include "GameState.h"
#include <thread>
#include <chrono>

class InputSystem {
private:
    static bool MouseInRect(Rectangle r) {
        Vector2 m = GetMousePosition();
        return CheckCollisionPointRec(m, r);
    }

public:
    void Handle(GameState& state) {

        // CLIENT: block all input until host sends INIT
        if (state.isClient) {
            if (!state.clientInitialized) return;
            if (IsKeyDown(KEY_LEFT)) state.paddle.MoveLeft(state.paddleSpeed);
            if (IsKeyDown(KEY_RIGHT)) state.paddle.MoveRight(state.paddleSpeed);
            return;
        }

        // HOST: async loading trigger (global)
        if (IsKeyPressed(KEY_L)) {
            std::lock_guard<std::mutex> lock(state.loadingMutex);
            if (state.loadingStatus == LoadingStatus::IDLE) {
                state.loadingStatus = LoadingStatus::LOADING;
                state.loadingFuture = std::async(std::launch::async, [&]() {
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                    std::lock_guard<std::mutex> lk(state.loadingMutex);
                    state.loadingStatus = LoadingStatus::DONE;
                    state.backgroundLoaded = true;
                });
            }
        }

        switch (state.currentState) {

        case GameStatus::MAIN_MENU: {
            bool canStart = !state.networkHost || state.networkConnected;
            Rectangle startBtn = {300, 280, 200, 60};
            if (canStart && MouseInRect(startBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                state.currentState = GameStatus::LOAD_MENU;
            }
            break;
        }

        case GameStatus::LOAD_MENU: {
            for (int slot = 1; slot <= 3; slot++) {
                float yBase = 110.0f + (slot - 1) * 145.0f;
                Rectangle loadBtn = {560, yBase + 10, 100, 35};
                Rectangle delBtn  = {560, yBase + 55, 100, 35};

                if (MouseInRect(loadBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (SaveManager::SlotExists(slot)) {
                        SaveData data = SaveManager::Load(slot);
                        state.score = data.score;
                        state.lives = data.lives;
                        state.maxUnlockedLevel = data.maxUnlockedLevel;
                        state.currentLevel = data.currentLevel;
                    } else {
                        state.score = 0;
                        state.lives = 3;
                        state.maxUnlockedLevel = 1;
                        state.currentLevel = 1;
                    }
                    state.selectedSlot = slot;
                    state.currentState = GameStatus::MENU;
                }

                if (MouseInRect(delBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    SaveManager::Delete(slot);
                }
            }
            break;
        }

        case GameStatus::MENU:
            if (IsKeyPressed(KEY_P)) {
                state.currentState = GameStatus::MAIN_MENU;
                break;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                state.currentState = GameStatus::PLAYING;
                if (state.lives <= 0) state.lives = 3;
            }
            if (IsKeyPressed(KEY_Q)) {
                state.currentState = GameStatus::LEVEL_SELECT;
            }
            if (IsKeyPressed(KEY_E)) {
                state.currentState = GameStatus::BRICK_EDITOR;
                state.editorSelectedIdx = -1;
            }
            break;

        case GameStatus::LEVEL_SELECT: {
            if (IsKeyPressed(KEY_P)) {
                state.currentState = GameStatus::MAIN_MENU;
                break;
            }
            if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE)) {
                state.currentState = GameStatus::MENU;
                break;
            }
            for (int lv = 1; lv <= 4; lv++) {
                float yBase = 100.0f + (lv - 1) * 105.0f;
                Rectangle btn = {250, yBase, 300, 55};
                bool unlocked = (lv <= state.maxUnlockedLevel) || (lv == 4);
                if (unlocked && MouseInRect(btn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    state.currentLevel = lv;
                    state.currentState = GameStatus::MENU;
                }
            }
            break;
        }

        case GameStatus::BRICK_EDITOR: {
            if (IsKeyPressed(KEY_P)) {
                state.currentState = GameStatus::MAIN_MENU;
                state.editorSelectedIdx = -1;
                break;
            }
            if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ESCAPE)) {
                state.currentState = GameStatus::MENU;
                state.bricksEdited = true;
                state.editorSelectedIdx = -1;
                break;
            }

            Vector2 m = GetMousePosition();

            // Click to select brick
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                state.editorSelectedIdx = -1;
                for (size_t i = 0; i < state.bricks.size(); i++) {
                    if (state.bricks[i].IsDestroyed()) continue;
                    if (CheckCollisionPointRec(m, state.bricks[i].GetRect())) {
                        state.editorSelectedIdx = (int)i;
                        break;
                    }
                }
            }

            // Right click on empty area: add brick
            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                bool onExisting = false;
                for (auto& b : state.bricks) {
                    if (b.IsDestroyed()) continue;
                    if (CheckCollisionPointRec(m, b.GetRect())) {
                        onExisting = true;
                        break;
                    }
                }
                if (!onExisting) {
                    state.bricks.emplace_back(
                        m.x - state.brickWidth / 2,
                        m.y - state.brickHeight / 2,
                        state.brickWidth, state.brickHeight, NORMAL);
                    state.editorSelectedIdx = (int)state.bricks.size() - 1;
                }
            }

            // Keyboard operations on selected brick
            if (state.editorSelectedIdx >= 0 && state.editorSelectedIdx < (int)state.bricks.size()) {
                if (IsKeyPressed(KEY_D)) {
                    state.bricks[state.editorSelectedIdx].Destroy();
                    state.editorSelectedIdx = -1;
                }
                if (IsKeyPressed(KEY_ONE)) {
                    state.bricks[state.editorSelectedIdx].SetType(NORMAL);
                }
                if (IsKeyPressed(KEY_TWO)) {
                    state.bricks[state.editorSelectedIdx].SetType(SPLIT);
                }
                if (IsKeyPressed(KEY_THREE)) {
                    state.bricks[state.editorSelectedIdx].SetType(DOUBLE_SCORE);
                }
                if (IsKeyPressed(KEY_FOUR)) {
                    state.bricks[state.editorSelectedIdx].SetType(ENLARGE_PADDLE);
                }
            }
            break;
        }

        case GameStatus::PLAYING:

            if (IsKeyPressed(KEY_P)) {
                SaveData sd;
                sd.score = state.score;
                sd.lives = state.lives;
                sd.maxUnlockedLevel = state.maxUnlockedLevel;
                sd.currentLevel = state.currentLevel;
                SaveManager::Save(state.selectedSlot, sd);
                state.balls.clear();
                state.bricks.clear();
                state.walls.clear();
                state.powerups.clear();
                state.particles.clear();
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
                state.currentState = GameStatus::MAIN_MENU;
                break;
            }

            if (IsKeyPressed(KEY_Q)) {
                state.currentState = GameStatus::LEVEL_SELECT;
                state.balls.clear();
                state.bricks.clear();
                state.walls.clear();
                state.powerups.clear();
                state.particles.clear();
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
                break;
            }

            if (IsKeyPressed(KEY_R)) {
                state.currentState = GameStatus::MENU;
                state.balls.clear();
                state.bricks.clear();
                state.walls.clear();
                state.powerups.clear();
                state.particles.clear();
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
            if (IsKeyPressed(KEY_P)) {
                SaveData sd;
                sd.score = state.score;
                sd.lives = state.lives;
                sd.maxUnlockedLevel = state.maxUnlockedLevel;
                sd.currentLevel = state.currentLevel;
                SaveManager::Save(state.selectedSlot, sd);
                state.balls.clear();
                state.bricks.clear();
                state.walls.clear();
                state.powerups.clear();
                state.particles.clear();
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
                state.currentState = GameStatus::MAIN_MENU;
                break;
            }
            if (IsKeyPressed(KEY_Q)) {
                state.currentState = GameStatus::LEVEL_SELECT;
                state.balls.clear();
                state.bricks.clear();
                state.walls.clear();
                state.powerups.clear();
                state.particles.clear();
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
                break;
            }
            if (IsKeyPressed(KEY_ENTER))
                state.currentState = GameStatus::PLAYING;
            break;

        case GameStatus::GAMEOVER:
        case GameStatus::VICTORY:

            if (IsKeyPressed(KEY_P)) {
                SaveData sd;
                sd.score = state.score;
                sd.lives = state.lives;
                sd.maxUnlockedLevel = state.maxUnlockedLevel;
                sd.currentLevel = state.currentLevel;
                SaveManager::Save(state.selectedSlot, sd);
                state.balls.clear();
                state.bricks.clear();
                state.walls.clear();
                state.powerups.clear();
                state.particles.clear();
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
                state.currentState = GameStatus::MAIN_MENU;
                break;
            }

            if (IsKeyPressed(KEY_Q)) {
                state.currentState = GameStatus::LEVEL_SELECT;
                state.balls.clear();
                state.bricks.clear();
                state.walls.clear();
                state.powerups.clear();
                state.particles.clear();
                state.lives = 3;
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
                break;
            }

            if (IsKeyPressed(KEY_R)) {
                state.currentState = GameStatus::MENU;
                state.balls.clear();
                state.bricks.clear();
                state.walls.clear();
                state.powerups.clear();
                state.particles.clear();
                state.lives = 3;
                state.ballLaunched = false;
                state.doubleScoreActive = false;
                state.doubleScoreTimer = 0.0f;
                state.enlargePaddleActive = false;
                state.enlargePaddleTimer = 0.0f;
                state.paddle.SetWidth(state.originalPaddleWidth);
            }

            break;

        default:
            break;
        }
    }
};
