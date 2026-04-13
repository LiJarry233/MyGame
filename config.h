#pragma once
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Config {

    // window
    int screenWidth;
    int screenHeight;
    std::string title;

    // ball
    float ballRadius;
    float ballSpeed;

    // paddle
    float paddleWidth;
    float paddleHeight;
    float paddleSpeed;

    // brick
    int brickRows;
    int brickCols;
    float brickWidth;
    float brickHeight;

    // game
    int lives;
    int scoreNormal;
    int scoreSpecial;
    float enlargePaddleDuration;
    float specialChanceSplit;
    float specialChanceDoubleScore;
    float maxSpecialBrickRate;

    void Load(const std::string& path) {
        std::ifstream f(path);
        json j = json::parse(f);

        screenWidth = j["window"]["width"];
        screenHeight = j["window"]["height"];
        title = j["window"]["title"];

        ballRadius = j["ball"]["radius"];
        ballSpeed = j["ball"]["speed"];

        paddleWidth = j["paddle"]["width"];
        paddleHeight = j["paddle"]["height"];
        paddleSpeed = j["paddle"]["speed"];

        brickRows = j["brick"]["rows"];
        brickCols = j["brick"]["cols"];
        brickWidth = j["brick"]["width"];
        brickHeight = j["brick"]["height"];
        specialChanceSplit = j["brick"]["specialChanceSplit"];
        specialChanceDoubleScore = j["brick"]["specialChanceDoubleScore"];
        maxSpecialBrickRate = j["brick"]["maxSpecialRate"];

        lives = j["game"]["lives"];
        scoreNormal = j["game"]["scoreNormal"];
        scoreSpecial = j["game"]["scoreSpecial"];
        enlargePaddleDuration = j["game"]["enlargePaddleDuration"];
    }
};