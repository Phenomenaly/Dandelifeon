#pragma once

#define NOMINMAX
#include <algorithm>

#include "../core/ISimulation.h"
#include "../core/BitboardHandler.h"
#include "Engine_25.h"


class DandelifeonSimulator_25 : public IDandelifeonSimulator<Bitboard_25> {
private:
    DandelifeonEngine_25 engine;

    const int maxTicks;
    const int manaPerCell;

    const uint32_t center_col_mask = 0x3800;

    inline bool checkAbsorption(const Bitboard_25& board) const {
        return (board.data[12] | board.data[13] | board.data[14]) & center_col_mask;
    }

    inline int countAbsorbedCells(const Bitboard_25& board) const {
        return  __popcnt(board.data[12] & center_col_mask) +
                __popcnt(board.data[13] & center_col_mask) +
                __popcnt(board.data[14] & center_col_mask);
    }

public:
    DandelifeonSimulator_25(const SimulatorConfig& config)
        : maxTicks(config.maxTicks), manaPerCell(config.manaPerCell) {
    }

    SimulationResult<Bitboard_25> run(const Bitboard_25& startBoard, const Bitboard_25& obstacles) const override {
        SimulationResult<Bitboard_25> result;
        result.footstep.clear();

        Bitboard_25 localObstacles = obstacles;
        localObstacles.setCell(12, 13, true);

        Bitboard_25 currentState = startBoard;
        currentState.applyObstacles(localObstacles);

        Bitboard_25 nextState;
        nextState.clear();

        Bitboard_25* curr = &currentState;
        Bitboard_25* nxt = &nextState;

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