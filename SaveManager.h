#pragma once
#include <string>
#include <fstream>
#include <cstdio>
#include "raylib.h"

struct SaveData {
    int score;
    int lives;
    int maxUnlockedLevel;
    int currentLevel;

    SaveData() : score(0), lives(3), maxUnlockedLevel(1), currentLevel(1) {}
};

class SaveManager {
public:
    static std::string GetSavePath(int slot) {
        return TextFormat("save_slot_%d.dat", slot);
    }

    static bool SlotExists(int slot) {
        std::ifstream f(GetSavePath(slot));
        return f.good();
    }

    static SaveData Load(int slot) {
        SaveData data;
        std::ifstream f(GetSavePath(slot));
        if (!f.good()) return data;
        f >> data.score >> data.lives >> data.maxUnlockedLevel >> data.currentLevel;
        if (data.lives <= 0) data.lives = 3;
        if (data.maxUnlockedLevel < 1) data.maxUnlockedLevel = 1;
        if (data.currentLevel < 1) data.currentLevel = 1;
        return data;
    }

    static void Save(int slot, const SaveData& data) {
        std::ofstream f(GetSavePath(slot));
        f << data.score << "\n" << data.lives << "\n"
          << data.maxUnlockedLevel << "\n" << data.currentLevel;
    }

    static void Delete(int slot) {
        std::remove(GetSavePath(slot).c_str());
    }
};
