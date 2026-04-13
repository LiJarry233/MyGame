#pragma once
#include "GameState.h"
#include "PowerUpFactory.h"
#include <random>
#include <algorithm>
#include <vector>

class PhysicsSystem {
private:
    int screenWidth, screenHeight;

public:
    PhysicsSystem(int w, int h) {
        screenWidth = w;
        screenHeight = h;
    }

    void ResetBricks(GameState& state) {
        state.bricks.clear();

        // 使用固定的基础配置值，避免被修改后的state值影响
        const int BASE_ROWS = 5;
        const int BASE_COLS = 8;
        const float BASE_WIDTH = 80.0f;
        const float BASE_HEIGHT = 25.0f;

        // 根据关卡调整砖块参数
        if (state.currentLevel == 2) {
            // Level 2: 砖块变小变多
            state.rows = BASE_ROWS + 1;  // 多一行
            state.cols = BASE_COLS + 3;  // 多三列 (8+3=11)
            state.brickWidth = BASE_WIDTH * 0.8f;  // 宽度缩小20%
            state.brickHeight = BASE_HEIGHT * 0.8f; // 高度缩小20%
        } else if (state.currentLevel == 3) {
            // Level 3: 与level2相同
            state.rows = BASE_ROWS + 1;
            state.cols = BASE_COLS + 3;
            state.brickWidth = BASE_WIDTH * 0.8f;
            state.brickHeight = BASE_HEIGHT * 0.8f;
        } else {
            // Level 1: 使用基础参数
            state.rows = BASE_ROWS;
            state.cols = BASE_COLS;
            state.brickWidth = BASE_WIDTH;
            state.brickHeight = BASE_HEIGHT;
        }

        // 先生成所有普通砖块
        for (int r = 0; r < state.rows; r++) {
            for (int c = 0; c < state.cols; c++) {
                // 计算砖块位置，确保不会超出边界
                float totalWidth = state.cols * state.brickWidth;
                float totalGapWidth = (state.cols - 1) * 10.0f;  // 基础间距
                float availableWidth = screenWidth - 80.0f;  // 左右各留40像素边距
                
                // 如果总宽度超过可用宽度，调整间距
                float gap = 10.0f;
                if (totalWidth + totalGapWidth > availableWidth) {
                    gap = (availableWidth - totalWidth) / (state.cols - 1);
                    if (gap < 1.0f) gap = 1.0f;  // 最小间距
                }
                
                float x = 40.0f + c * (state.brickWidth + gap);
                float y = 80.0f + r * (state.brickHeight + 5.0f);

                state.bricks.emplace_back(
                    x, y,
                    state.brickWidth,
                    state.brickHeight,
                    NORMAL
                );
            }
        }

        // 然后随机选择位置放置特殊砖块
        int totalBricks = state.rows * state.cols;
        int maxSpecialBricks = static_cast<int>(totalBricks * state.maxSpecialBrickRate);
        
        if (maxSpecialBricks > 0) {
            std::vector<int> brickIndices;
            for (int i = 0; i < totalBricks; i++) {
                brickIndices.push_back(i);
            }
            
            // 随机数生成器
            std::random_device rd;
            std::mt19937 gen(rd());
            
            // 随机打乱索引
            std::shuffle(brickIndices.begin(), brickIndices.end(), gen);
            
            // 确保每种特殊类型至少出现一次
            std::vector<BrickType> specialTypes = {SPLIT, DOUBLE_SCORE, ENLARGE_PADDLE};
            int typesToPlace = std::min(static_cast<int>(specialTypes.size()), maxSpecialBricks);
            int specialBrickCount = 0;
            
            for (int i = 0; i < typesToPlace; i++) {
                int randomIndex = brickIndices[i];
                state.bricks[randomIndex].SetType(specialTypes[i]);
                specialBrickCount++;
            }
            
            // 放置剩余的特殊砖块
            for (int i = typesToPlace; i < maxSpecialBricks && i < totalBricks; i++) {
                int randomIndex = brickIndices[i];
                // 随机选择特殊类型
                std::uniform_int_distribution<> typeDis(0, specialTypes.size() - 1);
                state.bricks[randomIndex].SetType(specialTypes[typeDis(gen)]);
                specialBrickCount++;
            }
        }
    }

    void ResetBall(GameState& state) {
        state.balls.clear();

        // 根据关卡调整小球速度
        float adjustedBallSpeed = state.ballSpeed;
        if (state.currentLevel == 2) {
            adjustedBallSpeed = state.ballSpeed * 1.3f;  // Level 2: 1.3倍速度
        } else if (state.currentLevel == 3) {
            adjustedBallSpeed = state.ballSpeed * 1.6f;  // Level 3: 1.6倍速度
        }

        state.balls.emplace_back(
            Vector2{
                state.paddle.GetRect().x + state.paddle.GetRect().width / 2,
                state.paddle.GetRect().y - 10
            },
            adjustedBallSpeed,
            90.0f,
            state.ballRadius
        );

        state.ballLaunched = false;
    }

    void Update(GameState& state) {

        if (state.currentState != GameStatus::PLAYING)
            return;

        std::vector<Ball> newBalls;

        for (auto& ball : state.balls) {

            if (state.ballLaunched)
                ball.Move();
            else {
                // 小球跟随板子
                ball.SetPosition(Vector2{
                    state.paddle.GetRect().x + state.paddle.GetRect().width / 2,
                    state.paddle.GetRect().y - 10
                });
            }

            ball.BounceEdge(screenWidth, screenHeight);
            
            // 检查边界碰撞并生成粒子效果
            Vector2 ballPos = ball.GetPosition();
            float ballRadius = ball.GetRadius();
            
            // 左边界碰撞
            if (ballPos.x - ballRadius <= 5) {
                for (int i = 0; i < 8; i++) {
                    state.particles.emplace_back(Vector2{5.0f, ballPos.y});
                }
            }
            // 右边界碰撞
            if (ballPos.x + ballRadius >= screenWidth - 5) {
                for (int i = 0; i < 8; i++) {
                    state.particles.emplace_back(Vector2{(float)screenWidth - 5.0f, ballPos.y});
                }
            }
            // 上边界碰撞
            if (ballPos.y - ballRadius <= 5) {
                for (int i = 0; i < 8; i++) {
                    state.particles.emplace_back(Vector2{ballPos.x, 5.0f});
                }
            }
            
            ball.UpdateTrail();

            // 检查与板子的碰撞
            if (CheckCollisionCircleRec(
                ball.GetPosition(),
                ball.GetRadius(),
                state.paddle.GetRect())) {
                // 计算反射角度基于撞击位置
                float paddleCenterX = state.paddle.GetRect().x + state.paddle.GetRect().width / 2.0f;
                float ballX = ball.GetPosition().x;
                float relativeX = (ballX - paddleCenterX) / (state.paddle.GetRect().width / 2.0f);
                // 限制范围
                if (relativeX < -1.0f) relativeX = -1.0f;
                if (relativeX > 1.0f) relativeX = 1.0f;
                // 反射角度：中心90度（垂直），边缘偏转45度（左右对换）
                float reflectAngleDeg = 90.0f - relativeX * 45.0f;
                ball.SetAngle(reflectAngleDeg);
            }

            for (auto& brick : state.bricks) {

                if (!brick.IsDestroyed() &&
                    CheckCollisionCircleRec(
                        ball.GetPosition(),
                        ball.GetRadius(),
                        brick.GetRect())) {

                    int addScore = (brick.GetType() == DOUBLE_SCORE) ? state.scoreSpecial : state.scoreNormal;
                    if (state.doubleScoreActive) {
                        addScore *= 2;
                    }
                    state.score += addScore;

                    brick.Destroy();

                    // 生成粒子
                    Rectangle rect = brick.GetRect();
                    Vector2 center = {
                    rect.x + rect.width / 2,
                    rect.y + rect.height / 2
                    };

                    for (int i = 0; i < 20; i++) {
                        state.particles.emplace_back(center);
                    }

                    // 根据砖块类型掉落对应道具，绿色砖块不掉落
                    if (brick.GetType() == SPLIT) {
                        state.powerups.push_back(state.powerUpFactory.CreatePowerUp(center, POWERUP_SPLIT_BALL));
                    } else if (brick.GetType() == DOUBLE_SCORE) {
                        state.powerups.push_back(state.powerUpFactory.CreatePowerUp(center, POWERUP_DOUBLE_SCORE));
                    } else if (brick.GetType() == ENLARGE_PADDLE) {
                        state.powerups.push_back(state.powerUpFactory.CreatePowerUp(center, POWERUP_ENLARGE_PADDLE));
                    }

                    ball.Reflect({0, 1});
                    break;
                }
            }
        }

        state.balls.insert(state.balls.end(), newBalls.begin(), newBalls.end());
    
        // 检查小球掉落 - 先删除所有坠落的球
        for (auto it = state.balls.begin(); it != state.balls.end(); ) {
            if (it->GetPosition().y - it->GetRadius() > screenHeight) {
                it = state.balls.erase(it);
            } else {
                ++it;
            }
        }

        // ✅ 只有在所有小球都掉落时（没有小球了），才扣除生命值和重置
        if (state.balls.empty()) {
            state.lives--;
            if (state.lives <= 0) {
                state.currentState = GameStatus::GAMEOVER;
            } else {
                ResetBall(state);
            }
        }

        for (auto& p : state.particles)
            p.Update();

            state.particles.erase(
            std::remove_if(state.particles.begin(), state.particles.end(),
            [](Particle& p) { return p.IsDead(); }),
            state.particles.end()
        );

        // 更新道具
        for (auto& powerup : state.powerups) {
            powerup.Update();
        }

        // 检查道具与板子的碰撞
        for (auto it = state.powerups.begin(); it != state.powerups.end(); ) {
            if (CheckCollisionCircleRec(
                it->GetPosition(),
                it->GetRadius(),
                state.paddle.GetRect())) {
                // 道具被板子接住
                PowerUpTypeEnum type = it->GetType();
                if (type == POWERUP_SPLIT_BALL) {
                    // 立即分裂所有小球为3个
                    std::vector<Ball> splitBalls;
                    for (auto& ball : state.balls) {
                        // 原球保留，添加2个副本
                        Vector2 pos = ball.GetPosition();
                        float speed = sqrt(ball.GetVelocity().x * ball.GetVelocity().x + 
                                         ball.GetVelocity().y * ball.GetVelocity().y);
                        
                        // 三个方向：120度间隔 + 随机旋转
                        float baseAngle = (rand() % 360) * 1.0f;  // 随机基础角度
                        
                        for (int i = 0; i < 2; i++) {  // 增加2个副本
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
                    // 板子变长效果：1.5倍宽度，持续配置时间
                    if (!state.enlargePaddleActive) {
                        state.originalPaddleWidth = state.paddle.GetWidth();
                    }
                    state.paddle.SetWidth(state.originalPaddleWidth * 1.5f);
                    state.enlargePaddleActive = true;
                    state.enlargePaddleTimer = state.enlargePaddleDuration;
                }
                it = state.powerups.erase(it);
            } else if (it->IsFallen(screenHeight)) {
                // 道具掉出屏幕
                it = state.powerups.erase(it);
            } else {
                ++it;
            }
        }

        // 更新道具效果计时（只有积分翻倍有时间限制）
        if (state.doubleScoreActive) {
            state.doubleScoreTimer -= 1.0f / 60.0f;  // 假设60fps
            if (state.doubleScoreTimer <= 0.0f) {
                state.doubleScoreActive = false;
            }
        }

        // 更新板子变长效果计时
        if (state.enlargePaddleActive) {
            state.enlargePaddleTimer -= 1.0f / 60.0f;  // 假设60fps
            if (state.enlargePaddleTimer <= 0.0f) {
                state.enlargePaddleActive = false;
                state.paddle.SetWidth(state.originalPaddleWidth);  // 恢复原始宽度
            }
        }

        // ✅ 胜利判定：检查是否所有砖块都被摧毁
        bool allBricksDestroyed = true;
        for (const auto& brick : state.bricks) {
            if (!brick.IsDestroyed()) {
                allBricksDestroyed = false;
                break;
            }
        }
        if (allBricksDestroyed) {
            state.currentState = GameStatus::VICTORY;
        }
    }
};