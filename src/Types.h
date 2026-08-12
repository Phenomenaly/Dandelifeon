#pragma once

#include <vector>
#include <cstdint>
#include "Bitboard.h"


struct SimulatorConfig {
    int maxTicks = 100;
    int manaPerCell = 60;
};

struct ThreadStatus {
    int threadId = 0;
    int ticks = 0;
    int mana = 0;
    uint64_t iterations = 0;
};

struct SharedLeaderboard {
    std::vector<ThreadStatus> threads;
    Bitboard bestCells;
    Bitboard bestWalls;
    int bestMana = 0;
    int bestTicks = 0;
};

struct LeaderboardEntry {
    Bitboard cells;
    Bitboard walls;
    int ticks = 0;
    int mana = 0;
};

struct SimulationResult {
    int ticks = 0;
    int mana = 0;
    bool success = false;
    Bitboard footstep;
};