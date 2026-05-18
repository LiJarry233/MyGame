#pragma once
#include "GameState.h"
#include "PowerUpFactory.h"
#include "SaveManager.h"
#include "LevelConfig.h"
#include <random>
#include <algorithm>
#include <vector>

class PhysicsSystem {
private:
    int screenWidth, screenHeight;
    const LevelsConfig* levelConfig;

public:
    PhysicsSystem(int w, int h)
        : screenWidth(w), screenHeight(h), levelConfig(nullptr) {}

    void SetLevelConfig(const LevelsConfig* cfg) { levelConfig = cfg; }

    const LevelDef* GetDef(int level) const {
        if (levelConfig) return levelConfig->GetLevel(level);
        return nullptr;
    }

    void ResetBricks(GameState& state) {
        state.bricks.clear();
        state.walls.clear();

        const LevelDef* def = GetDef(state.currentLevel);
        if (!def) {
            // Fallback: default grid
            state.rows = 5; state.cols = 8;
            state.brickWidth = 80; state.brickHeight = 25;
            for (int r = 0; r < state.rows; r++)
                for (int c = 0; c < state.cols; c++)
                    state.bricks.emplace_back(
                        40.0f + c * 90.0f, 80.0f + r * 30.0f,
                        80.0f, 25.0f, NORMAL);
            return;
        }

        state.rows = def->rows;
        state.cols = def->cols;
        state.brickWidth = def->brickWidth;
        state.brickHeight = def->brickHeight;

        if (!def->gridTemplate.empty()) {
            // Template-based layout (e.g. level 4 hui shape)
            for (int r = 0; r < def->rows; r++) {
                const std::string& row = def->gridTemplate[r];
                for (int c = 0; c < def->cols && c < (int)row.size(); c++) {
                    char ch = row[c];
                    if (ch == '.') continue;
                    float x = def->startX + c * (def->brickWidth + def->gapX);
                    float y = def->startY + r * (def->brickHeight + def->gapY);
                    BrickType type = NORMAL;
                    if (ch == 'S') type = SPLIT;
                    else if (ch == 'D') type = DOUBLE_SCORE;
                    else if (ch == 'E') type = ENLARGE_PADDLE;
                    state.bricks.emplace_back(x, y, def->brickWidth, def->brickHeight, type);
                }
            }
        } else {
            // Grid layout with random specials
            for (int r = 0; r < def->rows; r++) {
                for (int c = 0; c < def->cols; c++) {
                    float x = def->startX + c * (def->brickWidth + def->gapX);
                    float y = def->startY + r * (def->brickHeight + def->gapY);
                    state.bricks.emplace_back(x, y, def->brickWidth, def->brickHeight, NORMAL);
                }
            }
            AssignSpecials(state, def->specialRate);
        }

        // Create walls from level config
        for (auto& wd : def->walls) {
            state.walls.emplace_back(wd.x, wd.y, wd.width, wd.height);
        }
    }

    void ResetBricksPlain(GameState& state) {
        state.bricks.clear();
        state.walls.clear();

        const LevelDef* def = GetDef(state.currentLevel);
        if (!def) {
            state.rows = 5; state.cols = 8;
            state.brickWidth = 80; state.brickHeight = 25;
            for (int r = 0; r < state.rows; r++)
                for (int c = 0; c < state.cols; c++)
                    state.bricks.emplace_back(
                        40.0f + c * 90.0f, 80.0f + r * 30.0f,
                        80.0f, 25.0f, NORMAL);
            return;
        }

        state.rows = def->rows;
        state.cols = def->cols;
        state.brickWidth = def->brickWidth;
        state.brickHeight = def->brickHeight;

        if (!def->gridTemplate.empty()) {
            for (int r = 0; r < def->rows; r++) {
                const std::string& row = def->gridTemplate[r];
                for (int c = 0; c < def->cols && c < (int)row.size(); c++) {
                    char ch = row[c];
                    if (ch == '.') continue;
                    float x = def->startX + c * (def->brickWidth + def->gapX);
                    float y = def->startY + r * (def->brickHeight + def->gapY);
                    BrickType type = NORMAL;
                    if (ch == 'S') type = SPLIT;
                    else if (ch == 'D') type = DOUBLE_SCORE;
                    else if (ch == 'E') type = ENLARGE_PADDLE;
                    state.bricks.emplace_back(x, y, def->brickWidth, def->brickHeight, type);
                }
            }
        } else {
            for (int r = 0; r < def->rows; r++) {
                for (int c = 0; c < def->cols; c++) {
                    float x = def->startX + c * (def->brickWidth + def->gapX);
                    float y = def->startY + r * (def->brickHeight + def->gapY);
                    state.bricks.emplace_back(x, y, def->brickWidth, def->brickHeight, NORMAL);
                }
            }
        }

        for (auto& wd : def->walls) {
            state.walls.emplace_back(wd.x, wd.y, wd.width, wd.height);
        }
    }

    void ResetBall(GameState& state) {
        state.balls.clear();
        const LevelDef* def = GetDef(state.currentLevel);
        float mult = def ? def->ballSpeedMult : 1.0f;
        float speed = state.ballSpeed * mult;

        state.balls.emplace_back(
            Vector2{
                state.paddle.GetRect().x + state.paddle.GetRect().width / 2,
                state.paddle.GetRect().y - 10
            },
            speed,
            90.0f,
            state.ballRadius
        );
        state.ballLaunched = false;
    }

    void Update(GameState& state) {
        if (state.currentState != GameStatus::PLAYING)
            return;
        if (state.isClient) return;

        std::vector<Ball> newBalls;

        for (auto& ball : state.balls) {

            if (state.ballLaunched)
                ball.Move();
            else {
                ball.SetPosition(Vector2{
                    state.paddle.GetRect().x + state.paddle.GetRect().width / 2,
                    state.paddle.GetRect().y - 10
                });
            }

            ball.BounceEdge(screenWidth, screenHeight);

            Vector2 ballPos = ball.GetPosition();
            float ballRadius = ball.GetRadius();

            if (ballPos.x - ballRadius <= 5) {
                for (int i = 0; i < 8; i++)
                    state.particles.emplace_back(Vector2{5.0f, ballPos.y});
            }
            if (ballPos.x + ballRadius >= screenWidth - 5) {
                for (int i = 0; i < 8; i++)
                    state.particles.emplace_back(Vector2{(float)screenWidth - 5.0f, ballPos.y});
            }
            if (ballPos.y - ballRadius <= 5) {
                for (int i = 0; i < 8; i++)
                    state.particles.emplace_back(Vector2{ballPos.x, 5.0f});
            }

            ball.UpdateTrail();

            auto TryPaddleCollision = [&](const Rectangle& paddleRect) {
                if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), paddleRect)) {
                    float paddleCenterX = paddleRect.x + paddleRect.width / 2.0f;
                    float ballX = ball.GetPosition().x;
                    float relativeX = (ballX - paddleCenterX) / (paddleRect.width / 2.0f);
                    if (relativeX < -1.0f) relativeX = -1.0f;
                    if (relativeX > 1.0f) relativeX = 1.0f;
                    float reflectAngleDeg = 90.0f - relativeX * 45.0f;
                    ball.SetAngle(reflectAngleDeg);
                    return true;
                }
                return false;
            };

            if (!TryPaddleCollision(state.paddle.GetRect()) && state.networkHost) {
                TryPaddleCollision(state.remotePaddle.GetRect());
            }

            // Wall collisions
            for (auto& wall : state.walls) {
                Rectangle wr = wall.GetRect();
                float bx = ball.GetPosition().x;
                float by = ball.GetPosition().y;
                float br = ball.GetRadius();
                if (bx + br < wr.x || bx - br > wr.x + wr.width ||
                    by + br < wr.y || by - br > wr.y + wr.height)
                    continue;

                if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), wr)) {
                    // Determine which side was hit
                    float overlapLeft   = (bx + br) - wr.x;
                    float overlapRight  = (wr.x + wr.width) - (bx - br);
                    float overlapTop    = (by + br) - wr.y;
                    float overlapBottom = (wr.y + wr.height) - (by - br);
                    float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});

                    if (minOverlap == overlapTop || minOverlap == overlapBottom)
                        ball.Reflect({0, 1});
                    else
                        ball.Reflect({1, 0});

                    Vector2 wallCenter = {wr.x + wr.width / 2, wr.y + wr.height / 2};
                    for (int i = 0; i < 8; i++)
                        state.particles.emplace_back(wallCenter);
                }
            }

            // Brick collisions
            for (auto& brick : state.bricks) {
                if (brick.IsDestroyed()) continue;

                Rectangle r = brick.GetRect();
                float bx = ball.GetPosition().x;
                float by = ball.GetPosition().y;
                float br = ball.GetRadius();
                if (bx + br < r.x || bx - br > r.x + r.width ||
                    by + br < r.y || by - br > r.y + r.height)
                    continue;

                if (CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), r)) {

                    int addScore = (brick.GetType() == DOUBLE_SCORE) ? state.scoreSpecial : state.scoreNormal;
                    if (state.doubleScoreActive) addScore *= 2;
                    state.score += addScore;

                    brick.Destroy();

                    Rectangle rect = brick.GetRect();
                    Vector2 center = {rect.x + rect.width / 2, rect.y + rect.height / 2};
                    for (int i = 0; i < 20; i++)
                        state.particles.emplace_back(center);

                    if (brick.GetType() == SPLIT)
                        state.powerups.push_back(state.powerUpFactory.CreatePowerUp(center, POWERUP_SPLIT_BALL));
                    else if (brick.GetType() == DOUBLE_SCORE)
                        state.powerups.push_back(state.powerUpFactory.CreatePowerUp(center, POWERUP_DOUBLE_SCORE));
                    else if (brick.GetType() == ENLARGE_PADDLE)
                        state.powerups.push_back(state.powerUpFactory.CreatePowerUp(center, POWERUP_ENLARGE_PADDLE));

                    ball.Reflect({0, 1});
                    break;
                }
            }
        }

        state.balls.insert(state.balls.end(), newBalls.begin(), newBalls.end());

        // Ball fall check
        for (auto it = state.balls.begin(); it != state.balls.end(); ) {
            if (it->GetPosition().y - it->GetRadius() > screenHeight)
                it = state.balls.erase(it);
            else
                ++it;
        }

        if (state.balls.empty()) {
            state.lives--;
            if (state.lives <= 0) {
                SaveData sd;
                sd.score = state.score;
                sd.lives = 0;
                sd.maxUnlockedLevel = state.maxUnlockedLevel;
                sd.currentLevel = state.currentLevel;
                SaveManager::Save(state.selectedSlot, sd);
                state.currentState = GameStatus::GAMEOVER;
            } else {
                ResetBall(state);
            }
        }

        // Particles
        for (size_t i = 0; i < state.particles.size(); ) {
            if (state.particles[i].Update()) { ++i; }
            else {
                state.particles[i] = state.particles.back();
                state.particles.pop_back();
            }
        }

        // PowerUps
        for (auto& powerup : state.powerups)
            powerup.Update();

        for (auto it = state.powerups.begin(); it != state.powerups.end(); ) {
            if (CheckCollisionCircleRec(it->GetPosition(), it->GetRadius(), state.paddle.GetRect())) {
                PowerUpTypeEnum type = it->GetType();
                if (type == POWERUP_SPLIT_BALL) {
                    std::vector<Ball> splitBalls;
                    for (auto& ball : state.balls) {
                        Vector2 pos = ball.GetPosition();
                        float speed = sqrt(ball.GetVelocity().x * ball.GetVelocity().x +
                                         ball.GetVelocity().y * ball.GetVelocity().y);
                        float baseAngle = (rand() % 360) * 1.0f;
                        for (int i = 0; i < 2; i++) {
                            float angle = baseAngle + (i + 1) * 120.0f;
                            Ball newBall(pos, speed, angle, ball.GetRadius());
                            splitBalls.push_back(newBall);
                        }
                    }
                    state.balls.insert(state.balls.end(), splitBalls.begin(), splitBalls.end());
                } else if (type == POWERUP_DOUBLE_SCORE) {
                    state.doubleScoreActive = true;
                    state.doubleScoreTimer = 10.0f;
                } else if (type == POWERUP_ENLARGE_PADDLE) {
                    if (!state.enlargePaddleActive)
                        state.originalPaddleWidth = state.paddle.GetWidth();
                    state.paddle.SetWidth(state.originalPaddleWidth * 1.5f);
                    state.enlargePaddleActive = true;
                    state.enlargePaddleTimer = state.enlargePaddleDuration;
                }
                it = state.powerups.erase(it);
            } else if (it->IsFallen(screenHeight)) {
                it = state.powerups.erase(it);
            } else {
                ++it;
            }
        }

        // Timers
        if (state.doubleScoreActive) {
            state.doubleScoreTimer -= 1.0f / 60.0f;
            if (state.doubleScoreTimer <= 0.0f)
                state.doubleScoreActive = false;
        }
        if (state.enlargePaddleActive) {
            state.enlargePaddleTimer -= 1.0f / 60.0f;
            if (state.enlargePaddleTimer <= 0.0f) {
                state.enlargePaddleActive = false;
                state.paddle.SetWidth(state.originalPaddleWidth);
            }
        }

        // Victory check
        bool allBricksDestroyed = true;
        for (const auto& brick : state.bricks) {
            if (!brick.IsDestroyed()) {
                allBricksDestroyed = false;
                break;
            }
        }
        if (allBricksDestroyed) {
            if (state.currentLevel < 4 && state.maxUnlockedLevel <= state.currentLevel)
                state.maxUnlockedLevel = state.currentLevel + 1;

            SaveData sd;
            sd.score = state.score;
            sd.lives = state.lives;
            sd.maxUnlockedLevel = state.maxUnlockedLevel;
            sd.currentLevel = state.currentLevel;
            SaveManager::Save(state.selectedSlot, sd);
            state.currentState = GameStatus::VICTORY;
        }
    }

private:
    void AssignSpecials(GameState& state, float specialRate) {
        int total = (int)state.bricks.size();
        int maxSpecial = (int)(total * specialRate);
        if (maxSpecial <= 0) return;

        std::vector<int> indices(total);
        for (int i = 0; i < total; i++) indices[i] = i;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);

        std::vector<BrickType> specialTypes = {SPLIT, DOUBLE_SCORE, ENLARGE_PADDLE};
        int typeCount = std::min((int)specialTypes.size(), maxSpecial);

        for (int i = 0; i < typeCount; i++)
            state.bricks[indices[i]].SetType(specialTypes[i]);

        std::uniform_int_distribution<> typeDis(0, (int)specialTypes.size() - 1);
        for (int i = typeCount; i < maxSpecial && i < total; i++)
            state.bricks[indices[i]].SetType(specialTypes[typeDis(gen)]);
    }
};
