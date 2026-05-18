#pragma once
#include "raylib.h"
#include "GameState.h"
#include "InputSystem.h"
#include "PhysicsSystem.h"
#include "RenderSystem.h"
#include "NetworkManager.h"
#include "Config.h"
#include "LevelConfig.h"
#include <vector>
#include "Particle.h"
#include <algorithm>
#include "ScoreCalculator.h"
#include "GameObject.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <fstream>
#include <cstdio>

class Game {
private:
    Config config;
    LevelsConfig levelCfg;

    GameState state;
    InputSystem input;
    PhysicsSystem* physics;
    RenderSystem render;
    NetworkManager network;
    NetworkMode netMode;
    bool benchmarkMode;

    ScoreCalculator scoreCalculator;
    std::vector<GameObject*> gameObjects;

public:
    Game(NetworkMode mode = NetworkMode::NONE, bool bench = false)
        : netMode(mode), benchmarkMode(bench) {

        config.Load("config.json");
        levelCfg.Load("levels.json");

        InitWindow(config.screenWidth, config.screenHeight, config.title.c_str());

        float paddleY = (netMode == NetworkMode::CLIENT) ? 520.0f : 550.0f;
        state.paddle = Paddle(
            350.0f,
            paddleY,
            config.paddleWidth,
            config.paddleHeight
        );

        float remotePaddleY = (netMode == NetworkMode::CLIENT) ? 550.0f : 520.0f;
        state.remotePaddle = Paddle(
            350.0f,
            remotePaddleY,
            config.paddleWidth,
            config.paddleHeight
        );

        state.lives = config.lives;
        state.currentLevel = 1;
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

        physics = new PhysicsSystem(config.screenWidth, config.screenHeight);
        physics->SetLevelConfig(&levelCfg);

        if (netMode == NetworkMode::CLIENT) {
            // CLIENT: no bricks, no ball - wait for INIT from HOST
            state.networkHost = false;
            state.networkClient = true;
            state.isClient = true;
            network.SetLevelConfig(&levelCfg);
            network.InitializeClient("127.0.0.1", 1234, state);
        } else {
            // HOST or single-player: normal initialization
            physics->ResetBricks(state);
            physics->ResetBall(state);
            state.powerUpFactory.ResetGenerationTracking();

            if (netMode == NetworkMode::HOST) {
                state.networkHost = true;
                state.networkClient = false;
                network.InitializeHost(1234, state);
            }
        }

        SetTargetFPS(60);
        if (benchmarkMode) {
            // benchmark: 解除FPS上限，便于真实测量
            SetTargetFPS(0);
        }
    }

    ~Game() {
        delete physics;
        for (auto obj : gameObjects) {
            delete obj;
        }
        CloseWindow();
    }

    void Run() {
        GameStatus previousState = state.currentState;

        while (!WindowShouldClose()) {
            if (netMode == NetworkMode::CLIENT) {
                RunClientFrame();
            } else {
                RunHostFrame(previousState);
            }
        }
    }

    // Benchmark: warmup + sample, write per-frame ms to benchmark_<tag>.csv + summary
    void RunBenchmark(const char* tag) {
        // Force into PLAYING with maximum load
        state.currentLevel = 3;            // hardest level, most/fastest bricks
        physics->ResetBricks(state);
        physics->ResetBall(state);
        state.currentState = GameStatus::PLAYING;
        state.ballLaunched = true;

        // Spawn extra balls for heavy collision and particle load
        if (!state.balls.empty()) {
            Ball seed = state.balls.front();
            Vector2 p = seed.GetPosition();
            float r = seed.GetRadius();
            for (int i = 0; i < 11; i++) {  // 12 balls total
                state.balls.emplace_back(p, 6.0f, 30.0f + i * 25.0f, r);
            }
        }

        // Continuously spawn particles to stress rendering
        const int WARMUP_FRAMES  = 120;    // warmup
        const int SAMPLE_FRAMES  = 1800;   // sample ~30s uncapped

        int frameIdx = 0;
        std::vector<float> physMs;  physMs.reserve(SAMPLE_FRAMES);
        std::vector<float> renderMs; renderMs.reserve(SAMPLE_FRAMES);
        while (!WindowShouldClose() && frameIdx < WARMUP_FRAMES + SAMPLE_FRAMES) {

            // Keep state in PLAYING so physics doesn't early-return
            state.lives = 999;
            state.currentState = GameStatus::PLAYING;

            // Inject continuous particle load
            for (int i = 0; i < 40; i++) {
                state.particles.emplace_back(Vector2{
                    (float)GetRandomValue(20, config.screenWidth - 20),
                    (float)GetRandomValue(20, config.screenHeight - 20)
                });
            }

            // Prevent powerup-driven ball explosion in benchmark
            state.powerups.clear();
            if (state.balls.size() > 32) state.balls.erase(state.balls.begin() + 32, state.balls.end());

            // Keep at least ~24 balls alive at all times
            while (state.balls.size() < 24) {
                state.balls.emplace_back(
                    Vector2{(float)GetRandomValue(100, config.screenWidth - 100),
                            (float)GetRandomValue(100, 300)},
                    6.0f,
                    (float)GetRandomValue(20, 160),
                    state.ballRadius);
            }
            state.ballLaunched = true;

            // Refill bricks if all destroyed
            bool anyAlive = false;
            for (const auto& b : state.bricks) if (!b.IsDestroyed()) { anyAlive = true; break; }
            if (!anyAlive) physics->ResetBricks(state);

            double t0 = GetTime();
            physics->Update(state);
            double t1 = GetTime();

            // Time only scene preparation (exclude EndDrawing's vsync-bound swap)
            BeginDrawing();
            double r0 = GetTime();
            render.DrawScene(state);
            double r1 = GetTime();
            EndDrawing();

            double t2 = GetTime();
            (void)t2;

            if (frameIdx >= WARMUP_FRAMES) {
                physMs.push_back((float)((t1 - t0) * 1000.0));
                renderMs.push_back((float)((r1 - r0) * 1000.0));
            }
            frameIdx++;
        }

        // Compute stats & write
        if (physMs.empty()) return;

        auto stats = [&](std::vector<float>& v, const char* name, std::ofstream& sm) {
            std::sort(v.begin(), v.end());
            double sum = 0; for (float x : v) sum += x;
            double mean = sum / v.size();
            float p50  = v[v.size() * 50 / 100];
            float p95  = v[v.size() * 95 / 100];
            float p99  = v[v.size() * 99 / 100];
            float mn = v.front(); float mx = v.back();
            sm << "[" << name << "]\n";
            sm << "  samples=" << v.size() << "\n";
            sm << "  mean_ms=" << mean << "\n";
            sm << "  p50_ms="  << p50  << "\n";
            sm << "  p95_ms="  << p95  << "\n";
            sm << "  p99_ms="  << p99  << "\n";
            sm << "  min_ms="  << mn   << "\n";
            sm << "  max_ms="  << mx   << "\n";
            sm << "  avg_fps=" << (1000.0 / mean) << "\n";
            return mean;
        };

        char sumPath[256], csvPath[256];
        std::snprintf(sumPath, sizeof(sumPath), "benchmark_%s.txt", tag);
        std::snprintf(csvPath, sizeof(csvPath), "benchmark_%s.csv", tag);

        std::ofstream sm(sumPath);
        double pMean = stats(physMs,  "physics",  sm);
        double rMean = stats(renderMs, "render",   sm);

        // Combined frame budget (physics + scene prep), excluding vsync swap
        sm << "[combined]\n";
        double cMean = pMean + rMean;
        sm << "  mean_ms=" << cMean << "\n";
        sm << "  avg_fps=" << (1000.0 / cMean) << "\n";

        std::ofstream csv(csvPath);
        csv << "frame,phys_ms,render_ms\n";
        // unsort: rewrite combined per-frame ordering already lost; emit sorted side-by-side
        for (size_t i = 0; i < physMs.size(); i++) {
            csv << i << "," << physMs[i] << "," << (i < renderMs.size() ? renderMs[i] : 0.0f) << "\n";
        }
    }

private:
    void RunClientFrame() {
        input.Handle(state);
        network.Service(state);

        if (network.IsConnected()) {
            network.SendClientPaddle(state);
        }

        render.Draw(state);
    }

    void RunHostFrame(GameStatus& previousState) {
        input.Handle(state);
        network.Service(state);

        // State transition handling
        if (state.currentState != previousState) {
            if (state.currentState == GameStatus::PLAYING && previousState == GameStatus::MENU) {
                if (!state.bricksEdited) {
                    physics->ResetBricks(state);
                }
                physics->ResetBall(state);
                state.powerUpFactory.ResetGenerationTracking();
                state.bricksEdited = false;
            }
            if (state.currentState == GameStatus::BRICK_EDITOR) {
                physics->ResetBricks(state);
                state.editorSelectedIdx = -1;
            }
            previousState = state.currentState;
        }

        physics->Update(state);

        // Network sending: only sync game state when PLAYING
        if (netMode == NetworkMode::HOST && network.IsConnected()) {
            if (state.currentState == GameStatus::PLAYING) {
                if (!network.IsClientInitialized()) {
                    network.SendInit(state);
                } else {
                    network.SendHostSnapshot(state);
                }
            }
        }

        render.Draw(state);
    }
};
