#pragma once
#define NOMINMAX
#include <algorithm>
#include "../core/Bitboard.h"
#include "../core/Engine.h"


struct SimulatorConfig {
    int maxTicks = 60;
    int manaPerCell = 100;
};

struct SimulationResult_25 {
    int ticks = 0;
    int mana = 0;
    bool success = false;
    Bitboard_25 footstep;
};

class DandelifeonSimulator_25 {
private:
    DandelifeonEngine_25 engine;

    const int maxTicks;
    const int manaPerCell;

    // (1 << 11) | (1 << 12) | (1 << 13)
    const uint32_t center_col_mask = 0x3800;

    inline bool checkAbsorption(const Bitboard_25& board) const {
        return (board.data[11] | board.data[12] | board.data[13]) & center_col_mask;
    }

    inline int countAbsorbedCells(const Bitboard_25& board) const {
        return __popcnt(board.data[11] & center_col_mask) +
            __popcnt(board.data[12] & center_col_mask) +
            __popcnt(board.data[13] & center_col_mask);
    }

public:
    DandelifeonSimulator_25(const SimulatorConfig& config)
        : maxTicks(config.maxTicks), manaPerCell(config.manaPerCell) {
    }

    SimulationResult_25 run(const Bitboard_25& startBoard, const Bitboard_25& obstacles) const {
        SimulationResult_25 result;
        result.footstep.clear();

        Bitboard_25 currentState = startBoard;
        currentState.applyObstacles(obstacles);

        Bitboard_25 nextState;
        nextState.clear();

        Bitboard_25* curr = &currentState;
        Bitboard_25* nxt = &nextState;

        for (int t = 1; t <= maxTicks; ++t) {
            engine.step(*curr, *nxt, obstacles);

            if (checkAbsorption(*nxt)) [[unlikely]] {
                result.success = true;
                result.ticks = t;
                result.mana = countAbsorbedCells(*nxt) * t * manaPerCell;
                result.footstep.merge(*curr);
                return result;
            }

            result.footstep.merge(*curr);
            std::swap(curr, nxt);

            if (curr->isEmpty()) [[unlikely]] {
                break;
            }
        }

        result.ticks = maxTicks;
        result.success = false;
        result.mana = 0;
        return result;
    }
};