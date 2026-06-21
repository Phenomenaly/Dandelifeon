#pragma once

#include <vector>
#include <cstdint>


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

template <typename BitboardType>
struct SimulationResult {
    int ticks = 0;
    int mana = 0;
    bool success = false;
    BitboardType footstep;
};

template <typename BitboardType>
struct LeaderboardEntry {
    BitboardType cells;
    BitboardType walls;
    int ticks = 0;
    int mana = 0;
};

template <typename BitboardType>
struct SharedLeaderboard {
    std::vector<ThreadStatus> threads;
    BitboardType bestCells;
    BitboardType bestWalls;
    int bestMana = 0;
    int bestTicks = 0;
};