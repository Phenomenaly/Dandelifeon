#pragma once

#define NOMINMAX
#include <algorithm>
#include "Bitboard.h"
#include "Engine.h"
#include "Types.h"


class Simulator {
private:
    Engine engine;
    const int maxTicks;
    const int manaPerCell;
    const uint32_t center_col_mask = 0x3800;

    inline bool checkAbsorption(const Bitboard& board) const {
        return (board.data[12] | board.data[13] | board.data[14]) & center_col_mask;
    }

    inline int countAbsorbedCells(const Bitboard& board) const {
        return __popcnt(board.data[12] & center_col_mask) +
               __popcnt(board.data[13] & center_col_mask) +
               __popcnt(board.data[14] & center_col_mask);
    }

public:
    Simulator(const SimulatorConfig& config)
        : maxTicks(config.maxTicks), manaPerCell(config.manaPerCell) {}

    SimulationResult run(const Bitboard& startBoard, const Bitboard& obstacles) const {
        SimulationResult result;
        result.footstep.clear();

        Bitboard localObstacles = obstacles;
        localObstacles.setCell(12, 13, true);

        Bitboard currentState = startBoard;
        currentState.applyObstacles(localObstacles);

        Bitboard nextState;
        nextState.clear();

        Bitboard* curr = &currentState;
        Bitboard* nxt = &nextState;

        for (int t = 1; t <= maxTicks; ++t) {
            nxt->clear();
            engine.step(*curr, *nxt, localObstacles);

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