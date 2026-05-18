#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
#include "raylib.h"

using json = nlohmann::json;

struct WallDef {
    float x, y, width, height;
};

struct LevelDef {
    int id;
    std::string name;
    float ballSpeedMult;
    float brickWidth;
    float brickHeight;
    float specialRate;
    int rows;
    int cols;
    float startX;
    float startY;
    float gapX;
    float gapY;
    std::vector<std::string> gridTemplate;  // for template-based levels
    std::vector<WallDef> walls;
};

struct LevelsConfig {
    std::vector<LevelDef> levels;

    bool Load(const std::string& path) {
        std::ifstream f(path);
        if (!f.good()) return false;
        json j = json::parse(f);

        for (auto& lj : j["levels"]) {
            LevelDef def;
            def.id = lj["id"];
            def.name = lj.value("name", "");
            def.ballSpeedMult = lj.value("ballSpeedMult", 1.0f);
            def.brickWidth = lj.value("brickWidth", 80.0f);
            def.brickHeight = lj.value("brickHeight", 25.0f);
            def.specialRate = lj.value("specialRate", 0.3f);
            def.rows = lj.value("rows", 5);
            def.cols = lj.value("cols", 8);
            def.startX = lj.value("startX", 40.0f);
            def.startY = lj.value("startY", 80.0f);
            def.gapX = lj.value("gapX", 10.0f);
            def.gapY = lj.value("gapY", 5.0f);

            if (lj.contains("template")) {
                for (auto& row : lj["template"]) {
                    def.gridTemplate.push_back(row.get<std::string>());
                }
                def.rows = (int)def.gridTemplate.size();
                def.cols = def.gridTemplate.empty() ? 0 : (int)def.gridTemplate[0].size();
            }

            if (lj.contains("walls")) {
                for (auto& wj : lj["walls"]) {
                    WallDef w;
                    w.x = wj["x"];
                    w.y = wj["y"];
                    w.width = wj["width"];
                    w.height = wj["height"];
                    def.walls.push_back(w);
                }
            }

            levels.push_back(def);
        }
        return true;
    }

    const LevelDef* GetLevel(int id) const {
        for (auto& l : levels) {
            if (l.id == id) return &l;
        }
        return nullptr;
    }
};
