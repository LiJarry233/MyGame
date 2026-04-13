#pragma once
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include "GameStateEnum.h"
#include "Particle.h"
#include "PowerUp.h"
#include "PowerUpFactory.h"
#include <vector>

struct GameState {

    Paddle paddle;
    std::vector<Particle> particles;
    std::vector<Ball> balls;
    std::vector<Brick> bricks;
    std::vector<PowerUp> powerups;

    // PowerUp工厂
    PowerUpFactory powerUpFactory;

    int score;
    int lives;

    // ✅ 状态机（替代多个 bool）
    GameStatus currentState;

    // 子状态（是否发射）
    bool ballLaunched;

    // 特效
    bool doubleScoreActive;
    float doubleScoreTimer;

    // 板子变长效果
    bool enlargePaddleActive;
    float enlargePaddleTimer;
    float originalPaddleWidth;  // 保存原始板子宽度

    // 配置驱动参数
    float ballRadius;
    float ballSpeed;
    float paddleSpeed;
    int scoreNormal;
    int scoreSpecial;
    float specialChanceSplit;
    float specialChanceDoubleScore;
    float maxSpecialBrickRate;
    float enlargePaddleDuration;

    // 关卡参数
    int rows, cols;
    float brickWidth, brickHeight;
    int currentLevel;  // 当前关卡 (1, 2, 3)

    GameState()
        : paddle(350.0f, 550.0f, 100.0f, 20.0f)
    {
        score = 0;
        lives = 3;
        currentLevel = 1;  // 默认从level1开始

        currentState = GameStatus::MENU;
        ballLaunched = false;

        doubleScoreActive = false;
        doubleScoreTimer = 0.0f;

        enlargePaddleActive = false;
        enlargePaddleTimer = 0.0f;
        originalPaddleWidth = 100.0f;  // 保存原始宽度

        ballRadius = 10.0f;
        ballSpeed = 4.0f;
        paddleSpeed = 6.0f;
        scoreNormal = 10;
        scoreSpecial = 20;
        specialChanceSplit = 0.15f;
        specialChanceDoubleScore = 0.10f;
        maxSpecialBrickRate = 0.3f;
        enlargePaddleDuration = 10.0f;

        rows = 5;
        cols = 8;
        brickWidth = 80.0f;
        brickHeight = 25.0f;
    }
};